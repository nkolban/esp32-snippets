/*
 * PWM.h
 *
 *  Created on: Mar 9, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_PWM_H_
#define COMPONENTS_CPP_UTILS_PWM_H_
#include <driver/ledc.h>
/**
 * @brief A wrapper for ESP32 %PWM control.
 */
class PWM {
public:
	PWM(ledc_timer_bit_t bitSize, ledc_timer_t timer, ledc_channel_t channel, int gpioNum);

	uint32_t getDuty();
	uint32_t getFrequency();
	void setDuty(uint32_t duty);
	void setFrequency(uint32_t freq);
	void stop(bool idleLevel=false);
private:
	ledc_channel_t channel;
	ledc_timer_t timer;
};

#endif /* COMPONENTS_CPP_UTILS_PWM_H_ */
