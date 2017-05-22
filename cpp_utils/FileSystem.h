/*
 * FileSystem.h
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_FILESYSTEM_H_
#define COMPONENTS_CPP_UTILS_FILESYSTEM_H_
#include <string>
/**
 * @brief File system utilities.
 */
class FileSystem {
public:
	FileSystem();
	virtual ~FileSystem();
	static void dumpDirectory(std::string path);
	static int mkdir(std::string path);
};

#endif /* COMPONENTS_CPP_UTILS_FILESYSTEM_H_ */
