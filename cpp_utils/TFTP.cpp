/*
 * TFTP.cpp
 *
 *  Created on: May 21, 2017
 *      Author: kolban
 */

#include "TFTP.h"
#include <esp_log.h>
#include <FreeRTOS.h>
#include <GeneralUtils.h>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <Socket.h>

#include "sdkconfig.h"

static char tag[] = "TFTP";

enum opcode {
	TFTP_OPCODE_RRQ   = 1,
	TFTP_OPCODE_WRQ   = 2,
	TFTP_OPCODE_DATA  = 3,
	TFTP_OPCODE_ACK   = 4,
	TFTP_OPCODE_ERROR = 5
};

struct data_packet {
	uint16_t blockNumber;
	std::string data;
};



TFTP::TFTP() {
	m_baseDir = "";
}

TFTP::~TFTP() {
	// TODO Auto-generated destructor stub
}

TFTP::TFTP_Transaction::TFTP_Transaction() {
	m_baseDir = "";
	m_filename = "";
	m_mode = "";
	m_opCode = -1;
}

/**
 * @brief Wait for a server request.
 * A %TFTP server waits for requests to send or receive files.  A request can be
 * either WRQ (write request) which is a request from the client to write a new local
 * file or it can be a RRQ (read request) which is a request from the clien to
 * read a local file.
 * @param pServerSocket The server socket on which to listen for requests.
 * @return N/A.
 */
void TFTP::TFTP_Transaction::waitForRequest(Socket *pServerSocket) {
/*
 *        2 bytes    string   1 byte     string   1 byte
 *        -----------------------------------------------
 * RRQ/  | 01/02 |  Filename  |   0  |    Mode    |   0  |
 * WRQ    -----------------------------------------------
 */
	uint8_t buf[512];
	size_t length = 100;
	ESP_LOGD(tag, "TFTP: Waiting for a request");
	pServerSocket->receiveFrom_cpp(buf, length, &m_partnerAddress);
	m_filename = std::string((char *)(buf+2));
	m_mode = std::string((char *)(buf + 3 + m_filename.length()));
	m_opCode = ntohs(*(uint16_t *)buf);
	switch(m_opCode) {
		case TFTP_OPCODE_WRQ:
			m_partnerSocket.createSocket_cpp(true);
			m_partnerSocket.bind_cpp(0, INADDR_ANY);
			sendAck(0);
			break;
		default:
			ESP_LOGD(tag, "Un-handled opcode: %d", m_opCode);
			break;
	}
} // waitForRequest

/**
 * @brief Process a partner request ...
 * @return N/A.
 */
void TFTP::TFTP_Transaction::process() {
/*
 *        2 bytes    2 bytes       n bytes
 *        ---------------------------------
 * DATA  | 03    |   Block #  |    Data    |
 *        ---------------------------------
 * The opcode for data is 0x03 - TFTP_OPCODE_DATA
 */
	struct recv_data {
		uint16_t opCode;
		uint16_t blockNumber;
		uint8_t data;
	} *pRecv_data;
	struct sockaddr recvAddr;
	uint8_t dataBuffer[512+2+2];
	bool finished = false;

	FILE *file;

	ESP_LOGD(tag, "Writing to file: %s", m_filename.c_str());
	std::string tmpName = m_baseDir + "/" + m_filename;
	file = fopen(tmpName.c_str(), "w");
	if (file == nullptr) {
		ESP_LOGE(tag, "Failed to open file %s: %s", tmpName.c_str(), strerror(errno));
		return;
	}
	while(!finished) {
		pRecv_data = (struct recv_data *)dataBuffer;
		int rc = m_partnerSocket.receiveFrom_cpp(dataBuffer, sizeof(dataBuffer), &recvAddr);
		if (rc == -1) {
			ESP_LOGE(tag, "rc == -1 from receive_from");
		}
		struct data_packet dp;
		dp.blockNumber = ntohs(pRecv_data->blockNumber);
		dp.data = std::string((char *)&pRecv_data->data, rc-4);
		fwrite(dp.data.data(), dp.data.length(), 1, file);
		sendAck(dp.blockNumber);
		ESP_LOGD(tag, "Block size: %d", dp.data.length());
		if (dp.data.length() < 512) {
			finished = true;
		}
	}
	fclose(file);
	m_partnerSocket.close_cpp();
} // process

/**
 * @brief Send an acknowledgement back to the partner.
 * A TFTP acknowledgement packet contains an opcode (4) and a block number.
 *
 * @param [in] pSocket The socket to use to send the response.
 * @param [in] addr The address of the partner.
 * @param [in] blockNumber The block number to send.
 */
void TFTP::TFTP_Transaction::sendAck(uint16_t blockNumber) {
	struct {
		uint16_t opCode;
		uint16_t blockNumber;
	} ackData;

	ackData.opCode      = htons(TFTP_OPCODE_ACK);
	ackData.blockNumber = htons(blockNumber);

	ESP_LOGD(tag, "Sending ack to %s, blockNumber=%d", Socket::addressToString(&m_partnerAddress).c_str(), blockNumber);
	m_partnerSocket.sendTo_cpp((uint8_t *)&ackData, sizeof(ackData), &m_partnerAddress);
} // sendAck

/**
 * @brief Start being a TFTP server.
 *
 * This function does not return.
 *
 * @param [in] port The port number on which to listen.  The default is 69.
 * @return N/A.
 */
void TFTP::start(uint16_t port) {
	ESP_LOGD(tag, "Starting TFTP::start()");
	Socket serverSocket;
	serverSocket.listen_cpp(port, true); // Create a listening socket that is a datagram.
	while(true) {
		// This would be a good place to start a transaction in the background.
		TFTP_Transaction *pTFTPTransaction = new TFTP_Transaction();
		pTFTPTransaction->setBaseDir(m_baseDir);
		pTFTPTransaction->waitForRequest(&serverSocket);
		pTFTPTransaction->process();
		delete pTFTPTransaction;
	}
} // run

/**
 * @brief Set the base dir for file access.
 * If we are asked to put a file to the file system, this is the base relative directory.
 * @param baseDir Base directory for file access.
 * @return N/A.
 */
void TFTP::TFTP_Transaction::setBaseDir(std::string baseDir) {
	m_baseDir = baseDir;
}

/**
 * @brief Set the base dir for file access.
 * If we are asked to put a file to the file system, this is the base relative directory.
 * @param baseDir Base directory for file access.
 * @return N/A.
 */
void TFTP::setBaseDir(std::string baseDir) {
	m_baseDir = baseDir;
}
