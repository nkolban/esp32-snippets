/*
 * TFTP.h
 *
 *  Created on: May 21, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_TFTP_H_
#define COMPONENTS_CPP_UTILS_TFTP_H_
#define TFTP_DEFAULT_PORT (69)
#include <string>
/**
 * @brief A %TFTP server.
 *
 * The Trivial File Transfer Protocol is a specification for simple file transfer
 * without the richness of implementation of full FTP.  It is very easy to implement
 * both a server and a client.  The protocol leverages UDP as opposed to connection
 * oriented (TCP).  The specification can be found <a href="https://tools.ietf.org/html/rfc1350">here</a>.
 *
 * Here is an example fragment which mounts a file system and then starts a %TFTP server
 * to provide access to its content.
 *
 * @code{.cpp}
 * FATFS_VFS fs("/spiflash", "storage");
 * fs.mount();
 * TFTP tftp;
 * tftp.setBaseDir("/spiflash");
 * tftp.start();
 * @endcode
 *
 * On Linux, I recommend the <a href="https://linux.die.net/man/1/atftp">atftp</a> client.
 */
class TFTP {
public:
	TFTP();
	virtual ~TFTP();
	void start(uint16_t port=TFTP_DEFAULT_PORT);
	void setBaseDir(std::string baseDir);
private:
	std::string m_baseDir;
};

#endif /* COMPONENTS_CPP_UTILS_TFTP_H_ */
