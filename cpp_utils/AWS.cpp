/*
 * AWS.cpp
 *
 *  Created on: May 31, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#ifdef CONFIG_AWS_IOT_SDK
#include <esp_log.h>
#include "AWS.h"
#include <string>

static char tag[] = "AWS";

AWS::AWS() {
}


AWS::~AWS() {
}

/**
 * Convert an AWS IoT error code to a string representation.
 * @param err The error code to be mapped.
 * @return A string representation of the error code.
 */
/* static */ std::string AWS::errorToString(IoT_Error_t err) {
	switch(err) {
	case NETWORK_PHYSICAL_LAYER_CONNECTED :
		return "NETWORK_PHYSICAL_LAYER_CONNECTED";
	case NETWORK_MANUALLY_DISCONNECTED :
		return "NETWORK_MANUALLY_DISCONNECTED";
	case NETWORK_ATTEMPTING_RECONNECT:
		return "NETWORK_ATTEMPTING_RECONNECT";
	case NETWORK_RECONNECTED:
		return "NETWORK_RECONNECTED";
	case MQTT_NOTHING_TO_READ :
		return "MQTT_NOTHING_TO_READ";
	case MQTT_CONNACK_CONNECTION_ACCEPTED:
		return "MQTT_CONNACK_CONNECTION_ACCEPTED";
	case SUCCESS :
		return "SUCCESS";
	case FAILURE:
		return "FAILURE";
	case NULL_VALUE_ERROR :
		return "NULL_VALUE_ERROR";
	case TCP_CONNECTION_ERROR :
		return "TCP_CONNECTION_ERROR";
	case SSL_CONNECTION_ERROR:
		return "SSL_CONNECTION_ERROR";
	case TCP_SETUP_ERROR :
		return "TCP_SETUP_ERROR";
	case NETWORK_SSL_CONNECT_TIMEOUT_ERROR :
		return "NETWORK_SSL_CONNECT_TIMEOUT_ERROR";
	case NETWORK_SSL_WRITE_ERROR :
		return "NETWORK_SSL_WRITE_ERROR";
	case NETWORK_SSL_INIT_ERROR :
		return "NETWORK_SSL_INIT_ERROR";
	case NETWORK_SSL_CERT_ERROR :
		return "NETWORK_SSL_CERT_ERROR";
	case NETWORK_SSL_WRITE_TIMEOUT_ERROR :
		return "NETWORK_SSL_WRITE_TIMEOUT_ERROR";
	case NETWORK_SSL_READ_TIMEOUT_ERROR :
		return "NETWORK_SSL_READ_TIMEOUT_ERROR";
	case NETWORK_SSL_READ_ERROR :
		return "NETWORK_SSL_READ_ERROR";
	case NETWORK_DISCONNECTED_ERROR :
		return "NETWORK_DISCONNECTED_ERROR";
	case NETWORK_RECONNECT_TIMED_OUT_ERROR:
		return "NETWORK_RECONNECT_TIMED_OUT_ERROR";
	case NETWORK_ALREADY_CONNECTED_ERROR :
		return "NETWORK_ALREADY_CONNECTED_ERROR";
	case NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED :
		return "NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED";
	case NETWORK_SSL_UNKNOWN_ERROR :
		return "NETWORK_SSL_UNKNOWN_ERROR";
	case NETWORK_PHYSICAL_LAYER_DISCONNECTED :
		return "NETWORK_PHYSICAL_LAYER_DISCONNECTED";
	case NETWORK_X509_ROOT_CRT_PARSE_ERROR :
		return "NETWORK_X509_ROOT_CRT_PARSE_ERROR";
	case NETWORK_X509_DEVICE_CRT_PARSE_ERROR :
		return "NETWORK_X509_DEVICE_CRT_PARSE_ERROR";
	case NETWORK_PK_PRIVATE_KEY_PARSE_ERROR :
		return "NETWORK_PK_PRIVATE_KEY_PARSE_ERROR";
	case NETWORK_ERR_NET_SOCKET_FAILED :
		return "NETWORK_ERR_NET_SOCKET_FAILED";
	case NETWORK_ERR_NET_UNKNOWN_HOST :
		return "NETWORK_ERR_NET_UNKNOWN_HOST";
	case NETWORK_ERR_NET_CONNECT_FAILED :
		return "NETWORK_ERR_NET_CONNECT_FAILED";
	case NETWORK_SSL_NOTHING_TO_READ :
		return "NETWORK_SSL_NOTHING_TO_READ";
	case MQTT_CONNECTION_ERROR :
		return "MQTT_CONNECTION_ERROR";
	case MQTT_CONNECT_TIMEOUT_ERROR :
		return "MQTT_CONNECT_TIMEOUT_ERROR";
	case MQTT_REQUEST_TIMEOUT_ERROR:
		return "MQTT_REQUEST_TIMEOUT_ERROR";
	case MQTT_UNEXPECTED_CLIENT_STATE_ERROR :
		return "MQTT_UNEXPECTED_CLIENT_STATE_ERROR";
	case MQTT_CLIENT_NOT_IDLE_ERROR :
		return "MQTT_CLIENT_NOT_IDLE_ERROR";
	case MQTT_RX_MESSAGE_PACKET_TYPE_INVALID_ERROR :
		return "MQTT_RX_MESSAGE_PACKET_TYPE_INVALID_ERROR";
	case MQTT_RX_BUFFER_TOO_SHORT_ERROR :
		return "MQTT_RX_BUFFER_TOO_SHORT_ERROR";
	case MQTT_TX_BUFFER_TOO_SHORT_ERROR :
		return "MQTT_TX_BUFFER_TOO_SHORT_ERROR";
	case MQTT_MAX_SUBSCRIPTIONS_REACHED_ERROR :
		return "MQTT_MAX_SUBSCRIPTIONS_REACHED_ERROR";
	case MQTT_DECODE_REMAINING_LENGTH_ERROR :
		return "MQTT_DECODE_REMAINING_LENGTH_ERROR";
	case MQTT_CONNACK_UNKNOWN_ERROR :
		return "MQTT_CONNACK_UNKNOWN_ERROR";
	case MQTT_CONNACK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR :
		return "MQTT_CONNACK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR";
	case MQTT_CONNACK_IDENTIFIER_REJECTED_ERROR:
		return "MQTT_CONNACK_IDENTIFIER_REJECTED_ERROR";
	case MQTT_CONNACK_SERVER_UNAVAILABLE_ERROR :
		return "MQTT_CONNACK_SERVER_UNAVAILABLE_ERROR";
	case MQTT_CONNACK_BAD_USERDATA_ERROR:
		return "MQTT_CONNACK_BAD_USERDATA_ERROR";
	case MQTT_CONNACK_NOT_AUTHORIZED_ERROR :
		return "MQTT_CONNACK_NOT_AUTHORIZED_ERROR";
	case JSON_PARSE_ERROR :
		return "JSON_PARSE_ERROR";
	case SHADOW_WAIT_FOR_PUBLISH :
		return "SHADOW_WAIT_FOR_PUBLISH";
	case SHADOW_JSON_BUFFER_TRUNCATED :
		return "SHADOW_JSON_BUFFER_TRUNCATED";
	case SHADOW_JSON_ERROR :
		return "SHADOW_JSON_ERROR";
	case MUTEX_INIT_ERROR :
		return "MUTEX_INIT_ERROR";
	case MUTEX_LOCK_ERROR:
		return "MUTEX_LOCK_ERROR";
	case MUTEX_UNLOCK_ERROR :
		return "MUTEX_UNLOCK_ERROR";
	case MUTEX_DESTROY_ERROR :
		return "MUTEX_DESTROY_ERROR";
	default:
		return "Unknown error!";
	}
} // AWS#errorToString

