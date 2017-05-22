/*
 * RF24_ESP_IDF.h
 *
 *  Created on: Feb 18, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_RF24_UTILITY_ESP_IDF_RF24_ESP_IDF_H_
#define COMPONENTS_RF24_UTILITY_ESP_IDF_RF24_ESP_IDF_H_

#define OUTPUT 0
void     delay(int value);
void     delayMicroseconds(int value);
void     digitalWrite(int pin, int value);
uint32_t millis();
void     pinMode(int pin, int mode);

#endif /* COMPONENTS_RF24_UTILITY_ESP_IDF_RF24_ESP_IDF_H_ */
