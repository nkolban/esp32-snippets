/*
 PubSubClient.cpp - A simple client for MQTT.
 Nick O'Leary
 http://knolleary.net

 edit by marcel.seerig
 */
#include "esp_log.h"

#include "PubSubClient.h"
#include "Task.h"
#include "FreeRTOS.h"
#include "FreeRTOSTimer.h"

#include "sdkconfig.h"

static const char* TAG = "PubSubClient";

#define pgm_read_byte_near(x) *(x)

/**
 * @brief A task that will handle the PubSubClient.
 *
 * This Task is started, when we have a valid connection.
 * If the connection breaks, we stop this Task.
 */
class PubSubClientTask: public Task {
public:
	PubSubClientTask(std::string name) :
		Task(name, 16 * 1024) {
		taskName = name;
	};

private:
	std::string taskName;
	/**
	 * @brief Loop over the PubSubClient.
	 * @param [in] data A pointer to an instance of the PubSubClient.
	 */
	void run(void* data) {
		PubSubClient* pPubSubClient = (PubSubClient*) data;
		ESP_LOGD("PubSubClientTask", "PubSubClientTask Task started!");

		while (true) {
			if (pPubSubClient->connected()) {
				uint16_t len = pPubSubClient->readPacket();

				if (len > 0) { // if there was data

					pPubSubClient->keepAliveTimer->reset(0); //lastInActivity = t;

					mqtt_message* msg = new mqtt_message;
					pPubSubClient->parseData(msg, len);

					//pPubSubClient->dumpData(msg);
					ESP_LOGD(TAG, "Message type (%s)!", pPubSubClient->messageType_toString(msg->type).c_str());

					if (msg->type == PUBLISH) {
						if (pPubSubClient->callback) {
							if (msg->qos == QOS0) {
								pPubSubClient->callback(msg->topic, msg->payload);
							} else if (msg->qos == QOS1) {
								pPubSubClient->callback(msg->topic, msg->payload);

								pPubSubClient->buffer[0] = PUBACK;
								pPubSubClient->buffer[1] = 2;
								pPubSubClient->buffer[2] = (msg->msgId >> 8);
								pPubSubClient->buffer[3] = (msg->msgId & 0xFF);

								int rc = pPubSubClient->_client->send(pPubSubClient->buffer, 4);
								if (rc < 0) pPubSubClient->_state = CONNECTION_LOST;

								pPubSubClient->keepAliveTimer->reset(0); //lastOutActivity = t;
							} else if(msg->qos == QOS2) {
								ESP_LOGD(TAG, "QOS2 is not supported!");
							} else {
								ESP_LOGD(TAG, "QOS-Level unkonwon yet!");
							}
						}
					} else if (msg->type == PINGREQ) {
						pPubSubClient->buffer[0] = PINGRESP;
						pPubSubClient->buffer[1] = 0;
						int rc = pPubSubClient->_client->send(pPubSubClient->buffer, 2);
						if (rc < 0) pPubSubClient->_state = CONNECTION_LOST;

					} else if (msg->type == PINGRESP) {
						pPubSubClient->PING_outstanding = false;
					} else if (msg->type == SUBACK) {
						pPubSubClient->SUBACK_outstanding = false;
						pPubSubClient->timeoutTimer->stop(0);
					} else if (msg->type == UNSUBACK) {
						pPubSubClient->UNSUBACK_Outstanding = false;
						pPubSubClient->timeoutTimer->stop(0);
					}

					delete(msg);
				}
			}
		} // while (true)
	} // run
}; // PubSubClientTask


PubSubClient::PubSubClient() {
	setup();
	this->_state = DISCONNECTED;
	this->_client = new Socket;
	setCallback(NULL);
}


PubSubClient::PubSubClient(Socket& client) {
	setup();
	this->_state = DISCONNECTED;
	setClient(client);
}


PubSubClient::PubSubClient(std::string addr, uint16_t port) {
	setup();
	this->_state = DISCONNECTED;
	this->_client = new Socket;
	setServer(addr, port);
}


PubSubClient::PubSubClient(std::string addr, uint16_t port, Socket& client) {
	setup();
	this->_state = DISCONNECTED;
	setServer(addr, port);
	setClient(client);
}


