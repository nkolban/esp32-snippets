#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Adafruit_GFX.h"
#include "Adafruit_PCD8544.h"
#include "sdkconfig.h"

static char tag[] = "scroll_text";

static Adafruit_PCD8544 display = Adafruit_PCD8544(
	14, // sclk
	13, // MOSI
	27, // D/C
	15,	// CS
	26);// RST

extern "C" {
	void task_scroll_text(void *ignore);
}

static void print(char *text) {
	while(*text != 0) {
		display.write(*text);
		text++;
	}
}


void task_scroll_text(void *ignore) {
	ESP_LOGD(tag, "start");
  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
	ESP_LOGD(tag, "set contrast");
  display.setContrast(60);

  display.setTextWrap(0);
  int x;
  while(1) {
		for (x=0; x>-80; x-=6) {
			display.clearDisplay();
			display.setCursor(x,5);
			print((char *)"192.168.113.254");
			display.display();
			vTaskDelay(150/portTICK_PERIOD_MS);
			//ESP_LOGD(tag, "next: %d", x);
		}
  }
  ESP_LOGD(tag, "end");
  vTaskDelete(NULL);
}
