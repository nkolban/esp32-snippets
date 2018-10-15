/*
 * SOC.cpp
 *
 *  Created on: May 16, 2017
 *      Author: kolban
 */

#include "SOC.h"
#include <esp_log.h>
extern "C" {
	#include <soc/i2s_struct.h>
}
#include <stdio.h>

SOC::SOC() {
}

SOC::~SOC() {
}

/**
 * @brief Dump the status of the I2S peripheral.
 * @return N/A.
 */
void SOC::I2S::dump() {
	printf("I2S settings\n");
	printf("I2S_CLKM_CONF_REG\n");
	printf("-----------------\n");
	printf("clkm_div_num: %d, clkm_div_b: %d, clkm_div_a: %d, clk_en: %d, clka_en: %d\n",
		I2S0.clkm_conf.clkm_div_num,
		I2S0.clkm_conf.clkm_div_b,
		I2S0.clkm_conf.clkm_div_a,
		I2S0.clkm_conf.clk_en,
		I2S0.clkm_conf.clka_en);
	uint32_t clockSpeed;
	if (I2S0.clkm_conf.clkm_div_a == 0) {
		clockSpeed = 160000000 / I2S0.clkm_conf.clkm_div_num;
	} else {
		clockSpeed = 160000000 / (I2S0.clkm_conf.clkm_div_num + I2S0.clkm_conf.clkm_div_b / I2S0.clkm_conf.clkm_div_a);
	}
	printf("Clock speed: %d\n", clockSpeed);
	printf("\n");

	printf("I2S_CONF_REG\n");
	printf("------------\n");
	printf("tx_slave_mod: %s, rx_slave_mod: %s, rx_msb_right: %d, rx_right_first: %d\n",
		(I2S0.conf.tx_slave_mod == 0) ? "Master" : "Slave",
		(I2S0.conf.rx_slave_mod == 0) ? "Master" : "Slave",
		I2S0.conf.rx_msb_right,
		I2S0.conf.rx_right_first);
	printf("\n");

	printf("I2S_CONF2_REG\n");
	printf("-------------\n");
	printf("camera_en: %d, lcd_en: %d, inter_valud_en: %d\n",
		I2S0.conf2.camera_en,
		I2S0.conf2.lcd_en,
		I2S0.conf2.inter_valid_en);
	printf("\n");

	printf("I2S_CONF_CHAN_REG\n");
	printf("-----------------\n");
	printf("tx_chan_mod: %d, rx_chan_mod: %d\n",
		I2S0.conf_chan.tx_chan_mod,
		I2S0.conf_chan.rx_chan_mod);
	printf("\n");
} // I2S::dump
