/*
 * Socket.cpp
 *
 *  Created on: Mar 5, 2017
 *      Author: kolban
 */

#include "Socket.h"
#include <unistd.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

Socket::Socket() {
	sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

Socket::~Socket() {
	close_cpp();
}

void Socket::close_cpp() {
	::close(sock);
}

void Socket::connect_cpp(struct in_addr address, uint16_t port) {
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr   = address;
	serverAddress.sin_port   = htons(port);

	int rc = ::connect(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
	if (rc == -1) {
		printf("connect_cpp: Error: %s\n", strerror(errno));
	}
}

int Socket::receive_cpp(uint8_t* data, size_t length) {
	int rc = ::recv(sock, data, length, 0);
	return rc;
}

void Socket::send_cpp(const uint8_t* data, size_t length) {
	::send(sock, data, length, 0);
}

void Socket::send_cpp(std::string value) {
	send_cpp((uint8_t *)value.data(), value.size());
}
