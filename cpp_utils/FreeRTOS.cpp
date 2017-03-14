/*
 * FreeRTOS.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include "FreeRTOS.h"
#include "sdkconfig.h"

FreeRTOS::FreeRTOS() {
	// TODO Auto-generated constructor stub

}

FreeRTOS::~FreeRTOS() {
	// TODO Auto-generated destructor stub
}

/**
 * Sleep for the specified number of milliseconds.
 * @param[in] ms The period in milliseconds for which to sleep.
 */
void FreeRTOS::sleep(uint32_t ms) {
	::vTaskDelay(ms/portTICK_PERIOD_MS);
}

/**
 * Start a new task.
 * @param[in] task The function pointer to the function to be run in the task.
 * @param[in] taskName A string identifier for the task.
 * @param[in] param An optional parameter to be passed to the started task.
 * @param[in] stackSize An optional paremeter supplying the size of the stack in which to run the task.
 */
void FreeRTOS::startTask(void task(void*), std::string taskName, void *param, int stackSize) {
	::xTaskCreate(task, taskName.data(), stackSize, param, 5, NULL);
}

/**
 * Delete the task.
 * @param[in] pTask An optional handle to the task to be deleted.  If not supplied the calling task will be deleted.
 */
void FreeRTOS::deleteTask(TaskHandle_t pTask) {
	::vTaskDelete(pTask);
}

/**
 * Get the time in milliseconds since the %FreeRTOS scheduler started.
 * @return The time in milliseconds since the %FreeRTOS scheduler started.
 */
uint32_t FreeRTOS::getTimeSinceStart() {
	return (uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS);
}
