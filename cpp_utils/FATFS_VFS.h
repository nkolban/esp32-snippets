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
#include <wear_levelling.h>
}
/**
 * @brief Provide access to the FAT file system on %SPI flash.
 * The FATFS_VFS file system needs a partition definition.  This is a map of flash memory that
 * specified an array into which the files should be saved and loaded.  A partition is a named
 * entity and the name we choose in the partition definition should be named in the constructor.
 *
 * A partition configuration file can be described in the `make menuconfig` settings.  For example:
 * ~~~~
 * nvs,      data, nvs,     0x9000,  0x6000,
 * phy_init, data, phy,     0xf000,  0x1000,
 * factory,  app,  factory, 0x10000, 1M,
 * storage,  data, fat,     ,        1M,
 * ~~~~
 *
 * The recommended file name for the partition description is `partitions.csv`
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
	int         m_maxFiles;
};

#endif /* COMPONENTS_CPP_UTILS_FATFS_VFS_H_ */
