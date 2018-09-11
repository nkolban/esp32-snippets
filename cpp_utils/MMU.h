/*
 * MMU.h
 *
 *  Created on: Jun 30, 2018
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_MMU_H_
#define COMPONENTS_CPP_UTILS_MMU_H_
#include <esp_attr.h>
#include <stdint.h>
#include <unistd.h>

class MMU {
public:
	MMU();
	virtual ~MMU();
	static void dump();
	static void mapFlashToVMA(uint32_t flashOffset, void* vma, size_t size);
};

#endif /* COMPONENTS_CPP_UTILS_MMU_H_ */
