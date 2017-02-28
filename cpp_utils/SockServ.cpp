/*
 * SockServ.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */

#include <esp_log.h>
#include <lwip/sockets.h>
#include <stdint.h>
#include <string>
//#include "/opt/xtensa-esp32-elf/xtensa-esp32-elf/include/c++/5.2.0/string"

#include "FreeRTOS.h"
#include "SockServ.h"
#include "sdkconfig.h"

static char tag[] = "SockServ";

/**
 *
 */
void SockServ::acceptTask(void *data) {

	SockServ *pSockServ = (SockServ *)data;
	struct sockaddr_in clientAddress;

	while(1) {
		socklen_t clientAddressLength = sizeof(clientAddress);
		int tempSock = ::accept(pSockServ->sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		ESP_LOGD(tag, "accept() - New socket");
		if (pSockServ->clientSock != -1) {
			::close(pSockServ->clientSock);
		}
		pSockServ->clientSock = tempSock;
	}
}


SockServ::SockServ(uint16_t port) {
	this->port = port;
	clientSock = -1;
}


/**
 * Start listening for new partner connections.
 */
void SockServ::start() {
	sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);
	::bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	::listen(sock, 5);
	ESP_LOGD(tag, "Now listening on port %d", port);
	FreeRTOS::startTask(acceptTask, "acceptTask", this);
}

/**
 * Stop listening for new partner connections.
 */
void SockServ::stop() {
	::close(sock);
}

/**
 * Send data to any connected partners.
 * @param[in] data A sequence of bytes to send to the partner.
 * @param[in] length The length of the sequence of bytes to send to the partner.
 */
void SockServ::sendData(uint8_t *data, size_t length) {
	if (clientSock == -1) {
		return;
	}
	::send(clientSock, data, length, 0);
}


/**
 * Send data from a string to any connected partners.
 * @param[in] str A string from which sequence of bytes will be used to send to the partner.
 */
void SockServ::sendData(std::string str) {
	sendData((uint8_t *)str.data(), str.size());
}
