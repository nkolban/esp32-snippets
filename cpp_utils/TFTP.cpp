/*
 * TFTP.cpp
 *
 * See also:
 * * https://tools.ietf.org/html/rfc1350
 *  Created on: May 21, 2017
 *      Author: kolban
 */

#include "TFTP.h"
#include <esp_log.h>
#include "FreeRTOS.h"
#include "GeneralUtils.h"
#include <string>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "Socket.h"

#include "sdkconfig.h"

extern "C" {
	extern uint16_t lwip_ntohs(uint16_t);
	extern uint32_t lwip_ntohl(uint32_t);
	extern uint16_t lwip_htons(uint16_t);
	extern uint32_t lwip_htonl(uint32_t);
}

static const char* LOG_TAG = "TFTP";

enum opcode {
	TFTP_OPCODE_RRQ   = 1, // Read request
	TFTP_OPCODE_WRQ   = 2, // Write request
	TFTP_OPCODE_DATA  = 3, // Data
	TFTP_OPCODE_ACK   = 4, // Acknowledgement
	TFTP_OPCODE_ERROR = 5  // Error
};

enum ERRORCODE {
	ERROR_CODE_NOTDEFINED        = 0,
	ERROR_CODE_FILE_NOT_FOUND    = 1,
	ERROR_CODE_ACCESS_VIOLATION  = 2,
	ERROR_CODE_NO_SPACE          = 3,
	ERROR_CODE_ILLEGAL_OPERATION = 4,
	ERROR_CODE_UNKNOWN_ID        = 5,
	ERROR_CODE_FILE_EXISTS       = 6,
	ERROR_CODE_UNKNOWN_USER      = 7
};

/**
 * Size of the TFTP data payload.
 */
const int TFTP_DATA_SIZE = 512;

struct data_packet {
	uint16_t blockNumber;
	std::string data;
};



TFTP::TFTP() {
	m_baseDir = "";
}

TFTP::~TFTP() {
}

/**
 * @brief Start a TFTP transaction.
 * @return N/A.
 */
TFTP::TFTP_Transaction::TFTP_Transaction() {
	m_baseDir  = "";
	m_filename = "";
	m_mode     = "";
	m_opCode   = -1;
} // TFTP_Transaction


/**
 * @brief Process a client read request.
 * @return N/A.
 */

void TFTP::TFTP_Transaction::processRRQ() {
/*
 *   2 bytes     2 bytes     n bytes
 *  ----------------------------------
 * | Opcode |   Block #  |   Data     |
 *  ----------------------------------
 *
 */
	FILE* file;
	bool finished = false;

	ESP_LOGD(LOG_TAG, "Reading TFTP data from file: %s", m_filename.c_str());
	std::string tmpName = m_baseDir + "/" + m_filename;
	/*
	struct stat buf;
	if (stat(tmpName.c_str(), &buf) != 0) {
		ESP_LOGE(LOG_TAG, "Stat file: %s: %s", tmpName.c_str(), strerror(errno));
		return;
	}
	int length = buf.st_size;
	*/

	int blockNumber = 1;

	file = fopen(tmpName.c_str(), "r");
	if (file == nullptr) {
		ESP_LOGE(LOG_TAG, "Failed to open file for reading: %s: %s", tmpName.c_str(), strerror(errno));
		sendError(ERROR_CODE_FILE_NOT_FOUND, tmpName);
		return;
	}

	struct {
		uint16_t opCode;
		uint16_t blockNumber;
		uint8_t buf[TFTP_DATA_SIZE];
	} record;

	record.opCode = htons(TFTP_OPCODE_DATA); // Set the op code to be DATA.

	while (!finished) {
		record.blockNumber = htons(blockNumber);

		int sizeRead = fread(record.buf, 1, TFTP_DATA_SIZE, file);

		ESP_LOGD(LOG_TAG, "Sending data to %s, blockNumber=%d, size=%d",
				Socket::addressToString(&m_partnerAddress).c_str(), blockNumber, sizeRead);

		m_partnerSocket.sendTo((uint8_t*) &record, sizeRead + 4, &m_partnerAddress);


		if (sizeRead < TFTP_DATA_SIZE) {
			finished = true;
		} else {
			waitForAck(blockNumber);
		}
		blockNumber++; // Increment the block number.
	}
	ESP_LOGD(LOG_TAG, "File sent");
} // processRRQ


/**
 * @brief Process a client write request.
 * @return N/A.
 */
