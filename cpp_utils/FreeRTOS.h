/*
 * FreeRTOS.h
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */

#ifndef MAIN_FREERTOS_H_
#define MAIN_FREERTOS_H_
#include <stdint.h>
#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief Interface to %FreeRTOS functions.
 */
class FreeRTOS {
public:
	FreeRTOS();
	virtual ~FreeRTOS();
	static void sleep(uint32_t ms);
	static void startTask(void task(void *), std::string taskName, void *param=nullptr, int stackSize = 2048);
	static void deleteTask(TaskHandle_t pTask = nullptr);

	static uint32_t getTimeSinceStart();
};

#endif /* MAIN_FREERTOS_H_ */
