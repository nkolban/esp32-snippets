/*
 * DMA.cpp
 *
 *  Created on: May 27, 2018
 *      Author: kolban
 */

#include "DMA.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <soc/soc.h>             // Inclusions for WRITE_PERI_xxx and family.
#include <soc/i2s_reg.h>         // Inclusions for I2S registers.
#include <soc/i2s_struct.h>

#include <driver/periph_ctrl.h>
#include <driver/gpio.h>
#include <rom/lldesc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <GeneralUtils.h>

#include <Task.h>

#define D0    GPIO_NUM_4
#define D1    GPIO_NUM_5
#define D2    GPIO_NUM_18
#define D3    GPIO_NUM_19
#define D4    GPIO_NUM_36
#define D5    GPIO_NUM_39
#define D6    GPIO_NUM_34
#define D7    GPIO_NUM_35
#define VSYNC GPIO_NUM_25
#define HREF  GPIO_NUM_23
#define PCLK  GPIO_NUM_22

lldesc_t ll1;

QueueHandle_t queueHandle;
intr_handle_t intHandle;

class LogInterruptTask : public Task {
	void run(void *data) {
		printf("Starting interrupt log\n");
		uint32_t queueItem;
		while(1) {
			xQueueReceive(queueHandle, &queueItem, portMAX_DELAY);
			printf("New interrupt value: 0x%x\n", queueItem);
		} // End while 1
	} // LogInterruptTask#run
}; // LogInterruptTask

static void IRAM_ATTR dmaInt(void *arg) {
	uint32_t value = 0x99;
	xQueueSendToBackFromISR(queueHandle, &value, nullptr);
  I2S0.int_clr.val = I2S0.int_raw.val;
}


DMA::DMA() {

}

DMA::~DMA() {
}


void DMA::clearInterupts() {

	// Set 1 bits in the flags for I2S_INT_CLR_REG to clear interrupts.
  I2S0.int_clr.val = 0xFFFFFFFF;              // 32 bits of 1 to clear all flags.

} // DMA#clearInterupts


void DMA::reset() {

	clearInterupts();
  ll1.eof    = 1;
  ll1.length = 0; // Number of valid bytes.
  ll1.size   = 1024; // Size of buffer.
  ll1.offset = 0;
  ll1.sosf   = 0;
  ll1.empty  = 0; // No next link
  ll1.owner  = 1; // 1 = DMA, 2 = software
  memset((void *)ll1.buf, 0xff, ll1.size);



	const uint32_t lc_conf_reset_flags = I2S_IN_RST_M | I2S_AHBM_RST_M | I2S_AHBM_FIFO_RST_M;
	I2S0.lc_conf.val |=  lc_conf_reset_flags;
	I2S0.lc_conf.val &= ~lc_conf_reset_flags;


	// See TRM 12.4.2.
	const uint32_t conf_reset_flags = I2S_RX_RESET_M | I2S_RX_FIFO_RESET_M | I2S_TX_RESET_M | I2S_TX_FIFO_RESET_M;
	I2S0.conf.val |= conf_reset_flags;
	I2S0.conf.val &= ~conf_reset_flags;

	while (I2S0.state.rx_fifo_reset_back) { // Bit is 1 while not ready
		;
	}

  I2S0.in_link.addr = (uint32_t)&ll1;
} // DMA#reset


void DMA::dumpBuffer() {
	printf(">> dumpBuffer\n");
	GeneralUtils::hexDump((uint8_t*)ll1.buf, 64);
	printf("<< dumpBuffer\n");
}

