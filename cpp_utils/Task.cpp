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
 * Create an instance of the task class.
 *
 * @param taskName
 * @param stackSize
 * @param taskData
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
 */

void Task::delay(int ms) {
	::vTaskDelay(ms/portTICK_PERIOD_MS);
} // delay

/**
 * Static class member that actually runs the target task.
 *
 * The code here will run on the task thread.
 */
void Task::runTask(void *pData) {
	ESP_LOGD(tag, ">> runTask");
	Task *pTask = (Task *)pData;
	pTask->run(pTask->taskData);
	pTask->stop();
} // runTask

/**
 * @brief Start an instance of the task.
 *
 * @param [in] Data to be passed into the task.
 */
void Task::start(void *taskData) {
	if (handle != nullptr) {
		ESP_LOGW(tag, "Task::start - There might be a task already running!");
	}
	this->taskData = taskData;
	::xTaskCreate(&runTask, taskName.c_str(), stackSize, this, 5, &handle);
} // start


void Task::stop() {
	if (handle == nullptr) {
		return;
	}
	xTaskHandle temp = handle;
	handle = nullptr;
	::vTaskDelete(temp);
} // stop

void Task::setStackSize(uint16_t stackSize) {
	this->stackSize = stackSize;
}
