/*
 * c_timeutils.c
 *
 *  Created on: Nov 26, 2016
 *      Author: kolban
 */
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Add a number of milliseconds to a timeval and replace the
 * existing timeval with the new value.
 */
void timeval_addMsecs(struct timeval *a, uint32_t msecs) {
	int uSecs = (msecs%1000) * 1000 + a->tv_usec;
	a->tv_usec = uSecs % 1000000;
	a->tv_sec += msecs/1000 + uSecs/1000000;
} // addMsecs

/**
 * Convert a timeval into the number of msecs.
 */
uint32_t timeval_toMsecs(struct timeval *a) {
	return a->tv_sec * 1000 + a->tv_usec/1000;
} // timeval_toMsecs


/**
 * Subtract one timeval from another.
 */
struct timeval timeval_sub(struct timeval *a, struct timeval *b) {
	struct timeval result;
	result.tv_sec = a->tv_sec - b->tv_sec;
	result.tv_usec = a->tv_usec - b->tv_usec;
	if (a->tv_usec < b->tv_usec) {
		result.tv_sec -= 1;
		result.tv_usec += 1000000;
	}
	return result;
} // timeval_sub


/*
 * Add one timeval to another.
 */
struct timeval timeval_add(struct timeval *a, struct timeval *b) {
	struct timeval result;
	result.tv_sec = a->tv_sec + b->tv_sec;
	result.tv_usec = a->tv_usec + b->tv_usec;
	if (result.tv_usec >= 1000000) {
		result.tv_sec += 1;
		result.tv_usec -= 1000000;
	}
	return result;
} // timeval_add

/**
 * Return the number of milliseconds until  future time value.
 */
uint32_t timeval_durationFromNow(struct timeval *a) {
	struct timeval b;
	gettimeofday(&b, NULL);
	struct timeval delta = timeval_sub(a, &b);
	if (delta.tv_sec < 0) {
		return 0;
	}
	return timeval_toMsecs(&delta);
	// assuming that a is later than b, then the result is a-b
} // timeval_durationFromNow

/**
 * Return the number of milliseconds the historic time value was before now.
 */
uint32_t timeval_durationBeforeNow(struct timeval *a) {
	struct timeval b;
	gettimeofday(&b, NULL);
	struct timeval delta = timeval_sub(&b, a);
	if (delta.tv_sec < 0) {
		return 0;
	}
	return timeval_toMsecs(&delta);
} // timeval_durationBeforeNow
