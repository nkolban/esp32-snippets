/**
 * DMA Driver
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
#include "DMA.h"
#include "GPIODriver.h"
#include <GeneralUtils.h>

DMA dma;


struct arg_end* reset_arg_end = arg_end(10);
void *reset_argtable[] = {
		reset_arg_end
};

int runReset(int argc, char *argv[]) {
	printf("Reset\n");
	dma.reset();
	return 0;
}


struct arg_end* start_arg_end = arg_end(10);
void *start_argtable[] = {
		start_arg_end
};

int runStart(int argc, char *argv[]) {
	printf("Started\n");
	return 0;
}


struct arg_end* startRX_arg_end = arg_end(10);
void *startRX_argtable[] = {
		startRX_arg_end
};

int runStartRX(int argc, char *argv[]) {
	printf("StartRX\n");
	return 0;
}


struct arg_end* stopRX_arg_end = arg_end(10);
void *stopRX_argtable[] = {
		stopRX_arg_end
};

int runStopRX(int argc, char *argv[]) {
	printf("StopRX\n");
	return 0;
}


struct arg_end* hexdump_arg_end = arg_end(10);
void *hexdump_argtable[] = {
		hexdump_arg_end
};

int runHexdump(int argc, char *argv[]) {
	printf("Hexdump\n");
	dma.dumpBuffer();
	return 0;
}

struct arg_end* clientStart_arg_end = arg_end(10);
void *clientStart_arg_end_argtable[] = {
		clientStart_arg_end
};

int runClientStart(int argc, char *argv[]) {
	printf("runClientStart\n");
	GPIODriver gpioDriver;
	gpioDriver.run();
	return 0;
}


struct arg_end* camera_arg_end = arg_end(10);
void *camera_argtable[] = {
		camera_arg_end
};

int runCamera(int argc, char *argv[]) {
	printf("camera\n");
	dma.setCameraMode();
	return 0;
}

struct arg_end* stop_arg_end  = arg_end(10);
void *stop_argtable[] = {
		stop_arg_end
};

int runStop(int argc, char *argv[]) {
	printf("Stopped\n");
	return 0;
}


struct arg_end* status_arg_end  = arg_end(10);
void *status_argtable[] = {
		stop_arg_end
};

int runStatus(int argc, char *argv[]) {
	printf("Status\n");
	dma.dumpStatus();
	return 0;
}


void test_console(void)
{
	dma.start();
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
  consoleCmd.command  = "start";
  consoleCmd.func     = runStart;
  consoleCmd.help     = "Start processing";
  consoleCmd.argtable = start_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "stop";
  consoleCmd.func     = runStop;
  consoleCmd.help     = "Stop processing";
  consoleCmd.argtable = stop_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "status";
  consoleCmd.func     = runStatus;
  consoleCmd.help     = "Status processing";
  consoleCmd.argtable = status_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "camera";
  consoleCmd.func     = runCamera;
  consoleCmd.help     = "camera processing";
  consoleCmd.argtable = camera_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "clientStart";
  consoleCmd.func     = runClientStart;
  consoleCmd.help     = "clientStart processing";
  consoleCmd.argtable = clientStart_arg_end_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "hexdump";
  consoleCmd.func     = runHexdump;
  consoleCmd.help     = "hexdump processing";
  consoleCmd.argtable = hexdump_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "startRX";
  consoleCmd.func     = runStartRX;
  consoleCmd.help     = "startRX processing";
  consoleCmd.argtable = startRX_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "stopRX";
  consoleCmd.func     = runStopRX;
  consoleCmd.help     = "stopRX processing";
  consoleCmd.argtable = stopRX_argtable;
  esp_console_cmd_register(&consoleCmd);

  consoleCmd.command  = "reset";
  consoleCmd.func     = runReset;
  consoleCmd.help     = "reset processing";
  consoleCmd.argtable = reset_argtable;
  esp_console_cmd_register(&consoleCmd);


  //linenoiseClearScreen();
  linenoiseSetMultiLine(0);


	while(1) {
		char* line = linenoise("Enter command > ");
		if (line != NULL) {
			int ret;
			esp_console_run(line, &ret);
			linenoiseHistoryAdd(line);
			linenoiseFree(line);
		} // Line is not null
		printf("--------------\n");
	} // End while loop
}