PubSubClient::PubSubClient(std::string addr, uint16_t port, MQTT_CALLBACK_SIGNATURE, Socket& client) {
	setup();
	this->_state = DISCONNECTED;
	setServer(addr, port);
	setCallback(callback);
	setClient(client);
}


PubSubClient::~PubSubClient() {
	_client->close();
	keepAliveTimer->stop(0);
	timeoutTimer->stop(0);
	m_task->stop();
	delete (_client);
	delete (keepAliveTimer);
	delete (timeoutTimer);
	delete (m_task);
}


/**
 * @brief 	This is a Timer called routine mapping routine, which calls
 * 			the PubSubClient member function keepAliveChecker.
 * @param 	The FreeRTOSTimer root instance for this callback function.
 * @return 	N/A.
 */
void keepAliveTimerMapper(FreeRTOSTimer* pTimer) {
	PubSubClient* m_pubSubClient = (PubSubClient*) pTimer->getData();
	m_pubSubClient->keepAliveChecker();
} //keepAliveChecker


/**
 * @brief 	This is a Timer called routine mapping routine, which calls
 * 			the PubSubClient member function timeoutChecker.
 * @param 	The FreeRTOSTimer root instance for this callback function.
 * @return 	N/A.
 */
void timeoutTimerMapper(FreeRTOSTimer* pTimer) {
	PubSubClient* m_pubSubClient = (PubSubClient*) pTimer->getData();
	m_pubSubClient->timeoutChecker();
} //keepAliveChecker


/**
 * @brief 	This is a internal setup routine for the PubSubClient.
 * @param 	N/A.
 * @return 	N/A.
 */
void PubSubClient::setup() {
	PING_outstanding = false;
	SUBACK_outstanding = false;
	UNSUBACK_Outstanding = false;

	keepAliveTimer = new FreeRTOSTimer((char*) "keepAliveTimer",
			(MQTT_KEEPALIVE * 1000) / portTICK_PERIOD_MS, pdTRUE, this,
			keepAliveTimerMapper);
	timeoutTimer = new FreeRTOSTimer((char*) "timeoutTimer",
				(MQTT_KEEPALIVE * 1000) / portTICK_PERIOD_MS, pdTRUE, this,
				timeoutTimerMapper);
	m_task = new PubSubClientTask("PubSubClientTask");
} // setup

/**
 * @brief 	This is a Timer called routine, which checks the PING_outstanding flag.
 * 			This flag is set in this function. We send a Keep alive message to the
 * 			server here. If there is a data in, or output, or we receive the MQTT
 * 			ping request, the flag will be set to false by other functions.
 * 			This function is called every MQTT_KEEPALIVE interval. If the flag is
 * 			still true, we have a error with the connection.
 * @param 	N/A.
 * @return 	N/A.
 */

void PubSubClient::keepAliveChecker() {
	if (PING_outstanding && connected()) {
		_state = CONNECTION_TIMEOUT;
		//_client->close();
		ESP_LOGD(TAG, "KeepAlive TIMEOUT!");
	} else {
		buffer[0] = PINGREQ;
		buffer[1] = 0;
		int rc = _client->send(buffer, 2);
		if (rc < 0) _state = CONNECTION_LOST;
		ESP_LOGD(TAG, "send KeepAlive REQUEST!");
		PING_outstanding = true;
	}
} //keepAliveChecker

/**
 * @brief 	This is a Timer called routine, which is called, when we reach the timeout.
 * 			Used is this function for all ACK commands, which comes over MQTT. Notice,
 * 			that the KeepAlivePing has his own Timer, to prevent conflicts. Basically, if
 * 			this function is called, we have detected a MQTT timeout!
 * @param 	N/A.
 * @return 	N/A.
 */
void PubSubClient::timeoutChecker() {
	if (connected() && (SUBACK_outstanding || UNSUBACK_Outstanding)) {
		_state = CONNECTION_TIMEOUT;
		//_client->close();
		ESP_LOGD(TAG, "MQTT TIMEOUT!");
	}
} //keepAliveChecker


