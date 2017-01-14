/*
This is a simple read-only implementation of a file system. It uses a block of data coming from the
mkespfsimg tool, and can use that block to do abstracted operations on the files that are in there.
It's written for use with httpd, but doesn't need to be used as such.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


//These routines can also be tested by comping them in with the espfstest tool. This
//simplifies debugging, but needs some slightly different headers. The #ifdef takes
//care of that.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <esp_spi_flash.h>
#include <esp_log.h>
#include <esp_err.h>

#include "espfsformat.h"
#include "espfs.h"
#include "sdkconfig.h"

static char tag[] = "espfs";


struct EspFsFile {
	EspFsHeader *header;
	char decompressor;
	int32_t posDecomp;
	char *posStart;
	char *posComp;
	void *decompData;
};

static spi_flash_mmap_handle_t handle;
static void *espFlashPtr = NULL;

EspFsInitResult espFsInit(void *flashAddress, size_t size) {

	spi_flash_init();
	if (size % (64*1024) != 0) {
		ESP_LOGE(tag, "Size is not divisible by 64K.  Supplied was %d", size);
		return ESPFS_INIT_RESULT_NO_IMAGE;
	}
	esp_err_t rc = spi_flash_mmap((uint32_t) flashAddress, size, SPI_FLASH_MMAP_DATA, (const void **)&espFlashPtr, &handle);
	if (rc != ESP_OK) {
		ESP_LOGD(tag, "rc from spi_flash_mmap: %d", rc);
	}

	// check if there is valid header at address
	EspFsHeader *testHeader = (EspFsHeader *)espFlashPtr;

	if (testHeader->magic != ESPFS_MAGIC) {
		ESP_LOGE(tag, "No valid header at flash address.  Expected to find %x and found %x", ESPFS_MAGIC, testHeader->magic);
		return ESPFS_INIT_RESULT_NO_IMAGE;
	}

	return ESPFS_INIT_RESULT_OK;
}



// Returns flags of opened file.
int espFsFlags(EspFsFile *fh) {
	if (fh == NULL) {
		ESP_LOGD(tag, "File handle not ready");
		return -1;
	}

	int8_t flags;
	memcpy((char*)&flags, (char*)&fh->header->flags, 1);
	return (int)flags;
}

//Open a file and return a pointer to the file desc struct.
EspFsFile *espFsOpen(char *fileName) {
	if (espFlashPtr == NULL) {
		ESP_LOGD(tag, "Call espFsInit first!");
		return NULL;
	}
	char *flashAddress = espFlashPtr;
	char *hpos;
	char *namebuf;
	EspFsHeader *header;
	EspFsFile *fileData;
	//Strip initial slashes
	while(fileName[0] == '/') {
		fileName++;
	}
	//Go find that file!
	while(1) {
		hpos=flashAddress;
		//Grab the next file header.
		header = (EspFsHeader *)flashAddress;

		if (header->magic != ESPFS_MAGIC) {
			ESP_LOGD(tag, "Magic mismatch. EspFS image broken.");
			return NULL;
		}
		if (header->flags & FLAG_LASTFILE) {
			ESP_LOGD(tag, "End of image.  File not found.");
			return NULL;
		}
		//Grab the name of the file.
		flashAddress += sizeof(EspFsHeader);
		namebuf = (char *)flashAddress;
		if (strcmp(namebuf, fileName) == 0) {
			//Yay, this is the file we need!
			flashAddress += header->nameLen; //Skip to content.
			fileData = (EspFsFile *)malloc(sizeof(EspFsFile)); //Alloc file desc mem
			if (fileData==NULL) {
				return NULL;
			}
			fileData->header = (EspFsHeader *)hpos;
			fileData->decompressor = header->compression;
			fileData->posComp = flashAddress;
			fileData->posStart = flashAddress;
			fileData->posDecomp = 0;
			if (header->compression == COMPRESS_NONE) {
				fileData->decompData = NULL;
			} else {
				ESP_LOGD(tag, "Invalid compression: %d", header->compression);
				return NULL;
			}
			return fileData;
		}
		//We don't need this file. Skip name and file
		flashAddress += header->nameLen+header->fileLenComp;
		if ((int)flashAddress&3) {
			flashAddress += 4-((int)flashAddress & 3); //align to next 32bit val
		}
	}
}

//Read len bytes from the given file into buff. Returns the actual amount of bytes read.
int espFsRead(EspFsFile *fh, char *buff, int len) {
	int flen;
	if (fh==NULL) {
		return 0;
	}
		
	memcpy((char*)&flen, (char*)&fh->header->fileLenComp, 4);

	if (fh->decompressor == COMPRESS_NONE) {
		int toRead;
		toRead = flen-(fh->posComp-fh->posStart);
		if (len > toRead) {
			len = toRead;
		}
		ESP_LOGD(tag, "copying %d bytes from 0x%x to 0x%x", len, (uint32_t)fh->posComp, (uint32_t)buff);
		memcpy(buff, fh->posComp, len);
		fh->posDecomp += len;
		fh->posComp += len;
		return len;
	}
	return 0;
}

int espFsAccess(EspFsFile *fh, void **buf, size_t *len) {
	*buf = fh->posStart;
	*len = fh->header->fileLenComp;
	return *len;
}

//Close the file.
void espFsClose(EspFsFile *fh) {
	if (fh == NULL) return;
	free(fh);
}
