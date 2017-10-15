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

#include <freertos/FreeRTOS.h>   // Include the base FreeRTOS definitions
#include <freertos/task.h>       // Include the task definitions
#include <freertos/semphr.h>     // Include the semaphore definitions


/**
 * @brief Interface to %FreeRTOS functions.
 */
class FreeRTOS {
public:
	static void sleep(uint32_t ms);
	static void startTask(void task(void *), std::string taskName, void *param=nullptr, int stackSize = 2048);
	static void deleteTask(TaskHandle_t pTask = nullptr);

	static uint32_t getTimeSinceStart();

	class Semaphore {
	public:
		Semaphore(std::string owner = "<Unknown>");
		~Semaphore();
		void give();
		void giveFromISR();
		void give(uint32_t value);
		void setName(std::string name);
		void take(std::string owner="<Unknown>");
		void take(uint32_t timeoutMs, std::string owner="<Unknown>");
		uint32_t wait(std::string owner="<Unknown>");
		std::string toString();
	private:
		SemaphoreHandle_t m_semaphore;
		std::string       m_name;
		std::string       m_owner;
		uint32_t          m_value;
	};
};

#endif /* MAIN_FREERTOS_H_ */
