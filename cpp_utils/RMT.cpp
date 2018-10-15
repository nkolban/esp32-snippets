/*
 * RMT.cpp
 *
 *  Created on: Mar 1, 2017
 *      Author: kolban
 */
#include <esp_log.h>
#include <driver/rmt.h>
#include "RMT.h"


//static char tag[] = "RMT";
/**
 * @brief Create a class instance.
 *
 * @param [in] pin The GPIO pin on which the signal is sent/received.
 * @param [in] channel The RMT channel to work with.
 */
RMT::RMT(gpio_num_t pin, rmt_channel_t channel) {
	this->channel = channel;

	rmt_config_t config;
	config.rmt_mode                  = RMT_MODE_TX;
	config.channel                   = channel;
	config.gpio_num                  = pin;
	config.mem_block_num             = 8 - this->channel;
	config.clk_div                   = 8;
	config.tx_config.loop_en         = 0;
	config.tx_config.carrier_en      = 0;
	config.tx_config.idle_output_en  = 1;
	config.tx_config.idle_level      = (rmt_idle_level_t)0;
	config.tx_config.carrier_freq_hz = 10000;
	config.tx_config.carrier_level   = (rmt_carrier_level_t)1;
	config.tx_config.carrier_duty_percent = 50;

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(this->channel, 0, 0));
}


/**
 * @brief Class destructor.
 */
RMT::~RMT() {
	ESP_ERROR_CHECK(::rmt_driver_uninstall(this->channel));
}


/**
 * @brief Start receiving.
 */
void RMT::rxStart() {
	ESP_ERROR_CHECK(::rmt_rx_start(this->channel, true));
}


/**
 * @brief Stop receiving.
 */
void RMT::rxStop() {
	ESP_ERROR_CHECK(::rmt_rx_stop(this->channel));
}


/**
 * @brief Start transmitting.
 */
void RMT::txStart() {
	ESP_ERROR_CHECK(::rmt_tx_start(this->channel, true));
}


/**
 * @brief Stop transmitting.
 */
void RMT::txStop() {
	ESP_ERROR_CHECK(::rmt_tx_stop(this->channel));
}


/**
 * @brief Write the items out through the RMT.
 *
 * The level/duration set of bits that were added to the transaction are written
 * out through the RMT device.  After transmission, the list of level/durations
 * is cleared.
 */
void RMT::write() {
	add(false, 0);
	ESP_ERROR_CHECK(::rmt_write_items(this->channel, &items[0], items.size(), true));
	clear();
}


/**
 * @brief Add a level/duration to the transaction to be written.
 *
 * @param [in] level The level of the bit to output.
 * @param [in] duration The duration of the bit to output.
 */
void RMT::add(bool level, uint32_t duration) {
	if (bitCount%2 == 0) {
		rmt_item32_t item;
		item.level0 = level;
		item.duration0 = duration;
		items.push_back(item);
	} else {
		items.at(bitCount / 2).level1 = level;
		items.at(bitCount / 2).duration1 = duration;
	}
	bitCount++;
}


/**
 * @brief Clear any previously written level/duration pairs that have not been sent.
 */
void RMT::clear() {
	items.clear();
	bitCount = 0;
}