void DMA::setCameraMode() {
	printf(">> setCameraMode\n");

	gpio_pad_select_gpio(D0);
	gpio_pad_select_gpio(D1);
	gpio_pad_select_gpio(D2);
	gpio_pad_select_gpio(D3);
	gpio_pad_select_gpio(D4);
	gpio_pad_select_gpio(D5);
	gpio_pad_select_gpio(D6);
	gpio_pad_select_gpio(D7);
	gpio_pad_select_gpio(VSYNC);
	gpio_pad_select_gpio(HREF);
	gpio_pad_select_gpio(PCLK);

	gpio_set_direction(D0,    GPIO_MODE_INPUT);
	gpio_set_direction(D1,    GPIO_MODE_INPUT);
	gpio_set_direction(D2,    GPIO_MODE_INPUT);
	gpio_set_direction(D3,    GPIO_MODE_INPUT);
	gpio_set_direction(D4,    GPIO_MODE_INPUT);
	gpio_set_direction(D5,    GPIO_MODE_INPUT);
	gpio_set_direction(D6,    GPIO_MODE_INPUT);
	gpio_set_direction(D7,    GPIO_MODE_INPUT);
	gpio_set_direction(VSYNC, GPIO_MODE_INPUT);
	gpio_set_direction(HREF,  GPIO_MODE_INPUT);
	gpio_set_direction(PCLK,  GPIO_MODE_INPUT);

	// Map sources / sinks of data to their logical counter parts.
	gpio_matrix_in(D0,    I2S0I_DATA_IN0_IDX, false);
	gpio_matrix_in(D1,    I2S0I_DATA_IN1_IDX, false);
	gpio_matrix_in(D2,    I2S0I_DATA_IN2_IDX, false);
	gpio_matrix_in(D3,    I2S0I_DATA_IN3_IDX, false);
	gpio_matrix_in(D4,    I2S0I_DATA_IN4_IDX, false);
	gpio_matrix_in(D5,    I2S0I_DATA_IN5_IDX, false);
	gpio_matrix_in(D6,    I2S0I_DATA_IN6_IDX, false);
	gpio_matrix_in(D7,    I2S0I_DATA_IN7_IDX, false);

	// Set constants for the bits [15:8] alternating 1 and 0.  This results in 0xAA.
	gpio_matrix_in(0x30,  I2S0I_DATA_IN8_IDX,  false);
	gpio_matrix_in(0x38,  I2S0I_DATA_IN9_IDX,  false);
	gpio_matrix_in(0x30,  I2S0I_DATA_IN10_IDX, false);
	gpio_matrix_in(0x38,  I2S0I_DATA_IN11_IDX, false);
	gpio_matrix_in(0x30,  I2S0I_DATA_IN12_IDX, false);
	gpio_matrix_in(0x38,  I2S0I_DATA_IN13_IDX, false);
	gpio_matrix_in(0x30,  I2S0I_DATA_IN14_IDX, false);
	gpio_matrix_in(0x38,  I2S0I_DATA_IN15_IDX, false);

	gpio_matrix_in(0x38 /*VSYNC*/, I2S0I_V_SYNC_IDX,   false);
	gpio_matrix_in(0x38,  I2S0I_H_SYNC_IDX,   false); // Constant high
	gpio_matrix_in(0x38 /* HREF */,  I2S0I_H_ENABLE_IDX, false);
	gpio_matrix_in(PCLK,  I2S0I_WS_IN_IDX,    false);

	//SET_PERI_REG_BITS(I2SCONF2_REG, I2S_CAMERA_EN, 1, I2S_CAMERA_EN_S);


	// We must enable I2S0 peripheral
	periph_module_enable(PERIPH_I2S0_MODULE);

	reset();


	/*
	rx_msb_right | result                                                                         |
---------------+--------------------------------------------------------------------------------+
	0            | 00 00 v1low v1high  00 00 v2low v2high  00 00 v3low v3high  00 00 v4low v4high |
	1            | 00 00 v1low v1high  00 00 v2low ??      00 00 v2low ??      00 00 v3 low ??    |
	*/

	// We enable I2S_CONF2_REG:I2S_CAMERA_EN
	I2S0.conf2.camera_en       = 1;           // 1 for camera mode (TRM).

	// We enable I2S_CONF_REG:I2S_LCD_EN
	I2S0.conf2.lcd_en          = 1;           // 1 for camera mode (TRM).

	// I2S_CONF_REG:I2S_RX_SLAVE_MOD
	I2S0.conf.rx_slave_mod     = 1;           // 1 for slave receiving mode.  Required for camera mode (TRM).

	// I2S_CONF_REG:I2S_RX_MSB_RIGHT
	I2S0.conf.rx_msb_right     = 0;           // 0 required for camera mode (TRM).

	// I2S_CONF_REG:I2S_RX_RIGHT_FIRST
	I2S0.conf.rx_right_first   = 0;           // 0 required for camera mode (TRM).

	// I2S_CONF_CHANG_REG:I2S_RX_CHAN_MOD
	I2S0.conf_chan.rx_chan_mod = 1;           // 1 required for camera mode (TRM).

	// I2S_FIFO_CONF_REG:I2S_RX_FIFO_MOD
	I2S0.fifo_conf.rx_fifo_mod = 1;           // 1 = 16bit single channel data.  Required for camera mode (TRM).

  // Configure clock divider
  I2S0.clkm_conf.clkm_div_a   = 1;
  I2S0.clkm_conf.clkm_div_b   = 0;
  I2S0.clkm_conf.clkm_div_num = 2;

  // FIFO will sink data to DMA
  I2S0.fifo_conf.dscr_en = 1;

  // FIFO configuration
  I2S0.fifo_conf.rx_fifo_mod          = 2;
  I2S0.fifo_conf.rx_fifo_mod_force_en = 1;
  I2S0.conf_chan.rx_chan_mod          = 1;

  // Clear flags which are used in I2S serial mode
  I2S0.sample_rate_conf.rx_bits_mod = 0;
  I2S0.conf.rx_right_first          = 0;
  I2S0.conf.rx_msb_right            = 0;
  I2S0.conf.rx_msb_shift            = 0;
  I2S0.conf.rx_mono                 = 0;
  I2S0.conf.rx_short_sync           = 0;
  I2S0.timing.val                   = 0;


  esp_err_t errRc;
  errRc = esp_intr_alloc(
  	ETS_I2S0_INTR_SOURCE,
		ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM,
		&dmaInt,
		NULL,
		&intHandle);
  if (errRc != ESP_OK) {
  	printf("esp_intr_alloc: %d\n", errRc);
  }

  errRc = esp_intr_enable(intHandle);
  if (errRc != ESP_OK) {
  	printf("esp_intr_enable: %d\n", errRc);
  }

  I2S0.rx_eof_num = 8;


  ll1.eof    = 1;
  ll1.length = 0;    // Number of valid bytes.
  ll1.size   = 1024; // Size of buffer.
  ll1.offset = 0;
  ll1.sosf   = 0;
  ll1.empty  = 0;    // No next link
  ll1.owner  = 1;    // 1 = DMA, 0 = software

  memset((void *)ll1.buf, 0xff, ll1.size);

  // The address of the first in-link descriptor.
  I2S0.in_link.addr = (uint32_t)&ll1;

  // Set to 1 to start in-link descriptor.
  //I2S0.in_link.start = 1;

  startRX();

  clearInterupts();

	printf("<< setCameraMode\n");
} // DMA#setCameraMode


