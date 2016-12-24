#include "spiffs.h"
#include <esp_vfs.h>
#include <esp_log.h>
#include <fcntl.h>
#include <errno.h>
#include "spiffs.h"
#include "sdkconfig.h"

static char tag[] = "spiffs_vfs";

static char *spiffsErrorToString(int code) {
	static char msg[10];
	switch(code) {
	case SPIFFS_OK:
		return "SPIFFS_OK";
	case SPIFFS_ERR_NOT_MOUNTED:
		return "SPIFFS_ERR_NOT_MOUNTED";
	case SPIFFS_ERR_FULL:
		return "SPIFFS_ERR_FULL";
	case SPIFFS_ERR_NOT_FOUND:
		return "SPIFFS_ERR_NOT_FOUND";
	case SPIFFS_ERR_END_OF_OBJECT:
		return "SPIFFS_ERR_END_OF_OBJECT";
	case SPIFFS_ERR_DELETED:
		return "SPIFFS_ERR_DELETED";
	case SPIFFS_ERR_FILE_CLOSED:
		return "SPIFFS_ERR_FILE_CLOSED";
	case SPIFFS_ERR_FILE_DELETED:
		return "SPIFFS_ERR_FILE_DELETED";
	case SPIFFS_ERR_BAD_DESCRIPTOR:
		return "SPIFFS_ERR_BAD_DESCRIPTOR";
	case SPIFFS_ERR_NOT_A_FS:
		return "SPIFFS_ERR_NOT_A_FS";
	case SPIFFS_ERR_FILE_EXISTS:
		return "SPIFFS_ERR_FILE_EXISTS";
	}
	sprintf(msg, "%d", code);
	return msg;
}

/*
static int spiffsErrMap(spiffs *fs) {
	int errorCode = SPIFFS_errno(fs);
	switch (errorCode) {
		case SPIFFS_ERR_FULL:
			return ENOSPC;
		case SPIFFS_ERR_NOT_FOUND:
			return ENOENT;
		case SPIFFS_ERR_FILE_EXISTS:
			return EEXIST;
		case SPIFFS_ERR_NOT_A_FILE:
			return EBADF;
		case SPIFFS_ERR_OUT_OF_FILE_DESCS:
			return ENFILE;
		default: {
			ESP_LOGE(tag, "We received SPIFFs error code %d but didn't know how to map to an errno", errorCode);
			return ENOMSG;
		}
	}
} // spiffsErrMap
*/


/**
 * Log the flags that are specified in an open() call.
 */
static void logFlags(int flags) {
	ESP_LOGD(tag, "flags:");
	if (flags & O_APPEND) {
		ESP_LOGD(tag, "- O_APPEND");
	}
	if (flags & O_CREAT) {
		ESP_LOGD(tag, "- O_CREAT");
	}
	if (flags & O_TRUNC) {
		ESP_LOGD(tag, "- O_TRUNC");
	}
	if (flags & O_RDONLY) {
		ESP_LOGD(tag, "- O_RDONLY");
	}
	if (flags & O_WRONLY) {
		ESP_LOGD(tag, "- O_WRONLY");
	}
	if (flags & O_RDWR) {
		ESP_LOGD(tag, "- O_RDWR");
	}
} // End of logFlags


static size_t vfs_write(void *ctx, int fd, const void *data, size_t size) {
	ESP_LOGI(tag, ">> write fd=%d, data=0x%lx, size=%d", fd, (unsigned long)data, size);
	spiffs *fs = (spiffs *)ctx;
	size_t retSize = SPIFFS_write(fs, (spiffs_file)fd, (void *)data, size);
	return retSize;
} // vfs_write


static off_t vfs_lseek(void *ctx, int fd, off_t offset, int whence) {
	ESP_LOGI(tag, ">> lseek fd=%d, offset=%d, whence=%d", fd, (int)offset, whence);
	return 0;
} // vfs_lseek


static ssize_t vfs_read(void *ctx, int fd, void *dst, size_t size) {
	ESP_LOGI(tag, ">> read fd=%d, dst=0x%lx, size=%d", fd, (unsigned long)dst, size);
	spiffs *fs = (spiffs *)ctx;
	ssize_t retSize = SPIFFS_read(fs, (spiffs_file)fd, dst, size);
	return retSize;
} // vfs_read


