/*
 * File.h
 *
 *  Created on: Jun 1, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_FILE_H_
#define COMPONENTS_CPP_UTILS_FILE_H_
#include <string>
#include <dirent.h>

/**
 * @brief A logical representation of a file.
 */
class File {
public:
	File(std::string name, uint8_t type = DT_UNKNOWN);
	virtual ~File();

	std::string getContent(bool base64Encode=false);
	std::string getContent(uint32_t offset, uint32_t size);
	std::string getName();
	uint8_t getType();
	bool isDirectory();
	uint32_t length();

private:
	std::string m_name;
	uint8_t m_type;
};

#endif /* COMPONENTS_CPP_UTILS_FILE_H_ */
