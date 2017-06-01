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
