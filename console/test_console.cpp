/**
 * Test of console
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


struct arg_lit* show_arg_show = arg_lit0("s", "show", "generate show");
struct arg_end* show_arg_end = arg_end(10);
void *show_argtable[] = {
		show_arg_show,
		show_arg_end
};

int runShow(int argc, char *argv[]) {
	printf("Found show!\n");
	return 0;
}

struct arg_str* greet_arg_name = arg_str1("n", "name", "<string>", "generate help");
struct arg_end* greet_arg_end  = arg_end(10);
void *greet_argtable[] = {
		greet_arg_name,
		greet_arg_end
};

int runGreet(int argc, char *argv[]) {
	printf("Found Greet!\n");
	int numErrors = arg_parse(argc, argv, greet_argtable);
	if (numErrors > 0) {
		arg_print_errors(stdout, greet_arg_end, "greet");
	} else {
		printf("Hello %s\n", greet_arg_name->sval[0]);
	}
	return 0;
}

void test_console(void)
{
	// Boiler plate setup for using linenoise
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
  esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
  uart_driver_install((uart_port_t)CONFIG_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0);
  esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);

  esp_console_config_t consoleConfig;
  consoleConfig.max_cmdline_args   = 5;
  consoleConfig.max_cmdline_length = 100;

  esp_console_init(&consoleConfig);
  esp_console_register_help_command();

  esp_console_cmd_t consoleCmd;
  consoleCmd.command  = "show";
  consoleCmd.func     = runShow;
  consoleCmd.help     = "Show something";
  consoleCmd.argtable = show_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "greet";
  consoleCmd.func     = runGreet;
  consoleCmd.help     = "Greet someone";
  consoleCmd.argtable = greet_argtable;
  esp_console_cmd_register(&consoleCmd);


  linenoiseClearScreen();
  linenoiseSetMultiLine(0);


	while(1) {
		char* line = linenoise("Enter command > ");
		if (line != NULL) {
			printf("Got: %s\n", line);
			int ret;
			esp_console_run(line, &ret);
			linenoiseHistoryAdd(line);
			linenoiseFree(line);
		} // Line is not null
		printf("--------------\n");
	} // End while loop
}
