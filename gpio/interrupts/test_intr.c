/*
 * Test interrupt handling on a GPIO.
 * In this fragment we watch for a change on the input signal
 * of GPIO 25.  When it goes high, an interrupt is raised which
 * adds a message to a queue which causes a task that is blocking
 * on the queue to wake up and process the interrupt.
 */
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "sdkconfig.h"

static char tag[] = "test_intr";
static QueueHandle_t q1;

#define TEST_GPIO (25)
static void handler(void *args) {
	gpio_num_t gpio;
	gpio = TEST_GPIO;
	xQueueSendToBackFromISR(q1, &gpio, NULL);
}

void test1_task(void *ignore) {
	ESP_LOGD(tag, ">> test1_task");
	gpio_num_t gpio;
	q1 = xQueueCreate(10, sizeof(gpio_num_t));

	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = GPIO_SEL_25;
	gpioConfig.mode         = GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
	gpioConfig.intr_type    = GPIO_INTR_POSEDGE;
	gpio_config(&gpioConfig);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(TEST_GPIO, handler, NULL	);
	while(1) {
		ESP_LOGD(tag, "Waiting on interrupt queue");
		BaseType_t rc = xQueueReceive(q1, &gpio, portMAX_DELAY);
		ESP_LOGD(tag, "Woke from interrupt queue wait: %d", rc);
	}
	vTaskDelete(NULL);
}
