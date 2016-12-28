#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern void bt_task(void *ignore);


void app_main(void)
{
  xTaskCreatePinnedToCore(&bt_task, "btTask", 2048, NULL, 5, NULL, 0);
}
