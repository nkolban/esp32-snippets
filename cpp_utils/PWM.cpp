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
 * A timer starts ticking up from 0 to the maximum value of the bit size.  When it reaches
 * the maximum value, it resets.
 *
 * In the following example, we create a signal that has a frequency of 100Hz and is
 * a square wave (50% on, 50% off).
 *
 * @code{.cpp}
 * PWM pwm(GPIO_NUM_21);
 * pwm.setDutyPercentage(50);
 * pwm.setFrequency(100);
 * @endcode
 *
 * @param [in] gpioNum The GPIO pin to use for output.
 * @param [in] frequency The frequency of the period.
 * @param [in] dutyResolution The size in bits of the timer.  Allowed values are LEDC_TIMER_10_BIT,
 * LEDC_TIMER_11_BIT, LEDC_TIMER_12_BIT, LEDC_TIMER_13_BIT, LEDC_TIMER_14_BIT, LEDC_TIMER_15_BIT.
 * @param [in] timer The timer to use. A value of LEDC_TIMER0, LEDC_TIMER1, LEDC_TIMER2 or LEDC_TIMER3.
 * @param [in] channel The channel to use.  A value from LEDC_CHANNEL0 to LEDC_CHANNEL1.

 * @return N/A.
 */
PWM::PWM(int gpioNum, uint32_t frequency, ledc_timer_bit_t dutyResolution, ledc_timer_t timer, ledc_channel_t channel) {
	ledc_timer_config_t timer_conf;
	timer_conf.duty_resolution    = dutyResolution;
	timer_conf.freq_hz    = frequency;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.timer_num  = timer;
	ESP_ERROR_CHECK(::ledc_timer_config(&timer_conf));

	ledc_channel_config_t ledc_conf;
	ledc_conf.channel    = channel;
	ledc_conf.duty       = 0;
	ledc_conf.gpio_num   = gpioNum;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.timer_sel  = timer;
	ESP_ERROR_CHECK(::ledc_channel_config(&ledc_conf));

	this->m_channel        = channel;
	this->m_timer          = timer;
	this->m_dutyResolution = dutyResolution;
} // PWM


/**
 * @brief Get the duty cycle value.
 *
 * @return The duty cycle value.
 */
uint32_t PWM::getDuty() {
	return ::ledc_get_duty(LEDC_HIGH_SPEED_MODE, m_channel);
} // getDuty


/**
 * @brief Get the frequency/period in Hz.
 *
 * @return The frequency/period in Hz.
 */
uint32_t PWM::getFrequency() {
	return ::ledc_get_freq(LEDC_HIGH_SPEED_MODE, m_timer);
} // getFrequency


/**
 * @brief Set the duty cycle value.
 *
 * The duty cycle value is a numeric between 0 and the maximum bit size.  When the
 * %PWM tick value is less than this value, the output is high while when it is higher
 * than this value, the output is low.
 * @param [in] duty The duty cycle value.
 * @return N/A.
 */
void PWM::setDuty(uint32_t duty) {
	ESP_ERROR_CHECK(::ledc_set_duty(LEDC_HIGH_SPEED_MODE, m_channel, duty));
	ESP_ERROR_CHECK(::ledc_update_duty(LEDC_HIGH_SPEED_MODE, m_channel));
} // setDuty


/**
 * @brief Set the duty cycle as a percentage value.
 *
 * @param [in] percent The percentage of the duty cycle (0-100).
 * @return N/A.
 */
void PWM::setDutyPercentage(uint8_t percent) {
	uint32_t max;
	switch (m_dutyResolution) {
		case LEDC_TIMER_10_BIT:
			max = 1 << 10;
			break;
		case LEDC_TIMER_11_BIT:
			max = 1 << 11;
			break;
		case LEDC_TIMER_12_BIT:
			max = 1 << 12;
			break;
		case LEDC_TIMER_13_BIT:
			max = 1 << 13;
			break;
		case LEDC_TIMER_14_BIT:
			max = 1 << 14;
			break;
		case LEDC_TIMER_15_BIT:
			max = 1 << 15;
			break;
		default:
			max = 1 << 10;
			break;
	}
	if (percent > 100) percent = 100;
	uint32_t value = max * percent / 100;
	if (value >= max) value = max - 1;
	setDuty(value);
} // setDutyPercentage


/**
 * @brief Set the frequency/period in Hz.
 *
 * @param [in] freq The frequency to set the %PWM.
 * @return N/A.
 */
void PWM::setFrequency(uint32_t freq) {
	ESP_ERROR_CHECK(::ledc_set_freq(LEDC_HIGH_SPEED_MODE, m_timer, freq));
} // setFrequency


/**
 * @brief Stop the %PWM.
 *
 * @param [in] idleLevel The level to leave the output after end.
 * @return N/A.
 */
void PWM::stop(bool idleLevel) {
	ESP_ERROR_CHECK(::ledc_stop(LEDC_HIGH_SPEED_MODE, m_channel, idleLevel ? 1 : 0));
} // stop
