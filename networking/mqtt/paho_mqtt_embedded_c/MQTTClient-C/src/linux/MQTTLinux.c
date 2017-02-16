/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include <esp_log.h>
#include "MQTTLinux.h"
#include "mbedtls/platform.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "sdkconfig.h"

static char tag[] = "MQTTESP32";

typedef struct {
	mbedtls_net_context      server_fd;
	mbedtls_entropy_context  entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context      ssl;
	mbedtls_ssl_config       conf;
	mbedtls_x509_crt         cacert;
} mqtt_ssl_context_t;

#ifndef timeradd
#define timeradd(a, b, result)                                 \
{                                                              \
  (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                \
  (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;             \
  if ( (result)->tv_usec >= 1000000 ) {                        \
          (result)->tv_sec++; (result)->tv_usec -= 1000000ul;  \
  }                                                            \
}
#endif

#ifndef timersub
#define timersub(a, b, result)                                 \
{                                                              \
  (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                \
  (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;             \
  if ( (result)->tv_usec < 0 ) {                               \
          (result)->tv_sec--; (result)->tv_usec += 1000000ul;  \
  }                                                            \
}
#endif

void TimerInit(Timer* timer) {
	timer->end_time = (struct timeval){0, 0};
}

char TimerIsExpired(Timer* timer) {
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);		
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout) {
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(Timer* timer, unsigned int timeout) {
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &timer->end_time);
}


int TimerLeftMS(Timer* timer) {
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

static void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
   ((void) level);
   ((void) ctx);
   printf("%s:%04d: %s", file, line, str);
}


static int linux_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	int bytes = 0;
	while (bytes < len) {
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)	{
			if (errno != ENOTCONN && errno != ECONNRESET) {
				bytes = -1;
				break;
			}
		}
		else {
			bytes += rc;
		}
	}
	return bytes;
}


static int linux_read_ssl(Network* n, unsigned char* buffer, int len, int timeout_ms) {
	ESP_LOGD(tag, "linux_read_ssl: %d", len);
	mqtt_ssl_context_t *mqtt_ssl_context = (mqtt_ssl_context_t *)n->sslData;
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	int bytes = 0;

	while (bytes < len)	{
		int rc = mbedtls_ssl_read(&mqtt_ssl_context->ssl, &buffer[bytes], (size_t)len-bytes);
		if (rc < 0) {
			ESP_LOGE(tag, "error from read: %d\n", len);
			return rc;
		}
		if (rc == -1) {
			if (errno != ENOTCONN && errno != ECONNRESET) {
				bytes = -1;
				break;
			}
		}
		else {
			bytes += rc;
		}
	}
	return bytes;
} // linux_read_ssl


static int linux_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval tv;

	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	int	rc = write(n->my_socket, buffer, len);
	return rc;
} // linux_write


static int linux_write_ssl(Network* n, unsigned char* buffer, int len, int timeout_ms) {
	ESP_LOGD(tag, "linux_write_ssl: %d", len);
	mqtt_ssl_context_t *mqtt_ssl_context = (mqtt_ssl_context_t *)n->sslData;
  int ret = mbedtls_ssl_write(&mqtt_ssl_context->ssl, buffer, len);
  if (ret < 0) {
  	char errortext[256];
  	mbedtls_strerror(ret, errortext, sizeof(errortext));
  	ESP_LOGE(tag, "error from write: %d -%x - %s\n", ret, ret, errortext);
  }
	return ret;
} // linux_write_ssl


void NetworkInit(Network* n)
{
	n->my_socket = 0;
	n->mqttread = linux_read;
	n->mqttwrite = linux_write;
	n->isSSL = 0;
} // NetworkInit


void NetworkInitSSL(Network *n) {
	n->isSSL = true;
	n->sslData = (void *)malloc(sizeof(mqtt_ssl_context_t));
	n->mqttread = linux_read_ssl;
	n->mqttwrite = linux_write_ssl;
} // NetworkInitSSL