void TFTP::TFTP_Transaction::processWRQ() {
/*
 *        2 bytes    2 bytes       n bytes
 *        ---------------------------------
 * DATA  |  03   |   Block #  |    Data    |
 *        ---------------------------------
 * The opcode for data is 0x03 - TFTP_OPCODE_DATA
 */
	struct recv_data {
		uint16_t opCode;
		uint16_t blockNumber;
		uint8_t  data;
	} *pRecv_data;

	struct sockaddr recvAddr;
	uint8_t dataBuffer[TFTP_DATA_SIZE + 2 + 2];
	bool finished = false;

	FILE* file;

	ESP_LOGD(LOG_TAG, "Writing TFTP data to file: %s", m_filename.c_str());
	std::string tmpName = m_baseDir + "/" + m_filename;
	file = fopen(tmpName.c_str(), "w");
	if (file == nullptr) {
		ESP_LOGE(LOG_TAG, "Failed to open file for writing: %s: %s", tmpName.c_str(), strerror(errno));
		return;
	}
	while(!finished) {
		pRecv_data = (struct recv_data*) dataBuffer;
		int receivedSize = m_partnerSocket.receiveFrom(dataBuffer, sizeof(dataBuffer), &recvAddr);
		if (receivedSize == -1) {
			ESP_LOGE(LOG_TAG, "rc == -1 from receive_from");
		}
		struct data_packet dp;
		dp.blockNumber = ntohs(pRecv_data->blockNumber);
		dp.data = std::string((char*) &pRecv_data->data, receivedSize - 4);
		fwrite(dp.data.data(), dp.data.length(), 1, file);
		sendAck(dp.blockNumber);
		ESP_LOGD(LOG_TAG, "Block size: %d", dp.data.length());
		if (dp.data.length() < TFTP_DATA_SIZE) {
			finished = true;
		}
	} // Finished
	fclose(file);
	m_partnerSocket.close();
} // process


/**
 * @brief Send an acknowledgment back to the partner.
 * A TFTP acknowledgment packet contains an opcode (4) and a block number.
 *
 * @param [in] blockNumber The block number to send.
 * @return N/A.
 */
void TFTP::TFTP_Transaction::sendAck(uint16_t blockNumber) {
	struct {
		uint16_t opCode;
		uint16_t blockNumber;
	} ackData;

	ackData.opCode      = htons(TFTP_OPCODE_ACK);
	ackData.blockNumber = htons(blockNumber);

	ESP_LOGD(LOG_TAG, "Sending ack to %s, blockNumber=%d", Socket::addressToString(&m_partnerAddress).c_str(), blockNumber);
	m_partnerSocket.sendTo((uint8_t*) &ackData, sizeof(ackData), &m_partnerAddress);
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
/*
 * Loop forever.  At the start of the loop we block waiting for an incoming client request.
 * The requests that we are expecting are either a request to read a file from the server
 * or write a file to the server.  Once we have received a request we then call the appropriate
 * handler to handle that type of request.  When the request has been completed, we start again.
 */
	ESP_LOGD(LOG_TAG, "Starting TFTP::start() on port %d", port);
	Socket serverSocket;
	serverSocket.listen(port, true); // Create a listening socket that is a datagram.
	while (true) {
		// This would be a good place to start a transaction in the background.
		TFTP_Transaction* pTFTPTransaction = new TFTP_Transaction();
		pTFTPTransaction->setBaseDir(m_baseDir);
		uint16_t receivedOpCode = pTFTPTransaction->waitForRequest(&serverSocket);
		switch (receivedOpCode) {
			// Handle the write request (client file upload)
			case opcode::TFTP_OPCODE_WRQ: {
				pTFTPTransaction->processWRQ();
				break;
			}

			// Handle the read request (server file download)
			case opcode::TFTP_OPCODE_RRQ: {
				pTFTPTransaction->processRRQ();
				break;
			}
			default:
				ESP_LOGE(LOG_TAG, "Unknown opcode: %d", receivedOpCode);
				break;
		}
		delete pTFTPTransaction;
	} // End while loop
} // run


/**
 * @brief Set the base dir for file access.
 * If we are asked to put a file to the file system, this is the base relative directory.
 * @param baseDir Base directory for file access.
 * @return N/A.
 */
void TFTP::TFTP_Transaction::setBaseDir(std::string baseDir) {
	m_baseDir = baseDir;
} // setBaseDir


/**
 * @brief Set the base dir for file access.
 * If we are asked to put a file to the file system, this is the base relative directory.
 * @param baseDir Base directory for file access.
 * @return N/A.
 */
