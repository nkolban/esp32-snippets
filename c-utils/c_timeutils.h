/*
 * c_timeutils.h
 *
 *  Created on: Nov 26, 2016
 *      Author: kolban
 */

#if !defined(_C_TIMEUTILS_H_)
#define _C_TIMEUTILS_H_
#include <sys/time.h>
#include <stdint.h>

struct timeval timeval_add(struct timeval *a, struct timeval *b);
void           timeval_addMsecs(struct timeval *a, uint32_t msecs);
uint32_t       timeval_durationBeforeNow(struct timeval *a);
uint32_t       timeval_durationFromNow(struct timeval *a);
struct timeval timeval_sub(struct timeval *a, struct timeval *b);
uint32_t       timeval_toMsecs(struct timeval *a);

#endif /* _C_TIMEUTILS_H_ */
