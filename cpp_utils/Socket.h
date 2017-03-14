/*
 * Socket.h
 *
 *  Created on: Mar 5, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SOCKET_H_
#define COMPONENTS_CPP_UTILS_SOCKET_H_
#include <lwip/inet.h>
#include <string>

class Socket {
public:
	Socket();
	virtual ~Socket();
	void close_cpp();
	int connect_cpp(struct in_addr address, uint16_t port);
	int connect_cpp(char *address, uint16_t port);
	int receive_cpp(uint8_t *data, size_t length);
	void send_cpp(const uint8_t *data, size_t length);
	void send_cpp(std::string value);
private:
	int sock;
};

#endif /* COMPONENTS_CPP_UTILS_SOCKET_H_ */
