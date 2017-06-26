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
#include <freertos/semphr.h>


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

	class Semaphore {
	public:
		Semaphore(std::string owner = "<Unknown>");
		~Semaphore();
		void give();
		void setName(std::string name);
		void take(std::string owner="<Unknown>");
		void take(uint32_t timeoutMs, std::string owner="<Unknown>");
		std::string toString();
	private:
		SemaphoreHandle_t m_semaphore;
		std::string m_name;
		std::string m_owner;
	};
};

#endif /* MAIN_FREERTOS_H_ */
