/*
 * TMRh20 2015
 * SPI layer for RF24 <-> BCM2835
 */
/**
 * @file spi.h
 * \cond HIDDEN_SYMBOLS
 * Class declaration for SPI helper files
 */
#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <stdio.h>

#define SPI_HAS_TRANSACTION
#define MSBFIRST       BCM2835_SPI_BIT_ORDER_MSBFIRST
#define SPI_MODE0      BCM2835_SPI_MODE0
#define RF24_SPI_SPEED BCM2835_SPI_SPEED_8MHZ

#include <stdint.h>

class NRF24_SPI {
public:

  static uint8_t transfer(uint8_t _data);
//  static void transfernb(char* tbuf, char* rbuf, uint32_t len);
//  static void transfern(char* buf, uint32_t len);

  static void begin();
//  static void end();

//  static void setBitOrder(uint8_t bit_order);
//  static void setDataMode(uint8_t data_mode);
//  static void setClockDivider(uint16_t spi_speed);
//  static void chipSelect(int csn_pin);
  
  //static void beginTransaction(SPISettings settings);
//  static void endTransaction();

};

extern NRF24_SPI _SPI;

#endif
