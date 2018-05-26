/**
 * Test of argtable
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
#include <argtable3/argtable3.h>
#include "sdkconfig.h"

void test_argtable(void)
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

  struct arg_lit* help = arg_lit0("h", "help", "Generate some help!");
  struct arg_lit* test = arg_lit1("t", "test", "Generate some help!");
  struct arg_end* end  = arg_end(20);

  void *argtable[] = {
  		help,
			test,
			end
  };

  char *argv[10];

	while(1) {
		char* line = linenoise("Enter command > ");
		if (line != NULL) {
			printf("Got: %s\n", line);
			size_t argc = esp_console_split_argv(line, argv, 10);
			printf("Parsed to %d argc count\n", argc);
			for (int i=0; i<argc; i++) {
				printf("argv[%d]: %s\n", i, argv[i]);
			}
			int numErrors = arg_parse(argc, argv, argtable);
			if (numErrors>0) {
				printf("Number of errors: %d\n", numErrors);
				arg_print_errors(stdout, end, "myprog");
				arg_print_syntaxv(stdout, argtable, "Here:");
			}
			else {
				if (help->count > 0) {
					printf("Found help!\n");
				}
			}
			linenoiseHistoryAdd(line);
			linenoiseFree(line);
		} // Line is not null
		printf("--------------\n");
	} // End while loop
}
