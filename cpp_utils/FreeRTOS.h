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
#include <pthread.h>

#include <freertos/FreeRTOS.h>   // Include the base FreeRTOS definitions.
#include <freertos/task.h>       // Include the task definitions.
#include <freertos/semphr.h>     // Include the semaphore definitions.
#include <freertos/ringbuf.h>    // Include the ringbuffer definitions.


/**
 * @brief Interface to %FreeRTOS functions.
 */
class FreeRTOS {
public:
	static void sleep(uint32_t ms);
	static void startTask(void task(void*), std::string taskName, void* param = nullptr, uint32_t stackSize = 2048);
	static void deleteTask(TaskHandle_t pTask = nullptr);

	static uint32_t getTimeSinceStart();

	class Semaphore {
	public:
		Semaphore(std::string owner = "<Unknown>");
		~Semaphore();
		void        give();
		void        give(uint32_t value);
		void        giveFromISR();
		void        setName(std::string name);
		bool        take(std::string owner = "<Unknown>");
		bool        take(uint32_t timeoutMs, std::string owner = "<Unknown>");
		std::string toString();
		uint32_t	wait(std::string owner = "<Unknown>");

	private:
		SemaphoreHandle_t m_semaphore;
		pthread_mutex_t   m_pthread_mutex;
		std::string       m_name;
		std::string       m_owner;
		uint32_t          m_value;
		bool              m_usePthreads;

	};
};


/**
 * @brief Ringbuffer.
 */
class Ringbuffer {
public:
	Ringbuffer(size_t length, RingbufferType_t type = RINGBUF_TYPE_NOSPLIT);
	~Ringbuffer();

	void*    receive(size_t* size, TickType_t wait = portMAX_DELAY);
	void     returnItem(void* item);
	bool     send(void* data, size_t length, TickType_t wait = portMAX_DELAY);
private:
	RingbufHandle_t m_handle;
};

#endif /* MAIN_FREERTOS_H_ */
