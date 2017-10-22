#include "freertos/FreeRTOS.h"
#include "esp_event.h"

extern "C"{
	void app_main(void);
}
void test_task(void*);

void app_main(void)
{
	xTaskCreate(&test_task, "test", 2048, NULL, 5, NULL);
}

