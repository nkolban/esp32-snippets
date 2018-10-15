/*
 * FreeRTOSTimer.cpp
 *
 *  Created on: Mar 8, 2017
 *      Author: kolban
 */

#include <assert.h>
#include <map>

#include "FreeRTOSTimer.h"


static std::map<void*, FreeRTOSTimer*> timersMap;

void FreeRTOSTimer::internalCallback(TimerHandle_t xTimer) {
	FreeRTOSTimer* timer = timersMap.at(xTimer);
	timer->callback(timer);
}

/**
 * @brief Construct a timer.
 *
 * We construct a timer that will fire after the given period has elapsed.  The period is measured
 * in ticks.  When the timer fires, the callback function is invoked within the scope/context of
 * the timer demon thread.  As such it must **not** block.  Once the timer has fired, if the reload
 * flag is true, then the timer will be automatically restarted.
 *
 * Note that the timer does *not* start immediately.  It starts ticking after the start() method
 * has been called.
 *
 * The signature of the callback function is:
 *
 * @code{.cpp}
 * void callback(FreeRTOSTimer *pTimer) {
 *	// Callback code here ...
 * }
 * @endcode
 *
 * @param [in] name The name of the timer.
 * @param [in] period The period of the timer in ticks.
 * @param [in] reload True if the timer is to restart once fired.
 * @param [in] data Data to be passed to the callback.
 * @param [in] callback Callback function to be fired when the timer expires.
 */
FreeRTOSTimer::FreeRTOSTimer(char* name, TickType_t period, UBaseType_t	reload, void* data, void (*callback)(FreeRTOSTimer* pTimer)) {
	/*
	 * The callback function to actually be called is saved as member data in the object and
	 * a static callback function is called.  This will be passed the FreeRTOS timer handle
	 * which is used as a key in a map to lookup the saved user supplied callback which is then
	 * actually called.
	 */

	assert(callback != nullptr);
	this->period = period;
	this->callback = callback;
	timerHandle = ::xTimerCreate(name, period, reload, data, internalCallback);

	// Add the association between the timer handle and this class instance into the map.
	timersMap.insert(std::make_pair(timerHandle, this));
} // FreeRTOSTimer

/**
 * @brief Destroy a class instance.
 *
 * The timer is deleted.
 */
FreeRTOSTimer::~FreeRTOSTimer() {
	::xTimerDelete(timerHandle, portMAX_DELAY);
	timersMap.erase(timerHandle);
}

/**
 * @brief Start the timer ticking.
 */
void FreeRTOSTimer::start(TickType_t blockTime) {
	::xTimerStart(timerHandle, blockTime);
} // start

/**
 * @brief Stop the timer from ticking.
 */
void FreeRTOSTimer::stop(TickType_t blockTime) {
	::xTimerStop(timerHandle, blockTime);
} // stop

/**
 * @brief Reset the timer to the period and start it ticking.
 */
void FreeRTOSTimer::reset(TickType_t blockTime) {
	::xTimerReset(timerHandle, blockTime);
} // reset


/**
 * @brief Return the period of the timer.
 *
 * @return The period of the timer.
 */
TickType_t FreeRTOSTimer::getPeriod() {
	return period;
} // getPeriod


/**
 * @brief Change the period of the timer.
 *
 * @param [in] newPeriod The new period of the timer in ticks.
 */
void FreeRTOSTimer::changePeriod(TickType_t newPeriod, TickType_t blockTime) {
	if (::xTimerChangePeriod(timerHandle, newPeriod, blockTime) == pdPASS) {
		period = newPeriod;
	}
} // changePeriod


/**
 * @brief Get the name of the timer.
 *
 * @return The name of the timer.
 */
const char* FreeRTOSTimer::getName() {
	return ::pcTimerGetTimerName(timerHandle);
} // getName


/**
 * @brief Get the user supplied data associated with the timer.
 *
 * @return The user supplied data associated with the timer.
 */
void* FreeRTOSTimer::getData() {
	return ::pvTimerGetTimerID(timerHandle);
} // getData