/**
 * @brief 	Connect to a MQTT server.
 * @param 	[in] Device id to identify this device.
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::connect(const char* id) {
	return connect(id, NULL, NULL, 0, 0, 0, 0);
}


/**
 * @brief 	Connect to a MQTT server.
 * @param 	[in] Device id to identify this device.
 * 			[in] my user name.
 * 			[in] my password.
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::connect(const char* id, const char* user, const char* pass) {
	return connect(id, user, pass, 0, 0, 0, 0);
}


/**
 * @brief 	Connect to a MQTT server.
 * @param 	[in] Device id to identify this device.
 * 			[in] last will: topic name.
 * 			[in] last will: qos-level.
 * 			[in] last will: as retained message.
 * 			[in] last will: payload.
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::connect(const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) {
	return connect(id, NULL, NULL, willTopic, willQos, willRetain, willMessage);
}


/**
 * @brief 	Connect to a MQTT server.
 * @param 	[in] Device id to identify this device.
 * 			[in] my user name.
 * 			[in] my password.
 * 			[in] last will: topic name.
 * 			[in] last will: qos-level.
 * 			[in] last will: as retained message.
 * 			[in] last will: payload.
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) {
	_config.id 			= id;

	_config.user		= user;
	_config.pass		= pass;

	_config.willTopic	= willTopic;
	_config.willQos		= willQos;
	_config.willRetain	= willRetain;
	_config.willMessage	= willMessage;

	return connect();
}


/**
 * @brief 	Connect to a MQTT server with the with the previous settings.
 * 			Note: do not call this function without settings, this will not work!
 * 			For the very first connect process, use the connect function with parameters.
 * @param 	N/A
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::connect() {
	if (!connected()) {
		ESP_LOGD(TAG, "Connect to mqtt server...");
		ESP_LOGD(TAG, "ip: %s  port: %d", _config.ip.c_str(), _config.port);
		int result = _client->connect((char *)_config.ip.c_str(), _config.port);

		if (result == 0) {
			nextMsgId = 1;
			// Leave room in the buffer for header and variable length field
			uint16_t length = 5;

#if MQTT_VERSION == MQTT_VERSION_3_1
			uint8_t d[9] = { 0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTT_VERSION };
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
			uint8_t d[7] = { 0x00, 0x04, 'M', 'Q', 'T', 'T', MQTT_VERSION };
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
			for (unsigned int j = 0; j < MQTT_HEADER_VERSION_LENGTH; j++) {
				buffer[length++] = d[j];
			}

			uint8_t v;
			if (_config.willTopic) {
				v = 0x06 | (_config.willQos << 3) | (_config.willRetain << 5);
			} else {
				v = 0x02;
			}

			if (_config.user != NULL) {
				v = v | 0x80;

				if (_config.pass != NULL) {
					v = v | (0x80 >> 1);
				}
			}

			buffer[length++] = v;

			buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
			buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
			length = writeString(_config.id, buffer, length);
			if (_config.willTopic) {
				length = writeString(_config.willTopic, buffer, length);
				length = writeString(_config.willMessage, buffer, length);
			}

			if (_config.user != NULL) {
				length = writeString(_config.user, buffer, length);
				if (_config.pass != NULL) {
					length = writeString(_config.pass, buffer, length);
				}
			}

			write(CONNECT, buffer, length - 5);

			// start keepAliveTimer in 1ms...
			keepAliveTimer->start(0); //lastInActivity = lastOutActivity = millis();

			readPacket();
			uint8_t type = buffer[0] & 0xF0;

			if (type == CONNACK) {
				ESP_LOGD(TAG, "Connected to mqtt server!");

				keepAliveTimer->reset(0); //lastInActivity = millis();
				PING_outstanding = false;
				_state = CONNECTED;

				m_task->start(this);
				return true;
			} else {
				_state = (mqtt_state) buffer[3];
				ESP_LOGD(TAG, "Error: %d", _state);
			}

		} else {
			keepAliveTimer->stop(0);
			_state = CONNECT_FAILED;
		}
		return false;
	}
	return true;
}


/**
 * @brief 	Receiving a MQTT packet and store it in the buffer to parse it.
 * @param 	N/A.
 * @return 	Number of received bytes.
 */
size_t PubSubClient::readPacket() {
	size_t res = _client->receive(buffer, MQTT_MAX_PACKET_SIZE);

	if (res > MQTT_MAX_PACKET_SIZE) {
		res = 0; // This will cause the packet to be ignored.
	}

	return (uint16_t) res;
}


