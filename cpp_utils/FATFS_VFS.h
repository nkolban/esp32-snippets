/*
 * FATFS.h
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_FATFS_VFS_H_
#define COMPONENTS_CPP_UTILS_FATFS_VFS_H_
#include <string>
extern "C" {
#include <esp_vfs_fat.h>
}
/**
 * @brief Provide access to the FAT file system on %SPI flash.
 *
 * A typical example would be:
 *
 * @code{.cpp}
 * FATFS_VFS *fs = new FATFS_VFS("/spiflash", "storage");
 * fs->mount();
 * // Perform file I/O
 * fs->unmount();
 * delete fs;
 * @endcode
 *
 */
class FATFS_VFS {
public:
	FATFS_VFS(std::string mountPath, std::string partitionName);
	virtual ~FATFS_VFS();
	void mount();
	void setMaxFiles(int maxFiles);
	void unmount();
private:
	wl_handle_t m_wl_handle;
	std::string m_mountPath;
	std::string m_partitionName;
	int m_maxFiles;
};

#endif /* COMPONENTS_CPP_UTILS_FATFS_VFS_H_ */
