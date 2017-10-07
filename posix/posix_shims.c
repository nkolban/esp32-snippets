#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

/**
 * @brief Provide a shim for the Posix access(2) function.
 * @param [in] pathname The file to check.
 * @param [in] mode The mode of access requested.
 * @return 0 on success, -1 on error with errno set.
 */
int access(const char *pathname, int mode) {
	struct stat statBuf;
	if (stat(pathname, &statBuf) == -1) {
		errno = ENOENT;
		return -1;
	}
	return 0; // Indicate that all we have access to the file.
} // access