/**
 * @brief Connect to the AWS IoT service.
 * Connect to the AWT IoT service.
 * @param [in] The client id.
 * @return N/A.
 */
void AWS::connect(std::string clientId) {
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;
	connectParams.keepAliveIntervalInSec = 10;
	connectParams.isCleanSession   = true;
	connectParams.MQTTVersion      = MQTT_3_1_1;
	connectParams.isWillMsgPresent = false;
	connectParams.pClientID        = clientId.c_str();
	connectParams.clientIDLen      = clientId.length();

	IoT_Error_t err = ::aws_iot_mqtt_connect(&m_client, &connectParams);
	if (err != SUCCESS) {
		ESP_LOGD(tag, "aws_iot_mqtt_connect: error=%d", err);
	}
} // connect


/**
 * @brief Disconnect a previously formed connection.
 * Disconnect from the AWT IoT service.
 * @return N/A.
 */
void AWS::disconnect() {
	IoT_Error_t err = ::aws_iot_mqtt_disconnect(&m_client);
	if (err != SUCCESS) {
		ESP_LOGD(tag, "aws_iot_mqtt_disconnect: error=%d", err);
	}
} // disconnect


/**
 * @brief Initialize our connection.
 * @param [in] host The the host of the AWS IoT service.
 * @param [in] port The port number of the AWS IoT service.
 * @return N/A.
 */