/**
 * Open the file specified by path.  The flags contain the instructions
 * on how the file is to be opened.  For example:
 *
 * O_CREAT  - Create the named file.
 * O_TRUNC  - Truncate (empty) the file.
 * O_RDONLY - Open the file for reading only.
 * O_WRONLY - Open the file for writing only.
 * O_RDWR   - Open the file for reading and writing.
 * O_APPEND - Append to the file.
 *
 * The mode are access mode flags.
 */
static int vfs_open(void *ctx, const char *path, int flags, int accessMode) {
	ESP_LOGI(tag, ">> open path=%s, flags=0x%x, accessMode=0x%x", path, flags, accessMode);
	logFlags(flags);
	spiffs *fs = (spiffs *)ctx;
	int spiffsFlags = 0;
	if (flags & O_CREAT) {
		spiffsFlags |= SPIFFS_O_CREAT;
	}
	if (flags & O_TRUNC) {
		spiffsFlags |= SPIFFS_O_TRUNC;
	}
	if (flags & O_RDONLY) {
		spiffsFlags |= SPIFFS_O_RDONLY;
	}
	if (flags & O_WRONLY) {
		spiffsFlags |= SPIFFS_O_WRONLY;
	}
	if (flags & O_RDWR) {
		spiffsFlags |= SPIFFS_O_RDWR;
	}
	if (flags & O_APPEND) {
		spiffsFlags |= SPIFFS_O_APPEND;
	}
	int rc = SPIFFS_open(fs, path, spiffsFlags, accessMode);
	return rc;
} // vfs_open


static int vfs_close(void *ctx, int fd) {
	ESP_LOGI(tag, ">> close fd=%d", fd);
	spiffs *fs = (spiffs *)ctx;
	int rc = SPIFFS_close(fs, (spiffs_file)fd);
	return rc;
} // vfs_close


static int vfs_fstat(void *ctx, int fd, struct stat *st) {
	ESP_LOGI(tag, ">> fstat fd=%d", fd);
	return 0;
} // vfs_fstat


static int vfs_stat(void *ctx, const char *path, struct stat *st) {
	ESP_LOGI(tag, ">> stat path=%s", path);
	return 0;
} // vfs_stat


static int vfs_link(void *ctx, const char *oldPath, const char *newPath) {
	ESP_LOGI(tag, ">> link oldPath=%s, newPath=%s", oldPath, newPath);
	return 0;
} // vfs_link

static int vfs_unlink(void *ctx, const char *path, ) {
	ESP_LOGI(tag, ">> unlink path=%s", path);
	spiffs *fs = (spiffs *)ctx;
	SPIFFS_remove(fs, path);
	return 0;
} // vfs_unlink


static int vfs_rename(void *ctx, const char *oldPath, const char *newPath) {
	ESP_LOGI(tag, ">> rename oldPath=%s, newPath=%s", oldPath, newPath);
	spiffs *fs = (spiffs *)ctx;
	int rc = SPIFFS_rename(fs, oldPath, newPath);
	return rc;
} // vfs_rename


/**
 * Register the VFS at the specified mount point.
 * The callback functions are registered to handle the
 * different functions that may be requested against the
 * VFS.
 */
void spiffs_registerVFS(char *mountPoint, spiffs *fs) {
	esp_vfs_t vfs;
	esp_err_t err;

	vfs.fd_offset = 0;
	vfs.flags = ESP_VFS_FLAG_CONTEXT_PTR;
	vfs.write_p  = vfs_write;
	vfs.lseek_p  = vfs_lseek;
	vfs.read_p   = vfs_read;
	vfs.open_p   = vfs_open;
	vfs.close_p  = vfs_close;
	vfs.fstat_p  = vfs_fstat;
	vfs.stat_p   = vfs_stat;
	vfs.link_p   = vfs_link;
	vfs.ulink_p  = vfs_ulink;
	vfs.rename_p = vfs_rename;

	err = esp_vfs_register(mountPoint, &vfs, (void *)fs);
	if (err != ESP_OK) {
		ESP_LOGE(tag, "esp_vfs_register: err=%d", err);
	}
} // spiffs_registerVFS