static int NetworkConnectSSL(Network *n, char *addr, int port) {
  char *pers = "ssl_client1";
	mqtt_ssl_context_t *mqtt_ssl_context = (mqtt_ssl_context_t *)n->sslData;
	mbedtls_net_init(&mqtt_ssl_context->server_fd);
	mbedtls_ssl_init(&mqtt_ssl_context->ssl);
	mbedtls_ssl_config_init(&mqtt_ssl_context->conf);
	mbedtls_x509_crt_init(&mqtt_ssl_context->cacert);
	mbedtls_ctr_drbg_init(&mqtt_ssl_context->ctr_drbg);
	mbedtls_entropy_init(&mqtt_ssl_context->entropy);

  mbedtls_ssl_conf_dbg(&mqtt_ssl_context->conf, my_debug, stdout);
  mbedtls_debug_set_threshold(4); // Log at verbose only

  int ret = mbedtls_ctr_drbg_seed(
     &mqtt_ssl_context->ctr_drbg,
     mbedtls_entropy_func, &mqtt_ssl_context->entropy, (const unsigned char *) pers, strlen(pers));
  if (ret != 0) {
  	ESP_LOGE(tag, " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
     return ret;
  }

  char portStr[8];
  sprintf(portStr, "%d", port);
  ESP_LOGD(tag, "calling mbedtls_net_connect: %s %s", addr, portStr);
  ret = mbedtls_net_connect(&mqtt_ssl_context->server_fd, addr, portStr, MBEDTLS_NET_PROTO_TCP);
  if (ret != 0) {
     ESP_LOGE(tag, " failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
     return ret;
  }

  ret = mbedtls_ssl_config_defaults(
     &mqtt_ssl_context->conf,
     MBEDTLS_SSL_IS_CLIENT,
     MBEDTLS_SSL_TRANSPORT_STREAM,
     MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0) {
     ESP_LOGE(tag, " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
     return ret;
  }

  mbedtls_ssl_conf_authmode(&mqtt_ssl_context->conf, MBEDTLS_SSL_VERIFY_NONE);

  mbedtls_ssl_conf_rng(&mqtt_ssl_context->conf, mbedtls_ctr_drbg_random, &mqtt_ssl_context->ctr_drbg);
  ret = mbedtls_ssl_setup(&mqtt_ssl_context->ssl, &mqtt_ssl_context->conf);
  if (ret != 0) {
  	char errortext[256];
     mbedtls_strerror(ret, errortext, sizeof(errortext));
     ESP_LOGE(tag, "error from mbedtls_ssl_setup: %d - %x - %s\n", ret, ret, errortext);
     return ret;
  }

  ret = mbedtls_ssl_set_hostname(&mqtt_ssl_context->ssl, "yi75rc.messaging.internetofthings.ibmcloud.com");
  if (ret != 0) {
  	char errortext[256];
     mbedtls_strerror(ret, errortext, sizeof(errortext));
     ESP_LOGE(tag, "error from mbedtls_ssl_set_hostname: %d - %x - %s\n", ret, ret, errortext);
     return ret;
  }

  mbedtls_ssl_set_bio(&mqtt_ssl_context->ssl, &mqtt_ssl_context->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
  return 0;
}


int NetworkConnect(Network* n, char* addr, int port)
{
	if (n->isSSL) {
		return NetworkConnectSSL(n, addr, port);
	}
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res) {
			if (res->ai_family == AF_INET) {
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET) {
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else {
			rc = -1;
		}

		freeaddrinfo(result);
	}

	if (rc == 0) {
		n->my_socket = socket(family, type, 0);
		if (n->my_socket != -1)
			rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));
	}

	return rc;
}


void NetworkDisconnect(Network* n) {
	if (n->isSSL) {
		mqtt_ssl_context_t *mqtt_ssl_context = (mqtt_ssl_context_t *)n->sslData;
		mbedtls_net_free(&mqtt_ssl_context->server_fd);
		mbedtls_ssl_free(&mqtt_ssl_context->ssl);
		mbedtls_ssl_config_free(&mqtt_ssl_context->conf);
		mbedtls_ctr_drbg_free(&mqtt_ssl_context->ctr_drbg);
		mbedtls_entropy_free(&mqtt_ssl_context->entropy);
		free(n->sslData);
		n->sslData = NULL;
	} else {
		close(n->my_socket);
	}
} // NetworkDisconnect
