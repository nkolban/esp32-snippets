#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <espfs.h>
#include "sdkconfig.h"

char tag[] = "espfs_main";
static void * flashAddress = (void *)(4*1024*1024 - 10 *64*1024); // 0x36 0000

void doWork(void *ignore) {

	char buff[5*1024];
	ESP_LOGD(tag, "Flash address is 0x%x", (int)flashAddress);
	if (espFsInit(flashAddress, 64*1024) != ESPFS_INIT_RESULT_OK) {
		ESP_LOGD(tag, "Failed to initialize espfs");
		return;
	}

	EspFsFile *fh = espFsOpen("files/test3.txt");;

	if (fh != NULL) {
		int sizeRead = 0;
		sizeRead = espFsRead(fh, buff, sizeof(buff));
		ESP_LOGD(tag, "Result: %.*s", sizeRead, buff);

		size_t fileSize;
		char *data;
		sizeRead = espFsAccess(fh, (void **)&data, &fileSize);
		ESP_LOGD(tag, "Result from access: %.*s", fileSize, data);

		espFsClose(fh);
		vTaskDelete(NULL);
	}
}
void app_main(void)
{
  xTaskCreatePinnedToCore(&doWork, "doWork", 8000, NULL, 5, NULL, 0);

}

