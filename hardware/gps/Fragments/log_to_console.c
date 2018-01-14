#include "esp_log.h"
#include "driver/uart.h"

static char tag[] = "gps";

char *readLine(uart_port_t uart) {
	static char line[256];
	int size;
	char *ptr = line;
	while(1) {
		size = uart_read_bytes(uart, (unsigned char *)ptr, 1, portMAX_DELAY);
		if (size == 1) {
			if (*ptr == '\n') {
				*ptr = 0;
				return line;
			}
			ptr++;
		} // End of read a character
	} // End of loop
} // End of readLine

void doGPS() {
	ESP_LOGD(tag, ">> doGPS");
	uart_config_t myUartConfig;
	myUartConfig.baud_rate           = 9600;
	myUartConfig.data_bits           = UART_DATA_8_BITS;
	myUartConfig.parity              = UART_PARITY_DISABLE;
	myUartConfig.stop_bits           = UART_STOP_BITS_1;
	myUartConfig.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
	myUartConfig.rx_flow_ctrl_thresh = 120;

	uart_param_config(UART_NUM_1, &myUartConfig);

	uart_set_pin(UART_NUM_1,
			UART_PIN_NO_CHANGE, // TX
			34,                 // RX
			UART_PIN_NO_CHANGE, // RTS
			UART_PIN_NO_CHANGE  // CTS
  );

	uart_driver_install(UART_NUM_1, 2048, 2048, 10, 17, NULL);

	while(1) {
		char *line = readLine(UART_NUM_1);
		ESP_LOGD(tag, "%s", line);
	}
}
