/*
 PubSubClient.h - A simple client for MQTT.
  Nick O'Leary
  http://knolleary.net

  edit by marcel.seerig
*/

#ifndef PubSubClient_h
#define PubSubClient_h

#include <string>
#include "Socket.h"
#include "FreeRTOSTimer.h"

#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

// MQTT_VERSION : Pick the version
//#define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 128
#endif

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client.state()

struct mqtt_InitTypeDef{
	std::string 	ip;
	uint16_t 		port;

	const char* 	user;
	const char* 	pass;

	const char* 	id;
	const char* 	willTopic;
	uint8_t 		willQos;
	bool 			willRetain;
	const char* 	willMessage;
};

typedef enum {
	CONNECTION_TIMEOUT      = -4,
	CONNECTION_LOST         = -3,
	CONNECT_FAILED          = -2,
	DISCONNECTED            = -1,
	CONNECTED               =  0,
	CONNECT_BAD_PROTOCOL    =  1,
	CONNECT_BAD_CLIENT_ID   =  2,
	CONNECT_UNAVAILABLE     =  3,
	CONNECT_BAD_CREDENTIALS =  4,
	CONNECT_UNAUTHORIZED    =  5,
} mqtt_state;

typedef enum {
	CONNECT     =  1 << 4, // Client request to connect to Server
	CONNACK     =  2 << 4, // Connect Acknowledgment
	PUBLISH     =  3 << 4, // Publish message
	PUBACK      =  4 << 4, // Publish Acknowledgment
	PUBREC      =  5 << 4, // Publish Received (assured delivery part 1)
	PUBREL      =  6 << 4, // Publish Release (assured delivery part 2)
	PUBCOMP     =  7 << 4, // Publish Complete (assured delivery part 3)
	SUBSCRIBE   =  8 << 4, // Client Subscribe request
	SUBACK      =  9 << 4, // Subscribe Acknowledgment
	UNSUBSCRIBE = 10 << 4, // Client Unsubscribe request
	UNSUBACK    = 11 << 4, // Unsubscribe Acknowledgment
	PINGREQ     = 12 << 4, // PING Request
	PINGRESP    = 13 << 4, // PING Response
	DISCONNECT  = 14 << 4, // Client is Disconnecting
	Reserved    = 15 << 4, // Reserved
} mqtt_message_type;

typedef enum {
	QOS0        = (0 << 1),
	QOS1        = (1 << 1),
	QOS2        = (2 << 1),
} mqtt_qos;

struct mqtt_message {
	uint8_t type;
	uint8_t qos;
	bool retained;
	bool dup;
	std::string topic;
	std::string payload;
	uint16_t msgId;
};

#define MQTT_CALLBACK_SIGNATURE void (*callback) (std::string, std::string)

class PubSubClientTask;

class PubSubClient {
public:
   PubSubClient();
   PubSubClient(Socket& client);
   PubSubClient(std::string ip, uint16_t port);
   PubSubClient(std::string ip, uint16_t port, Socket& client);
   PubSubClient(std::string ip, uint16_t port, MQTT_CALLBACK_SIGNATURE,Socket& client);
   ~PubSubClient();

   PubSubClient& setServer(std::string ip, uint16_t port);
   PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE);
   PubSubClient& setClient(Socket& client);

   bool connect(const char* id);
   bool connect(const char* id, const char* user, const char* pass);
   bool connect(const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage);
   bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage);
   bool connect();
   void disconnect();
   bool publish(const char* topic, const char* payload);
   bool publish(const char* topic, const char* payload, bool retained);
   bool publish(const char* topic, const uint8_t* payload, unsigned int plength);
   bool publish(const char* topic, const uint8_t* payload, unsigned int plength, bool retained);
   //bool publish_P(const char* topic, const uint8_t * payload, unsigned int plength, bool retained);

   bool subscribe(const char* topic, bool ack = false);
   bool unsubscribe(const char* topic, bool ack = false);
   bool isSubscribeDone();
   bool isUnsubscribeDone();

   bool connected();
   int state();
   void keepAliveChecker();
   void timeoutChecker();

private:
   friend class 	PubSubClientTask;
   PubSubClientTask* m_task;
   Socket* 			_client;
   mqtt_InitTypeDef _config;
   mqtt_state 		_state;
   uint8_t 			buffer[MQTT_MAX_PACKET_SIZE];
   uint16_t 		nextMsgId;
   bool 			PING_outstanding;
   bool 			SUBACK_outstanding;
   bool 			UNSUBACK_Outstanding;
   FreeRTOSTimer* 	keepAliveTimer;
   FreeRTOSTimer* 	timeoutTimer;

   MQTT_CALLBACK_SIGNATURE;
   void setup();
   size_t readPacket();
   bool write(uint8_t header, uint8_t* buf, uint16_t length);
   uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos);
   void parseData(mqtt_message* msg, uint16_t len);
   void dumpData(mqtt_message* msg);
   std::string messageType_toString(uint8_t type);

};

#endif
