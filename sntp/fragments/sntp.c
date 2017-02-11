#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include <apps/sntp/sntp.h>
#include <lwip/sockets.h>
#include <sys/time.h>
#include "sdkconfig.h"

/*
 * Connect to an Internet time server to get the current date and
 * time.
 */
void startSNTP() {
	ip_addr_t addr;
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	inet_pton(AF_INET, "129.6.15.28", &addr);
	sntp_setserver(0, &addr);
	sntp_init();
}