void DMA::stopRX() {
  // Set to 0 to stop receiving data.
  I2S0.conf.rx_start = 0;
  I2S0.conf.rx_reset = 1;
} // DMA#stopRX


void DMA::start() {
  ll1.buf    = (uint8_t*)malloc(1024);   // Allocate storage for the linked list buffer.
  queueHandle = xQueueCreate(50, sizeof(uint32_t));

  LogInterruptTask* pLogInterruptTask = new LogInterruptTask();
  pLogInterruptTask->start();
} // DMA#start


void DMA::startRX() {
  // Set to 1 to start receiving data.

	printf(">> DMA::startRX\n");

	I2S0.int_clr.val = I2S0.int_raw.val;

  I2S0.rx_eof_num = 8;

  I2S0.in_link.addr = (uint32_t)&ll1;
  //I2S0.in_link.stop    = 1;
  I2S0.in_link.start   = 1;
  //I2S0.in_link.restart = 1;
  I2S0.conf.rx_start   = 1;

  // Enable interrupt generation
  I2S0.int_ena.in_suc_eof    = true;
  //I2S0.int_ena.in_done       = true;
  /*
  I2S0.int_ena.in_dscr_empty = true;
  I2S0.int_ena.in_dscr_err   = true;
  I2S0.int_ena.in_err_eof    = true;

  I2S0.int_ena.rx_hung       = true;
  I2S0.int_ena.rx_rempty     = true;
  I2S0.int_ena.rx_take_data  = true;
  I2S0.int_ena.rx_wfull      = true;
  */
  //I2S0.int_ena.rx_take_data  = true;


	printf("<< DMA::startRX\n");
} // DMA#startRX


