/*
 * FATFS.cpp
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */

#include "FATFS_VFS.h"
#include <esp_err.h>

#include <esp_vfs_fat.h>


/**
 * @brief Constructor.
 *
 * In ESP32, every file system has a mount point.  If a file access is attempted south of that
 * mount point, then the corresponding file system will be used.  The `mountPath` parameter defines
 * the mount point for this instance of the FATFS file system.
 * In order to save the files for subsequent retrieval, the file data has to be written to flash memory.
 * A partition table provides a map or layout of the flash memory by defining named partitions.  The
 * `partitionName` parameter defines the name of the partition used to provide the underlying storage
 * for this instance of the FATFS file system.
 *
 * @param [in] mountPath The path in the VFS where the FAT file system should be mounted.
 * @param [in] partitionName The name of the partition used to store the FAT file system.
 */
FATFS_VFS::FATFS_VFS(std::string mountPath, std::string partitionName) {
	m_mountPath     = mountPath;
	m_partitionName = partitionName;
	m_maxFiles      = 4;
	m_wl_handle     = WL_INVALID_HANDLE;
} // FATFS_VFS


FATFS_VFS::~FATFS_VFS() {
} // ~FATFS_VFS


/**
 * @brief Mount the FAT file system into VFS.
 * The FAT file system found in the partition is mounted into the VFS.
 * @return N/A.
 */
void FATFS_VFS::mount() {
	esp_vfs_fat_mount_config_t mountConfig;
	mountConfig.max_files = m_maxFiles;
	mountConfig.format_if_mount_failed = true;
	ESP_ERROR_CHECK(esp_vfs_fat_spiflash_mount(m_mountPath.c_str(), m_partitionName.c_str(), &mountConfig, &m_wl_handle));
} // mount


/**
 * @brief Set the allowable number of concurrently open files.
 * @param [in] maxFiles Number of concurrently open files.
 * @return N/A.
 */
void FATFS_VFS::setMaxFiles(int maxFiles) {
	m_maxFiles = maxFiles;
} // setMaxFiles


/**
 * @brief Unmount a previously mounted file system.
 * @return N/A.
 */
void FATFS_VFS::unmount() {
	ESP_ERROR_CHECK(esp_vfs_fat_spiflash_unmount(m_mountPath.c_str(), m_wl_handle));
} // unmount