void TFTP::setBaseDir(std::string baseDir) {
	m_baseDir = baseDir;
} // setBaseDir


/**
 * @brief Wait for an acknowledgment from the client.
 * After having sent data to the client, we expect an acknowledment back from the client.
 * This function causes us to wait for an incoming acknowledgment.
 */
void TFTP::TFTP_Transaction::waitForAck(uint16_t blockNumber) {
	struct {
		uint16_t opCode;
		uint16_t blockNumber;
	} ackData;

	ESP_LOGD(LOG_TAG, "TFTP: Waiting for an acknowledgment request");
	int sizeRead = m_partnerSocket.receiveFrom((uint8_t*) &ackData, sizeof(ackData), &m_partnerAddress);
	ESP_LOGD(LOG_TAG, "TFTP: Received some data.");

	if (sizeRead != sizeof(ackData)) {
		ESP_LOGE(LOG_TAG, "waitForAck: Received %d but expected %d", sizeRead, sizeof(ackData));
		sendError(ERROR_CODE_NOTDEFINED, "Ack not correct size");
		return;
	}

	ackData.opCode      = ntohs(ackData.opCode);
	ackData.blockNumber = ntohs(ackData.blockNumber);

	if (ackData.opCode != opcode::TFTP_OPCODE_ACK) {
		ESP_LOGE(LOG_TAG, "waitForAck: Received opcode %d but expected %d", ackData.opCode, opcode::TFTP_OPCODE_ACK);
		return;
	}

	if (ackData.blockNumber != blockNumber) {
		ESP_LOGE(LOG_TAG, "waitForAck: Blocknumber received %d but expected %d", ackData.blockNumber, blockNumber);
		return;
	}
} // waitForAck


/**
 * @brief Wait for a client request.
 * A %TFTP server waits for requests to send or receive files.  A request can be
 * either WRQ (write request) which is a request from the client to write a new local
 * file or it can be a RRQ (read request) which is a request from the client to
 * read a local file.
 * @param pServerSocket The server socket on which to listen for client requests.
 * @return The op code received.
 */
/*
 *        2 bytes    string   1 byte     string   1 byte
 *        -----------------------------------------------
 * RRQ/  | 01/02 |  Filename  |   0  |    Mode    |   0  |
 * WRQ    -----------------------------------------------
 */
uint16_t TFTP::TFTP_Transaction::waitForRequest(Socket* pServerSocket) {
	union {
		uint8_t buf[TFTP_DATA_SIZE];
		uint16_t opCode;
	} record;
	size_t length = 100;

	ESP_LOGD(LOG_TAG, "TFTP: Waiting for a request");
	pServerSocket->receiveFrom(record.buf, length, &m_partnerAddress);

	// Save the filename, mode and op code.

	m_filename = std::string((char*) (record.buf + 2));
	m_mode	 = std::string((char*) (record.buf + 3 + m_filename.length()));
	m_opCode   = ntohs(record.opCode);
	switch (m_opCode) {
		// Handle the Write Request command.
		case TFTP_OPCODE_WRQ: {
			m_partnerSocket.createSocket(true);
			m_partnerSocket.bind(0, INADDR_ANY);
			sendAck(0);
			break;
		}

		// Handle the Read request command.
		case TFTP_OPCODE_RRQ: {
			m_partnerSocket.createSocket(true);
			m_partnerSocket.bind(0, INADDR_ANY);
			break;
		}

		default: {
			ESP_LOGD(LOG_TAG, "Un-handled opcode: %d", m_opCode);
			break;
		}
	}
	return m_opCode;
} // waitForRequest

/**
 * @brief Send an error indication to the client.
 * @param [in] code Error code to send to the client.
 * @param [in] message Explanation message.
 * @return N/A.
 */
void TFTP::TFTP_Transaction::sendError(uint16_t code, std::string message) {
/*
 *  2 bytes     2 bytes      string    1 byte
 *  -----------------------------------------
 * | Opcode |  ErrorCode |   ErrMsg   |   0  |
 *  -----------------------------------------
 */
	int size = 2  + 2 + message.length() + 1;
	uint8_t* buf = (uint8_t*) malloc(size);
	*(uint16_t*) (&buf[0]) = htons(opcode::TFTP_OPCODE_ERROR);
	*(uint16_t*) (&buf[2]) = htons(code);
	strcpy((char*) (&buf[4]), message.c_str());
	m_partnerSocket.sendTo(buf, size, &m_partnerAddress);
	free(buf);
} // sendError