void DMA::dumpStatus() {
	printf("conf\n");
	printf("%-20s: %d\n", "I2S_RX_SLAVE_MOD",   I2S0.conf.rx_slave_mod);
	printf("%-20s: %d\n", "I2S_RX_MSB_RIGHT",   I2S0.conf.rx_msb_right);
	printf("%-20s: %d\n", "I2S_RX_RIGHT_FIRST", I2S0.conf.rx_right_first);
	printf("%-20s: %d\n", "I2S_RX_START",       I2S0.conf.rx_start);
	printf("%-20s: %d\n", "I2S_RX_RESET",       I2S0.conf.rx_reset);
	printf("-----\n");

	printf("conf_chan\n");
	printf("%-20s: %d\n", "I2S_RX_CHAN_MOD", I2S0.conf_chan.rx_chan_mod);
	printf("-----\n");

	printf("fifo_conf (I2S_FIFO_CONF_REG)\n");
	printf("%-26s: %d\n", "I2S_RX_FIFO_MOD_FORCE_EN", I2S0.fifo_conf.rx_fifo_mod_force_en);
	printf("%-26s: %d\n", "I2S_RX_FIFO_MOD",          I2S0.fifo_conf.rx_fifo_mod);
	printf("%-26s: %d\n", "I2S_RX_DSCR_EN",           I2S0.fifo_conf.dscr_en);
	printf("%-26s: %d\n", "I2S_RX_DATA_NUM",          I2S0.fifo_conf.rx_data_num);
	printf("-----\n");

	printf("conf2\n");
	printf("%-20s: %d\n", "I2S_CAMERA_EN", I2S0.conf2.camera_en);
	printf("%-20s: %d\n", "I2S_LCD_EN",    I2S0.conf2.lcd_en);
	printf("-----\n");


	printf("state\n");
	printf("%-20s: %d\n", "I2S_RX_FIFO_RESET_BACK", I2S0.state.rx_fifo_reset_back);
	printf("-----\n");

	printf("rx_eof_num (I2S_RXEOF_NUM_REG)\n");
	printf("%-20s: %d\n", "I2S_RXEOF_NUM_REG", I2S0.rx_eof_num);
	printf("-----\n");

	printf("in_eof_des_addr (I2S_IN_EOF_DES_ADDR_REG)\n");
	printf("%-20s: 0x%x\n", "I2S_IN_EOF_DES_ADDR_REG", I2S0.in_eof_des_addr);
	printf("-----\n");

	printf("in_link_dscr (I2S_INLINK_DSCR_REG)\n");
	printf("%-20s: 0x%x\n", "I2S_INLINK_DSCR_REG", I2S0.in_link_dscr);
	printf("-----\n");

	printf("in_link_dscr_bf0 (I2S_INLINK_DSCR_BF0_REG)\n");
	printf("%-20s: 0x%x\n", "I2S_INLINK_DSCR_BF0_REG", I2S0.in_link_dscr_bf0);
	printf("-----\n");

	printf("in_link_dscr_bf1 (I2S_INLINK_DSCR_BF1_REG)\n");
	printf("%-20s: 0x%x\n", "I2S_INLINK_DSCR_BF1_REG", I2S0.in_link_dscr_bf1);
	printf("-----\n");

	printf("int_raw\n");
	printf("%-25s: %d\n", "I2S_IN_DSCR_EMPTY_INT_RAW", I2S0.int_raw.in_dscr_empty);
	printf("%-25s: %d\n", "I2S_IN_DSCR_ERR_INT_RAW",   I2S0.int_raw.in_dscr_err);
	printf("%-25s: %d\n", "I2S_IN_SUC_EOF_INT_RAW",    I2S0.int_raw.in_suc_eof);
	printf("%-25s: %d\n", "I2S_IN_ERR_EOF_INT_RAW",    I2S0.int_raw.in_err_eof);
	printf("%-25s: %d\n", "I2S_IN_DONE_INT_RAW",       I2S0.int_raw.in_done);
	printf("%-25s: %d\n", "I2S_RX_HUNG_INT_RAW",       I2S0.int_raw.rx_hung);
	printf("%-25s: %d\n", "I2S_RX_REMPTY_INT_RAW",     I2S0.int_raw.rx_rempty);
	printf("%-25s: %d\n", "I2S_RX_WFULL_INT_RAW",      I2S0.int_raw.rx_wfull);
	printf("%-25s: %d\n", "I2S_RX_TAKE_DATA_INT_RAW",  I2S0.int_raw.rx_take_data);
	printf("-----\n");

	printf("int_ena\n");
	printf("%-25s: %d\n", "I2S_IN_DSCR_EMPTY_INT_RAW", I2S0.int_ena.in_dscr_empty);
	printf("%-25s: %d\n", "I2S_IN_DSCR_ERR_INT_RAW",   I2S0.int_ena.in_dscr_err);
	printf("%-25s: %d\n", "I2S_IN_SUC_EOF_INT_RAW",    I2S0.int_ena.in_suc_eof);
	printf("%-25s: %d\n", "I2S_IN_ERR_EOF_INT_RAW",    I2S0.int_ena.in_err_eof);
	printf("%-25s: %d\n", "I2S_IN_DONE_INT_RAW",       I2S0.int_ena.in_done);
	printf("%-25s: %d\n", "I2S_RX_HUNG_INT_RAW",       I2S0.int_ena.rx_hung);
	printf("%-25s: %d\n", "I2S_RX_REMPTY_INT_RAW",     I2S0.int_ena.rx_rempty);
	printf("%-25s: %d\n", "I2S_RX_WFULL_INT_RAW",      I2S0.int_ena.rx_wfull);
	printf("%-25s: %d\n", "I2S_RX_TAKE_DATA_INT_RAW",  I2S0.int_ena.rx_take_data);
	printf("-----\n");

	printf("in_link\n");
	printf("%-20s: %d\n", "I2S_INLINK_PARK",    I2S0.in_link.park);
	printf("%-20s: %d\n", "I2S_INLINK_RESTART", I2S0.in_link.restart);
	printf("%-20s: %d\n", "I2S_INLINK_START",   I2S0.in_link.start);
	printf("%-20s: %d\n", "I2S_INLINK_STOP",    I2S0.in_link.stop);
	printf("%-20s: 0x%x\n", "I2S_INLINK_ADDR",  (I2S0.in_link.addr & 0xFFFFF));
	printf("-----\n");

	printf("lldesc 1: 0x%x\n", (uint32_t)&ll1);
	printf("%-20s: %d\n",   "length", ll1.length);
	printf("%-20s: %d\n",   "size",   ll1.size);
	printf("%-20s: %d\n",   "owner", ll1.owner);
	printf("%-20s: %d\n",   "empty", ll1.empty);
	printf("%-20s: 0x%x\n", "&buf",   (uint32_t)ll1.buf);
} // DMA#dumpStatus
