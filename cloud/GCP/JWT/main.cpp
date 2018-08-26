#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <tcpip_adapter.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <apps/sntp/sntp.h>

#include "passwords.h"
#include "base64url.h"

// This is an "xxd" file of the PEM of the private key.
#include "device1_private_pem.h"


extern "C" {
    void app_main();
}

/**
 * Return a string representation of an mbedtls error code
 */
static char* mbedtlsError(int errnum) {
    static char buffer[200];
    mbedtls_strerror(errnum, buffer, sizeof(buffer));
    return buffer;
} // mbedtlsError


/**
 * Create a JWT token for GCP.
 * For full details, perform a Google search on JWT.  However, in summary, we build two strings.  One that represents the
 * header and one that represents the payload.  Both are JSON and are as described in the GCP and JWT documentation.  Next
 * we base64url encode both strings.  Note that is distinct from normal/simple base64 encoding.  Once we have a string for
 * the base64url encoding of both header and payload, we concatenate both strings together separated by a ".".   This resulting
 * string is then signed using RSASSA which basically produces an SHA256 message digest that is then signed.  The resulting
 * binary is then itself converted into base64url and concatenated with the previously built base64url combined header and
 * payload and that is our resulting JWT token.
 * @param projectId The GCP project.
 * @param privateKey The PEM or DER of the private key.
 * @param privateKeySize The size in bytes of the private key.
 * @returns A JWT token for transmission to GCP.
 */
char* createGCPJWT(const char* projectId, uint8_t* privateKey, size_t privateKeySize) {
    char base64Header[100];
    const char header[] = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
    base64url_encode(
        (unsigned char *)header,   // Data to encode.
        strlen(header),            // Length of data to encode.
        base64Header);             // Base64 encoded data.

    time_t now;
    time(&now);
    uint32_t iat = now;              // Set the time now.
    uint32_t exp = iat + 60*60;      // Set the expiry time.

    char payload[100];
    sprintf(payload, "{\"iat\":%d,\"exp\":%d,\"aud\":\"%s\"}", iat, exp, projectId);

    char base64Payload[100];
    base64url_encode(
        (unsigned char *)payload,  // Data to encode.
        strlen(payload),           // Length of data to encode.
        base64Payload);            // Base64 encoded data.

    uint8_t headerAndPayload[800];
    sprintf((char*)headerAndPayload, "%s.%s", base64Header, base64Payload);

    // At this point we have created the header and payload parts, converted both to base64 and concatenated them
    // together as a single string.  Now we need to sign them using RSASSA

    mbedtls_pk_context pk_context;
    mbedtls_pk_init(&pk_context);
    int rc = mbedtls_pk_parse_key(&pk_context, privateKey, privateKeySize, NULL, 0);
    if (rc != 0) {
        printf("Failed to mbedtls_pk_parse_key: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return nullptr;
    }

    uint8_t oBuf[5000];

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    const char* pers="MyEntropy";
                
    mbedtls_ctr_drbg_seed(
        &ctr_drbg,
        mbedtls_entropy_func,
        &entropy,
        (const unsigned char*)pers,
        strlen(pers));
    

    uint8_t digest[32];
    rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), headerAndPayload, strlen((char*)headerAndPayload), digest);
    if (rc != 0) {
        printf("Failed to mbedtls_md: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return nullptr;        
    }

    size_t retSize;
    rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, digest, sizeof(digest), oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (rc != 0) {
        printf("Failed to mbedtls_pk_sign: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return nullptr;        
    }


    char base64Signature[600];
    base64url_encode((unsigned char *)oBuf, retSize, base64Signature);

    char* retData = (char*)malloc(strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1);

    sprintf(retData, "%s.%s", headerAndPayload, base64Signature);

    mbedtls_pk_free(&pk_context);
    return retData;
}

void run(void *) {
    printf("Task starting!\n");
    const char* projectId = "test-214415";
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "time-a-g.nist.gov");
    sntp_init();
    // https://www.epochconverter.com/
    time_t now = 0;
    time(&now);
    while(now < 5000) {
        vTaskDelay(1000 * portTICK_RATE_MS);
        time(&now);
    }

    char* jwt = createGCPJWT(projectId, device1_private_pem, device1_private_pem_len);
    if (jwt != nullptr) {
        printf("JWT: %s\n", jwt);
        free(jwt);
    }
    vTaskDelete(nullptr);
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
   if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
      printf("Our IP address is " IPSTR "\n",
         IP2STR(&event->event_info.got_ip.ip_info.ip));
      printf("We have now connected to a station and can do things...\n");
      xTaskCreate(run, "run", 16000, nullptr, 0, nullptr);

   }

   if (event->event_id == SYSTEM_EVENT_STA_START) {
      ESP_ERROR_CHECK(esp_wifi_connect());
   }
   return ESP_OK;
}

void app_main(void)
{
    printf("Starting\n");
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t sta_config;
    memset(&sta_config, 0, sizeof(sta_config));
    strcpy((char*)sta_config.sta.ssid, SSID);
    strcpy((char*)sta_config.sta.password, SSID_PASSWORD);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

