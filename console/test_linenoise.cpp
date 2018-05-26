/**
 * Test of linenoise
 *
 * @author Neil Kolban
 * @date 2018-05-26
 *
 */
#include <stdio.h>
#include <linenoise/linenoise.h>
#include <stdint.h>
#include <esp_console.h>
#include <esp_vfs_dev.h>
#include <esp_err.h>
#include <driver/uart.h>
#include "sdkconfig.h"

void test_linenoise(void)
{
	// Boiler plate setup for using linenoise
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
  esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
  uart_driver_install((uart_port_t)CONFIG_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0);
  esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);


  linenoiseClearScreen();
  linenoiseSetMultiLine(0);

	while(1) {
		char* line = linenoise("Enter command > ");
		if (line != NULL) {
			printf("Got: %s\n", line);
			linenoiseHistoryAdd(line);
			linenoiseFree(line);
		} // Line is not null
		printf("--------------\n");
	} // End while loop
}