/**
 * @brief 	Publish a MQTT message.
 * @param 	[in] my topic.
 * 			[in] my payload.
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::publish(const char* topic, const char* payload) {
	return publish(topic, (const uint8_t*) payload, strlen(payload), false);
}


/**
 * @brief 	Publish a MQTT message.
 * @param 	[in] my topic.
 * 			[in] my payload.
 * 			[in] is this a retained message (true/false)
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::publish(const char* topic, const char* payload, bool retained) {
	return publish(topic, (const uint8_t*) payload, strlen(payload), retained);
}


/**
 * @brief 	Publish a MQTT message.
 * @param 	[in] my topic.
 * 			[in] my payload.
 * 			[in] length of the message
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::publish(const char* topic, const uint8_t* payload, unsigned int plength) {
	return publish(topic, payload, plength, false);
}


/**
 * @brief 	Publish a MQTT message.
 * @param 	[in] my topic.
 * 			[in] my payload.
 * 			[in] length of the message
 * 			[in] is this a retained message (true/false)
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::publish(const char* topic, const uint8_t* payload, unsigned int plength, bool retained) {
	if (connected()) {
		if (MQTT_MAX_PACKET_SIZE < 5 + 2 + strlen(topic) + plength) {
			// Too long
			return false;
		}
		// Leave room in the buffer for header and variable length field
		uint16_t length = 5;
		length = writeString(topic, buffer, length);
		uint16_t i;
		for (i = 0; i < plength; i++) {
			buffer[length++] = payload[i];
		}
		uint8_t header = PUBLISH;
		if (retained) {
			header |= 1;
		}
		return write(header, buffer, length - 5);
	}
	return false;
}


//bool PubSubClient::publish_P(const char* topic, const uint8_t* payload,
//		unsigned int plength, bool retained) {
//	uint8_t llen = 0;
//	uint8_t digit;
//	unsigned int rc = 0;
//	uint16_t tlen;
//	unsigned int pos = 0;
//	unsigned int i;
//	uint8_t header;
//	unsigned int len;
//
//	if (!connected()) {
//		return false;
//	}
//
//	tlen = strlen(topic);
//
//	header = PUBLISH;
//	if (retained) {
//		header |= 1;
//	}
//	buffer[pos++] = header;
//	len = plength + 2 + tlen;
//	do {
//		digit = len % 128;
//		len = len / 128;
//		if (len > 0) {
//			digit |= 0x80;
//		}
//		buffer[pos++] = digit;
//		llen++;
//	} while (len > 0);
//
//	pos = writeString(topic, buffer, pos);
//
//	rc += _client->send(buffer, pos);
//
//	for (i = 0; i < plength; i++) {
//		//rc += _client->send((char)pgm_read_byte_near(payload + i));
//		//rc += _client->send((uint8_t*) pgm_read_byte_near(payload + i), 1);
//	}
//
//	keepAliveTimer->reset(0); //lastOutActivity = millis();
//
//	return rc == tlen + 4 + plength;
//}


/**
 * @brief 	Send a MQTT message over socket.
 * @param 	[in] MQTT header.
 * 			[in] Message buffer.
 * 			[in] total length.
 * @return 	success (true), or no success (false).
 */
bool PubSubClient::write(uint8_t header, uint8_t* buf, uint16_t length) {
	uint8_t lenBuf[4];
	uint8_t llen = 0;
	uint8_t digit;
	uint8_t pos = 0;
	int rc;
	uint16_t len = length;
	do {
		digit = len % 128;
		len = len / 128;
		if (len > 0) {
			digit |= 0x80;
		}
		lenBuf[pos++] = digit;
		llen++;
	} while (len > 0);

	buf[4 - llen] = header;
	for (int i = 0; i < llen; i++) {
		buf[5 - llen + i] = lenBuf[i];
	}

//#ifdef MQTT_MAX_TRANSFER_SIZE
//	uint8_t* writeBuf = buf + (4 - llen);
//	uint16_t bytesRemaining = length + 1 + llen;  //Match the length type
//	uint8_t bytesToWrite;
//	bool result = true;
//	while ((bytesRemaining > 0) && result) {
//		bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE) ? MQTT_MAX_TRANSFER_SIZE : bytesRemaining;
//		rc = _client->write(writeBuf, bytesToWrite);
//		result = (rc == bytesToWrite);
//		bytesRemaining -= rc;
//		writeBuf += rc;
//	}
//	return result;
//#else
	rc = _client->send(buf + (4 - llen), length + 1 + llen);
	if(rc < 0) _state = CONNECTION_LOST;
	keepAliveTimer->reset(0); //lastOutActivity = millis();
	return (rc == 1 + llen + length);
//#endif
}


