/*
 * Task.cpp
 *
 *  Created on: Mar 4, 2017
 *      Author: kolban
 */


#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>

#include "Task.h"
#include "sdkconfig.h"

static char tag[] = "Task";


/**
 * @brief Create an instance of the task class.
 *
 * @param [in] taskName The name of the task to create.
 * @param [in] stackSize The size of the stack.
 * @return N/A.
 */
Task::Task(std::string taskName, uint16_t stackSize) {
	this->stackSize = stackSize;
	taskData = nullptr;
	handle   = nullptr;
} // Task

Task::~Task() {
} // ~Task

/**
 * @brief Suspend the task for the specified milliseconds.
 *
 * @param [in] ms The delay time in milliseconds.
 * @return N/A.
 */

void Task::delay(int ms) {
	::vTaskDelay(ms/portTICK_PERIOD_MS);
} // delay

/**
 * Static class member that actually runs the target task.
 *
 * The code here will run on the task thread.
 * @param [in] pTaskInstance The task to run.
 */
void Task::runTask(void *pTaskInstance) {
	ESP_LOGD(tag, ">> runTask");
	Task *pTask = (Task *)pTaskInstance;
	pTask->run(pTask->taskData);
	pTask->stop();
} // runTask

/**
 * @brief Start an instance of the task.
 *
 * @param [in] taskData Data to be passed into the task.
 * @return N/A.
 */
void Task::start(void *taskData) {
	if (handle != nullptr) {
		ESP_LOGW(tag, "Task::start - There might be a task already running!");
	}
	this->taskData = taskData;
	::xTaskCreate(&runTask, taskName.c_str(), stackSize, this, 5, &handle);
} // start


/**
 * @brief Stop the task.
 *
 * @return N/A.
 */
void Task::stop() {
	if (handle == nullptr) {
		return;
	}
	xTaskHandle temp = handle;
	handle = nullptr;
	::vTaskDelete(temp);
} // stop

/**
 * @brief Set the stack size of the task.
 *
 * @param [in] stackSize The size of the stack for the task.
 * @return N/A.
 */
void Task::setStackSize(uint16_t stackSize) {
	this->stackSize = stackSize;
} // setStackSize
