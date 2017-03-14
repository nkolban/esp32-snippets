/*
 * PWM.cpp
 *
 *  Created on: Mar 9, 2017
 *      Author: kolban
 */

#include "PWM.h"

/**
 * @brief Construct an instance.
 *
 * @param [in] bitSize The size in bits of the timer.  Allowed values are LEDC_TIMER_10_BIT,
 * LEDC_TIMER_11_BIT, LEDC_TIMER_12_BIT, LEDC_TIMER_13_BIT, LEDC_TIMER_14_BIT, LEDC_TIMER_15_BIT.
 * @param [in] timer The timer to use. A value of LEDC_TIMER0, LEDC_TIMER1, LEDC_TIMER2 or LEDC_TIMER3.
 * @param [in] channel The channel to use.  A value from LEDC_CHANNEL0 to LEDC_CHANNEL1.
 */
PWM::PWM(ledc_timer_bit_t bitSize, ledc_timer_t timer, ledc_channel_t channel,
		int gpioNum) {
	ledc_timer_config_t timer_conf;
	timer_conf.bit_num = bitSize;
	timer_conf.freq_hz = 100;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.timer_num = timer;
	::ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.channel = channel;
	ledc_conf.duty = 0;
	ledc_conf.gpio_num = gpioNum;
	ledc_conf.intr_type = LEDC_INTR_DISABLE;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.timer_sel = timer;
	::ledc_channel_config(&ledc_conf);

	this->channel = channel;
	this->timer = timer;
}


/**
 * @brief Get the duty cycle value.
 *
 * @return The duty cycle value.
 */
uint32_t PWM::getDuty() {
	return ::ledc_get_duty(LEDC_HIGH_SPEED_MODE, channel);
} // getDuty


/**
 * @brief Get the frequency/period in Hz.
 *
 * @return The frequency/period in Hz.
 */
uint32_t PWM::getFrequency() {
	return ::ledc_get_freq(LEDC_HIGH_SPEED_MODE, timer);
} // getFrequency


/**
 * @brief Set the duty cycle value.
 */
void PWM::setDuty(uint32_t duty) {
	::ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty);
	::ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
} // setDuty


/**
 * @brief Set the frequency/period in Hz.
 */
void PWM::setFrequency(uint32_t freq) {
	::ledc_set_freq(LEDC_HIGH_SPEED_MODE, timer, freq);
} // setFrequency

/**
 * @brief Stop the %PWM.
 */
void PWM::stop(bool idleLevel) {
	::ledc_stop(LEDC_HIGH_SPEED_MODE, channel, idleLevel);
} // stop