/**
 * @brief 	Subscribe a MQTT topic.
 * @param 	[in] my topic
 * 			[in] qos of subscription
 * @return 	request transmitted with success (true), or no success (false).
 */
bool PubSubClient::subscribe(const char* topic, bool ack) {
	if (MQTT_MAX_PACKET_SIZE < 9 + strlen(topic)) return false; // Too long

	if (connected()) {
		// Leave room in the buffer for header and variable length field
		uint16_t length = 5;
		nextMsgId++;
		if (nextMsgId == 0) {
			nextMsgId = 1;
		}
		buffer[length++] = (nextMsgId >> 8);
		buffer[length++] = (nextMsgId & 0xFF);
		length = writeString(topic, buffer, length);
		buffer[length++] = QOS1;

		if (write(SUBSCRIBE | QOS1, buffer, length - 5)) {
			SUBACK_outstanding = true;
			if (ack) timeoutTimer->start(0);
			return true;
		}
	}
	return false;
}


/**
 * @brief 	Check the state of subscription. If there was received a subscription
 *          ACK, we return a true here.
 * @return 	Is subscription validated with ACK (true/false)
 */
bool PubSubClient::isSubscribeDone() {
	return !SUBACK_outstanding;
}


/**
 * @brief 	Unsubscribe a MQTT topic.
 * @param 	[in] my topic
 * 			[in] qos of unsubscription
 * @return 	request transmitted with success (true), or no success (false).
 */
bool PubSubClient::unsubscribe(const char* topic, bool ack) {
	if (MQTT_MAX_PACKET_SIZE < 9 + strlen(topic)) return false; // Too long

	if (connected()) {
		uint16_t length = 5;
		nextMsgId++;
		if (nextMsgId == 0) {
			nextMsgId = 1;
		}
		buffer[length++] = (nextMsgId >> 8);
		buffer[length++] = (nextMsgId & 0xFF);
		length = writeString(topic, buffer, length);

		if (write(UNSUBSCRIBE | QOS1, buffer, length - 5)) {
			UNSUBACK_Outstanding = true;
			if (ack) timeoutTimer->start(0);
			return true;
		}
	}
	return false;
}


/**
 * @brief 	Check the state of unsubscription. If there was received a unsubscription
 *          ACK, we return a true here.
 * @return 	Is unsubscription validated with ACK (true/false)
 */
bool PubSubClient::isUnsubscribeDone() {
	return !UNSUBACK_Outstanding;
}


/**
 * @brief 	Disconnect form MQTT server and close the socket.
 * @return 	N/A.
 */
void PubSubClient::disconnect() {
	buffer[0] = DISCONNECT;
	buffer[1] = 0;
	_client->send(buffer, 2);
	_state = DISCONNECTED;
	_client->close();
	keepAliveTimer->stop(0); //lastInActivity = lastOutActivity = millis();
	timeoutTimer->stop(0);
}


/**
 * @brief calculation help to send a string.
 */
uint16_t PubSubClient::writeString(const char* string, uint8_t* buf, uint16_t pos) {
	const char* idp = string;
	uint16_t i = 0;
	pos += 2;
	while (*idp) {
		buf[pos++] = *idp++;
		i++;
	}
	buf[pos - i - 2] = (i >> 8);
	buf[pos - i - 1] = (i & 0xFF);
	return pos;
}


/**
 * @brief 	Check the connection to the MQTT server.
 * @return 	connected (true/false)
 */
bool PubSubClient::connected() {
	if (this->_state != CONNECTED) return false;
	bool rc = true;

	if (_client == nullptr) {
		rc = false;
	} else if (!_client->isValid()) {
		rc = false;

		this->_state = CONNECTION_LOST;

		if (_client->isValid()) _client->close();
		keepAliveTimer->stop(0);
		timeoutTimer->stop(0);
	}
	return rc;
}


/**
 * @brief 	Set server ip and Port of my MQTT server.
 * @param   [in] ip of the distant MQTT server.
 * 			[in] the port of the distant MQTT port.
 * @return 	My instance.
 */
PubSubClient& PubSubClient::setServer(std::string ip, uint16_t port) {
	_config.ip = ip;
	_config.port = port;
	return *this;
}


