/*
 * GeneralUtils.h
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_GENERALUTILS_H_
#define COMPONENTS_CPP_UTILS_GENERALUTILS_H_
#include <stdint.h>

/**
 * @brief General utilities.
 */
class GeneralUtils {
public:
	GeneralUtils();
	virtual ~GeneralUtils();
	static void hexDump(uint8_t *pData, uint32_t length);
};

#endif /* COMPONENTS_CPP_UTILS_GENERALUTILS_H_ */