void AWS::init(std::string host, uint16_t port) {
	IoT_Client_Init_Params initParams = iotClientInitParamsDefault;

	initParams.pHostURL                  = (char *)host.c_str();
	initParams.port                      = port;
	initParams.pRootCALocation           = nullptr;
	initParams.pDeviceCertLocation       = nullptr;
	initParams.pDevicePrivateKeyLocation = nullptr;
	initParams.mqttCommandTimeout_ms     = 20000;
	initParams.tlsHandshakeTimeout_ms    = 5000;
	initParams.isSSLHostnameVerify       = true;

	IoT_Error_t err = ::aws_iot_mqtt_init(&m_client, &initParams);
	if (err != SUCCESS) {
		ESP_LOGD(tag, "aws_iot_mqtt_init: error=%d", err);
	}
} // init


/**
 * @brief Publish a message.
 * Publish a message on the given topic.
 *
 * @param [in] topic The topic against which we wish to publish.
 * @param [in] payload The payload of the message we wish to publish.
 * @param [in] qos The quality of service for the publish.
 * @return N/A.
 */
void AWS::publish(std::string topic, std::string payload, QoS qos) {
	IoT_Publish_Message_Params message;
	message.payload = (void *)payload.data();
	message.payloadLen = payload.length();
	message.qos = qos;
	message.isRetained = 0;
	IoT_Error_t err = ::aws_iot_mqtt_publish(&m_client, topic.c_str(), topic.length(), &message);
	if (err != SUCCESS) {
		ESP_LOGD(tag, "aws_iot_mqtt_publish: error=%d", err);
	}
} // publish


/**
 * @brief Subscribe to a topic.
 * Future publications on this topic will be delivered to us.
 * @param [in] topic The topic against which we wish to subscribe.
 * @return N/A.
 */
void AWS::subscribe(std::string topic) {
	pApplicationHandler_t iot_subscribe_callback_handler = nullptr;
	IoT_Error_t err = ::aws_iot_mqtt_subscribe(&m_client, topic.c_str(), topic.length(), QOS0, iot_subscribe_callback_handler, NULL);
	if (err != SUCCESS) {
		ESP_LOGD(tag, "aws_iot_mqtt_subscribe: error=%d", err);
	}
} // subscribe


/**
 * @brief Un-subscribe from a previous subscription.
 * Further publications on this topic will no longer be delivered to us.
 * @param [in] topic The topic to un-subscribe from.
 * @return N/A.
 */
void AWS::unsubscribe(std::string topic) {
	IoT_Error_t err = ::aws_iot_mqtt_unsubscribe(&m_client, topic.c_str(), topic.length());
	if (err != SUCCESS) {
		ESP_LOGD(tag, "aws_iot_mqtt_unsubscribe: error=%d", err);
	}
} // unsubscribe
#endif // CONFIG_AWS_IOT_SDK
