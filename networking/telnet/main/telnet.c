/**
 * Test the telnet functions.
 *
 * Perform a test using the telnet functions.
 * This code exports two new global functions:
 *
 * void telnet_listenForClients(void (*callback)(uint8_t *buffer, size_t size))
 * void telnet_sendData(uint8_t *buffer, size_t size)
 *
 * For additional details and documentation see:
 * * Free book on ESP32 - https://leanpub.com/kolban-ESP32
 *
 *
 * Neil Kolban <kolban1@kolban.com>
 *
 */
#include "stdlib.h" // Required for libtelnet.h
#include "esp_log.h"
#include "libtelnet.h"
#include "lwip/def.h"
#include "lwip/sockets.h"
#include "sdkconfig.h"
#include "errno.h"

static char tag[] = "telnet";

// The global tnHandle ... since we are only processing ONE telnet
// client at a time, this can be a global static.
static telnet_t *tnHandle;

void (*callback)(uint8_t *buffer, size_t size);

static char *eventToString(telnet_event_type_t type) {
	switch(type) {
	case TELNET_EV_COMPRESS:
		return "TELNET_EV_COMPRESS";
	case TELNET_EV_DATA:
		return "TELNET_EV_DATA";
	case TELNET_EV_DO:
		return "TELNET_EV_DO";
	case TELNET_EV_DONT:
		return "TELNET_EV_DONT";
	case TELNET_EV_ENVIRON:
		return "TELNET_EV_ENVIRON";
	case TELNET_EV_ERROR:
		return "TELNET_EV_ERROR";
	case TELNET_EV_IAC:
		return "TELNET_EV_IAC";
	case TELNET_EV_MSSP:
		return "TELNET_EV_MSSP";
	case TELNET_EV_SEND:
		return "TELNET_EV_SEND";
	case TELNET_EV_SUBNEGOTIATION:
		return "TELNET_EV_SUBNEGOTIATION";
	case TELNET_EV_TTYPE:
		return "TELNET_EV_TTYPE";
	case TELNET_EV_WARNING:
		return "TELNET_EV_WARNING";
	case TELNET_EV_WILL:
		return "TELNET_EV_WILL";
	case TELNET_EV_WONT:
		return "TELNET_EV_WONT";
	case TELNET_EV_ZMP:
		return "TELNET_EV_ZMP";
	}
	return "Unknown type";
} // eventToString

struct telnetUserData {
	int sockfd;
};

void telnet_sendData(uint8_t *buffer, size_t size) {
	if (tnHandle != NULL) {
		telnet_send(tnHandle, (char *)buffer, size);
	}
}


static void myTelnetHandler(
		telnet_t *thisTelnet,
		telnet_event_t *event,
		void *userData) {
	int rc;
	ESP_LOGD(tag, "telnet event: %s", eventToString(event->type));
	struct telnetUserData *telnetUserData = (struct telnetUserData *)userData;
	switch(event->type) {
	case TELNET_EV_SEND:
		rc = send(telnetUserData->sockfd, event->data.buffer, event->data.size, 0);
		if (rc < 0) {
			ESP_LOGE(tag, "send: %d", errno);
		}
		break;

	case TELNET_EV_DATA:
		ESP_LOGD(tag, "received data, len=%d", event->data.size);
		/**
		 * Here is where we would want to handle newly received data.
		 * The data receive is in event->data.buffer of size
		 * event->data.size.
		 */
		if (callback != NULL) {
			callback((uint8_t *)event->data.buffer, (size_t)event->data.size);
		}
		break;

	default:
		break;
	} // End of switch event type
} // myTelnetHandler


static void doTelnet(int partnerSocket) {
	ESP_LOGD(tag, ">> doTelnet");
  static const telnet_telopt_t my_telopts[] = {
    { TELNET_TELOPT_ECHO,      TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_TTYPE,     TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_BINARY,    TELNET_WILL, TELNET_DO   },
    { TELNET_TELOPT_NAWS,      TELNET_WILL, TELNET_DONT },
    { -1, 0, 0 }
  };
  struct telnetUserData *pTelnetUserData = (struct telnetUserData *)malloc(sizeof(struct telnetUserData));
  pTelnetUserData->sockfd = partnerSocket;

  tnHandle = telnet_init(my_telopts, myTelnetHandler, 0, pTelnetUserData);

  uint8_t buffer[1024];
  while(1) {
  	ESP_LOGD(tag, "waiting for data");
  	ssize_t len = recv(partnerSocket, (char *)buffer, sizeof(buffer), 0);
  	if (len == 0) {
  		break;
  	}
  	ESP_LOGD(tag, "received %d bytes", len);
  	telnet_recv(tnHandle, (char *)buffer, len);
  }
  ESP_LOGD(tag, "Telnet partner finished");
  telnet_free(tnHandle);
  tnHandle = NULL;
  free(pTelnetUserData);
} // doTelnet


void telnet_listenForClients(void (*callbackParam)(uint8_t *buffer, size_t size)) {
	ESP_LOGD(tag, ">> telnet_listenForClients");
	callback = callbackParam;
	int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(23);

	int rc = bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (rc < 0) {
		ESP_LOGE(tag, "bind: %d", errno);
		return;
	}

	rc = listen(serverSocket, 5);
	if (rc < 0) {
		ESP_LOGE(tag, "listen: %d", errno);
		return;
	}

	while(1) {
		socklen_t len = sizeof(serverAddr);
		rc = accept(serverSocket, (struct sockaddr *)&serverAddr, &len);
		if (rc < 0) {
			ESP_LOGE(tag, "accept: %d", errno);
			return;
		}
		int partnerSocket = rc;

		ESP_LOGD(tag, "We have a new client connection!");
		doTelnet(partnerSocket);
	}
} // listenForNewClient
