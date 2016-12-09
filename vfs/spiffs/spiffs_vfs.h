/*
 * spiffs_vfs.h
 *
 *  Created on: Dec 3, 2016
 *      Author: kolban
 */

#ifndef MAIN_SPIFFS_VFS_H_
#define MAIN_SPIFFS_VFS_H_
#include "spiffs.h"
void spiffs_registerVFS(char *mountPoint, spiffs *fs);


#endif /* MAIN_SPIFFS_VFS_H_ */
