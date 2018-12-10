/*
 * MMU.cpp
 *
 *  Created on: Jun 30, 2018
 *      Author: kolban
 */

#include "MMU.h"
#include <stdio.h>
#include <soc/dport_reg.h>
#include <rom/cache.h>

// The following functions are provided by spi_flash.h
extern "C" {
	extern void spi_flash_disable_interrupts_caches_and_other_cpu();

	// Enable cache, enable interrupts (to be added in future), resume scheduler
	extern void spi_flash_enable_interrupts_caches_and_other_cpu();
}


typedef struct {
	uint32_t low;
	uint32_t high;
} addressRange_t;

static addressRange_t entryNumberToAddressRange(uint32_t entryNumber) {
	addressRange_t ret;
	if (entryNumber < 64) {
		ret.low = 0x3F400000 + 64 * 1024 * entryNumber;
		ret.high = 0x3F400000 + 64 * 1024 * (entryNumber + 1) - 1;
		return ret;
	}
	ret.low = 0x40000000 + 64 * 1024 * (entryNumber - 64);
	ret.high = 0x40000000 + 64 * 1024 * (entryNumber + 1 - 64) - 1;
	return ret;
}


static uint32_t flashPageToOffset(uint32_t page) {
	return page * 64 * 1024;
}



/* static */ void MMU::dump() {
	const uint32_t mappingInvalid = 1 << 8;

	printf("PRO CPU MMU\n");
	for (uint16_t i = 0; i < 256; i++) {
		if (!(DPORT_PRO_FLASH_MMU_TABLE[i] & mappingInvalid)) {
			addressRange_t addressRange = entryNumberToAddressRange(i);
			printf("Entry: %2d (0x%8.8x - 0x%8.8x), Page: %d - offset: 0x%x\n",
				i,
				addressRange.low, addressRange.high,
				DPORT_PRO_FLASH_MMU_TABLE[i] & 0xff,
				flashPageToOffset(DPORT_PRO_FLASH_MMU_TABLE[i] & 0xff));
		}
	}
	printf("\n");
	printf("APP CPU MMU\n");
	for (uint16_t i = 0; i < 256; i++) {
		if (!(DPORT_APP_FLASH_MMU_TABLE[i] & mappingInvalid)) {
			addressRange_t addressRange = entryNumberToAddressRange(i);
			printf("Entry: %2d (0x%8.8x - 0x%8.8x), Page: %d - offset: 0x%x\n",
				i,
				addressRange.low, addressRange.high,
				DPORT_APP_FLASH_MMU_TABLE[i] & 0xff,
				flashPageToOffset(DPORT_APP_FLASH_MMU_TABLE[i] & 0xff));
		}
	}
} // MMU#dumpMMU

extern "C" {
	static void IRAM_ATTR mapFlashToVMA_Internal(uint32_t flashOffset, void* vma, size_t size);
}

static void IRAM_ATTR mapFlashToVMA_Internal(uint32_t flashOffset, void* vma, size_t size) {
	printf(">> MMU::mapFlashToVMA: flash offset: 0x%x, VMA: 0x%x, size: %d\n", flashOffset, (uint32_t)vma, size);
	uint32_t mmuEntryStart;  // The MMU table entry to start mapping.
	uint32_t mmuEntryEnd;	// The MMU table entry to end mapping.

	if ((uint32_t) vma >= 0x40000000 && (uint32_t) vma < 0x40C00000) {
		mmuEntryStart = (((uint32_t) vma - 0x40000000) / (64 * 1024)) + 64;
		mmuEntryEnd   = (((uint32_t) vma - 0x40000000 + size) / (64 * 1024)) + 64;
	}
	else if ((uint32_t) vma >= 0x3F400000 && (uint32_t) vma < 0x3F800000) {
		mmuEntryStart = (((uint32_t) vma - 0x3F400000) / (64 * 1024));
		mmuEntryEnd   = (((uint32_t) vma - 0x3F400000 + size) / (64 * 1024));
	} else {
		printf("   - Unable to map from flash to VMA.");
		return;
	}

	// At this point we have populated mmuEntryStart and mmuEntryEnd which are the MMU table entries.
	uint32_t pFlashStart = flashOffset;
	uint32_t pFlashEnd   = flashOffset + size;

	printf("   - Mapping flash to VMA via MMU.  MMU entries start: %d, end: %d, mapping flash 0x%x (flash page: %d) to 0x%x (flash page: %d)\n",
		mmuEntryStart, mmuEntryEnd, pFlashStart, pFlashStart/(64 * 1024), pFlashEnd,  pFlashEnd / (64 * 1024));

	uint32_t flashRegion = pFlashStart / (64 * 1024);   // Determine the 64K chunk of flash to be mapped (we map in units of 64K).

	spi_flash_disable_interrupts_caches_and_other_cpu();

	// For each of the mapping entries, map it to the corresponding flash region.
	for (uint32_t i = mmuEntryStart; i <= mmuEntryEnd; i++) {
		DPORT_PRO_FLASH_MMU_TABLE[i] = flashRegion;   // There are two tables.  One for the PRO CPU and one for the APP CPU.
		DPORT_APP_FLASH_MMU_TABLE[i] = flashRegion;   // Map both of them to the flash region.
		flashRegion++;
	}

	Cache_Flush(0);
	Cache_Flush(1);
	spi_flash_enable_interrupts_caches_and_other_cpu();
} // mapFlashToVMA

/**
 * Map an area of flash memory into VMA.
 * @param flashOffset The offset in flash of the start of data.
 * @param vma Where in VMA the data should appear.
 * @size How much data to map.
 */
void IRAM_ATTR MMU::mapFlashToVMA(uint32_t flashOffset, void* vma, size_t size) {
	mapFlashToVMA_Internal(flashOffset, vma, size);
}
