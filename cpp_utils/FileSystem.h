/*
 * FileSystem.h
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_FILESYSTEM_H_
#define COMPONENTS_CPP_UTILS_FILESYSTEM_H_
#include <string>
#include <vector>
#include <File.h>
/**
 * @brief File system utilities.
 */
class FileSystem {
public:
	static std::vector<File>        getDirectoryContents(std:: string path);
	static void                     dumpDirectory(std::string path);
	static bool                     isDirectory(std::string path);
	static int                      mkdir(std::string path);
	static std::vector<std::string> pathSplit(std::string path);
	static int                      remove(std::string path);
};

#endif /* COMPONENTS_CPP_UTILS_FILESYSTEM_H_ */
