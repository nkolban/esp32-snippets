/*
 * SockServ.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */

#include <errno.h>
#include <esp_log.h>
#include <lwip/sockets.h>
#include <stdint.h>
#include <string.h>
#include <string>

#include "FreeRTOS.h"
#include "sdkconfig.h"
#include "SockServ.h"

static char tag[] = "SockServ";


/**
 * @brief Create an instance of the class.
 *
 * We won't actually start listening for clients until after the start() method has been called.
 * @param [in] port The TCP/IP port number on which we will listen for incoming connection requests.
 */
SockServ::SockServ(uint16_t port) {
	this->port = port;
	clientSock = -1;
	sock = -1;
} // SockServ


/**
 * @brief Accept an incoming connection.
 *
 * Block waiting for an incoming connection and accept it when it arrives.
 */
void SockServ::acceptTask(void *data) {

	SockServ *pSockServ = (SockServ *)data;
	struct sockaddr_in clientAddress;

	while(1) {
		socklen_t clientAddressLength = sizeof(clientAddress);
		int tempSock = ::accept(pSockServ->sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (tempSock == -1) {
			ESP_LOGE(tag, "close(): %s", strerror(errno));
		}
		ESP_LOGD(tag, "accept() - New socket");
		if (pSockServ->clientSock != -1) {
			int rc = ::close(pSockServ->clientSock);
			if (rc == -1) {
				ESP_LOGE(tag, "close(): %s", strerror(errno));
			}
		}
		pSockServ->clientSock = tempSock;
	}
} // acceptTask


/**
 * @brief Start listening for new partner connections.
 *
 * The port number on which we will listen is the one defined when the class was created.
 */
void SockServ::start() {
	sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		ESP_LOGE(tag, "socket(): %s", strerror(errno));
	}
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);
	int rc = ::bind(sock, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc == -1) {
		ESP_LOGE(tag, "bind(): %s", strerror(errno));
	}
	rc = ::listen(sock, 5);
	if (rc == -1) {
		ESP_LOGE(tag, "listen(): %s", strerror(errno));
	}
	ESP_LOGD(tag, "Now listening on port %d", port);
	FreeRTOS::startTask(acceptTask, "acceptTask", this);
} // start


/**
 * @brief Stop listening for new partner connections.
 */
void SockServ::stop() {
	int rc = ::close(sock);
	if (rc == -1) {
		ESP_LOGE(tag, "close(): %s", strerror(errno));
	}
} // stop


/**
 * @brief Send data to any connected partners.
 *
 * @param[in] data A sequence of bytes to send to the partner.
 * @param[in] length The length of the sequence of bytes to send to the partner.
 */
void SockServ::sendData(uint8_t *data, size_t length) {
	if (connectedCount() == 0) {
		return;
	}
	int rc = ::send(clientSock, data, length, 0);
	if (rc == -1) {
		ESP_LOGE(tag, "send(): %s", strerror(errno));
	}
} // sendData


/**
 * @brief Send data from a string to any connected partners.
 *
 * @param[in] str A string from which sequence of bytes will be used to send to the partner.
 */
void SockServ::sendData(std::string str) {
	sendData((uint8_t *)str.data(), str.size());
} // sendData


/**
 * @brief Determine the number of connected partners.
 *
 * @return The number of connected partners.
 */
int SockServ::connectedCount() {
	if (clientSock == -1) {
		return 0;
	}
	return 1;
} // connectedCount


/**
 * @brief Disconnect any connected partners.
 */
void SockServ::disconnect() {
	if (clientSock == -1) {
		int rc = ::close(clientSock);
		if (rc == -1) {
			ESP_LOGE(tag, "close(): %s", strerror(errno));
		}
		clientSock = -1;
	}
} // disconnect