/**
 * @brief 	Set the callback function for incoming data.
 * @param   [in] callback function
 * @return 	My instance.
 */
PubSubClient& PubSubClient::setCallback(MQTT_CALLBACK_SIGNATURE) {
	this->callback = callback;
	return *this;
}


/**
 * @brief 	Set the socket, which we want to use for our MQTT communication.
 * @param   [in] the new socket instance
 * @return 	My instance.
 */
PubSubClient& PubSubClient::setClient(Socket& client) {
	this->_client = &client;
	return *this;
}


/**
 * @brief 	Get the current MYTT state form the instance.
 * @param   N/A.
 * @return 	current MQTT state.
 */
int PubSubClient::state() {
	return this->_state;
}


/**
 * @brief 	Parsing the received data in to the internal message struct.
 */
void PubSubClient::parseData(mqtt_message* msg, uint16_t len) {
	/********* Parse Fixed header *********/
	msg->type = buffer[0] & 0xF0;

	/* read DUP-Flag */
	if (msg->type == PUBLISH) {
		msg->dup = (bool) (buffer[0] & 0x18) >> 3;
	}

	/* read QoS-Level */
	if(msg->type == PUBLISH) {
		msg->qos = (buffer[0] & 0x06);
	}

	/* read RETAIN-Frag */
	if(msg->type == PUBLISH) {
		msg->retained = (bool) (buffer[0] & 0x01);
	}

	uint8_t remainingLength = buffer[1];

	/********* Parse Variable header *********/
	int pos = 2;

	/* read topic name */
	if (msg->type == PUBLISH) {
		uint16_t topicLen = (buffer[2] << 8) + buffer[3];

		msg->topic = "";
		for (int i = 4; i < (topicLen + 4); i++) {
			msg->topic += (char) buffer[i];
			pos++;
		}
	}

	/* read Message ID */
	if (msg->type == PUBLISH || msg->type == PUBACK || msg->type == PUBREC || msg->type == PUBCOMP || msg->type ==  SUBACK || msg->type ==  UNSUBACK) {
		msg->msgId = (buffer[pos] << 8) + (buffer[pos + 1]);
		pos += 2;
	}

	/********* read Payload *********/
	if (msg->type == PUBLISH) {
		msg->payload = "";
		for (int i = pos; i < remainingLength + 2; i++) {
			msg->payload += (char) buffer[i];
		}
	}
}


/**
 * @brief 	Dump the message struct.
 */
void PubSubClient::dumpData(mqtt_message* msg) {
	ESP_LOGD(TAG, "mqtt_message_type: %s", messageType_toString(msg->type).c_str());
	ESP_LOGD(TAG, "mqtt_qos:          %d", msg->qos);
	ESP_LOGD(TAG, "retained:          %d", msg->retained);
	ESP_LOGD(TAG, "dup:               %d", msg->dup);
	ESP_LOGD(TAG, "topic:             %s", msg->topic.c_str());
	ESP_LOGD(TAG, "payload:           %s", msg->payload.c_str());
	ESP_LOGD(TAG, "msgId:             %d", msg->msgId);
}


/**
 * @brief 	Convert the MQTT message type to string.
 * @param   [in] message type byte.
 * @return  message type as std::string.
 */
std::string PubSubClient::messageType_toString(uint8_t type) {
	std::string str = "Not in list!";
	switch (type) {
		case CONNECT	: str = "CONNECT"; break;
		case CONNACK	: str = "CONNACK"; break;
		case PUBLISH	: str = "PUBLISH"; break;
		case PUBACK	 : str = "PUBACK"; break;
		case PUBREC	 : str = "PUBREC"; break;
		case PUBREL	 : str = "PUBREL"; break;
		case PUBCOMP	: str = "PUBCOMP"; break;
		case SUBSCRIBE  : str = "SUBSCRIBE"; break;
		case SUBACK	 : str = "SUBACK"; break;
		case UNSUBSCRIBE: str = "UNSUBSCRIBE"; break;
		case UNSUBACK   : str = "UNSUBACK"; break;
		case PINGREQ	: str = "PINGREQ"; break;
		case PINGRESP   : str = "PINGRESP"; break;
		case DISCONNECT : str = "DISCONNECT"; break;
		case Reserved   : str = "Reserved"; break;
		default		 : break;
	}
	return str;
}
