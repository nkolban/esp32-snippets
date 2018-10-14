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
 *
 * Pulse Width Modulation (%PWM) is known as "LEDC" in the ESP32.  It allows us to set a repeating
 * clock signal.  There are two aspects to the signal called the frequency and duty cycle.  The
 * frequency is how many times per second the signal repeats.  The duty cycle is how much of a single
 * period the output signal is high compared to low.  For example, a duty cycle of 0% means that the signal
 * is always low, while a duty cycle of 100% means the signal is always high.  A duty cycle of 50% means
 * that the signal is high for 50% of the output and low for the remaining 50%.
 */
class PWM {
public:
	PWM(
		int gpioNum,
		uint32_t frequency       = 100,
		ledc_timer_bit_t bitSize = LEDC_TIMER_10_BIT,
		ledc_timer_t timer       = LEDC_TIMER_0,
		ledc_channel_t channel   = LEDC_CHANNEL_0);

	uint32_t getDuty();
	uint32_t getFrequency();
	void     setDuty(uint32_t duty);
	void     setDutyPercentage(uint8_t percent);
	void     setFrequency(uint32_t freq);
	void     stop(bool idleLevel = false);

private:
	ledc_channel_t   m_channel;
	ledc_timer_t     m_timer;
	ledc_timer_bit_t m_dutyResolution; // Bit size of timer.

};

#endif /* COMPONENTS_CPP_UTILS_PWM_H_ */
