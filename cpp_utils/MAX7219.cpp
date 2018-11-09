/*
 *    MAX7219.cpp - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 *
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 *
 *    This permission notice shall be included in all copies or
 *    substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * Modified by Neil Kolban (2017) for ESP32 support.
 */

#include "MAX7219.h"
#include <math.h>

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP        0x00
#define OP_DIGIT0      0x01
#define OP_DIGIT1      0x02
#define OP_DIGIT2      0x03
#define OP_DIGIT3      0x04
#define OP_DIGIT4      0x05
#define OP_DIGIT5      0x06
#define OP_DIGIT6      0x07
#define OP_DIGIT7      0x08
#define OP_DECODEMODE  0x09
#define OP_INTENSITY   0x0a
#define OP_SCANLIMIT   0x0b
#define OP_SHUTDOWN    0x0c
#define OP_DISPLAYTEST 0x0f

/*
 * Segments to be switched on for characters and digits on
 * 7-Segment Displays
 */
const static uint8_t charTable[] = {
		0b01111110, 0b00110000, 0b01101101,
		0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000, 0b01111111,
		0b01111011, 0b01110111, 0b00011111, 0b00001101, 0b00111101, 0b01001111,
		0b01000111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000,
		0b00000001, 0b10000000, 0b00000000, 0b01111110, 0b00110000, 0b01101101,
		0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000, 0b01111111,
		0b01111011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b01110111, 0b00011111, 0b00001101, 0b00111101,
		0b01001111, 0b01000111, 0b00000000, 0b00110111, 0b00000000, 0b00000000,
		0b00000000, 0b00001110, 0b00000000, 0b00000000, 0b00000000, 0b01100111,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00001000, 0b00000000, 0b01110111, 0b00011111,
		0b00001101, 0b00111101, 0b01001111, 0b01000111, 0b00000000, 0b00110111,
		0b00000000, 0b00000000, 0b00000000, 0b00001110, 0b00000000, 0b00010101,
		0b00011101, 0b01100111, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 };


MAX7219::MAX7219(SPI* spi, int numDevices) {
	assert(spi != nullptr);
	this->spi = spi;
	if (numDevices <= 0 || numDevices > 8) {
		numDevices = 8;
	}
	maxDevices = numDevices;

	for (uint8_t i = 0; i < 64; i++) {
		status[i] = 0x00;
	}
	for (int i = 0; i < maxDevices; i++) {
		spiTransfer(i, OP_DISPLAYTEST, 0);
		//scanlimit is set to max on startup
		setScanLimit(7, i);
		//decode is done in source
		spiTransfer(i, OP_DECODEMODE, 0);
		clearDisplay(i);
		//we go into shutdown-mode on startup
		shutdown(true, i);
	}
}


int MAX7219::getDeviceCount() {
	return maxDevices;
}


void MAX7219::shutdown(bool b, int addr) {
	if (addr < 0 || addr >= maxDevices) return;

	if (b) {
		spiTransfer(addr, OP_SHUTDOWN, 0);
	} else {
		spiTransfer(addr, OP_SHUTDOWN, 1);
	}
}


void MAX7219::setScanLimit(int limit, int addr) {
	if (addr < 0 || addr >= maxDevices) return;
	if (limit >= 0 && limit < 8) {
		spiTransfer(addr, OP_SCANLIMIT, limit);
	}
}


void MAX7219::setIntensity(int intensity, int addr) {
	if (addr < 0 || addr >= maxDevices) return;
	if (intensity >= 0 && intensity < 16) {
		spiTransfer(addr, OP_INTENSITY, intensity);
	}
}


void MAX7219::clearDisplay(int addr) {
	if (addr < 0 || addr >= maxDevices) return;

	int offset;
	offset = addr * 8;
	for (uint8_t i = 0; i < 8; i++) {
		status[offset + i] = 0;
		spiTransfer(addr, i + 1, status[offset + i]);
	}
}


void MAX7219::setLed(int row, int column, bool state, int addr) {
	if (addr < 0 || addr >= maxDevices) return;

	int offset;
	uint8_t val = 0;

	if (row < 0 || row > 7 || column < 0 || column > 7) {
		return;
	}
	offset = addr * 8;
	val = 0b10000000 >> column;
	if (state) {
		status[offset + row] = status[offset + row] | val;
	} else {
		val = ~val;
		status[offset + row] = status[offset + row] & val;
	}
	spiTransfer(addr, row + 1, status[offset + row]);
}


void MAX7219::setRow(int row, uint8_t value, int addr) {
	if (addr < 0 || addr >= maxDevices) return;
	if (row < 0 || row > 7) return;

	int offset = addr * 8;
	status[offset + row] = value;
	spiTransfer(addr, row + 1, status[offset + row]);
}


void MAX7219::setColumn(int col, uint8_t value, int addr) {
	if (addr < 0 || addr >= maxDevices) return;
	if (col < 0 || col > 7) return;

	uint8_t val;
	for (int row = 0; row < 8; row++) {
		val = value >> (7 - row);
		val = val & 0x01;
		setLed(row, col, val, addr);
	}
}


void MAX7219::setDigit(int digit, uint8_t value, bool dp, int addr) {
	if (addr < 0 || addr >= maxDevices) return;
	if (digit < 0 || digit > 7 || value > 15) return;

	int offset = addr * 8;
	uint8_t v = charTable[value];
	if (dp) {
		v |= 0b10000000;
	}
	status[offset + digit] = v;
	spiTransfer(addr, digit + 1, v);
}


void MAX7219::setChar(int digit, char value, bool dp, int addr) {
	if (addr < 0 || addr >= maxDevices) return;
	if (digit < 0 || digit > 7) return;

	int offset = addr * 8;
	uint8_t index = (uint8_t) value;
	if (index > 127) index = 32; // not defined beyond index 127, so we use the space char
	uint8_t v = charTable[index];
	if (dp) {
		v |= 0b10000000;
	}
	status[offset + digit] = v;
	spiTransfer(addr, digit + 1, v);
}


void MAX7219::spiTransfer(int addr, volatile uint8_t opcode, volatile uint8_t data) {
	//Create an array with the data to shift out
	int offset = addr * 2;
	int maxbytes = maxDevices * 2;

	/* The array for shifting the data to the devices */
	uint8_t spidata[16];

	for (int i = 0; i < maxbytes; i++) {
		spidata[i] = (uint8_t) 0;
	}
	//put our device data into the array
	spidata[offset] = opcode;
	spidata[offset + 1] = data;
	spi->transfer(spidata, maxbytes);
}

void MAX7219::setNumber(uint32_t number, int addr) {
	// number = number % (uint32_t) pow(10, maxDevices);
	for (uint8_t i = 0; i < 8; i++) {
		if (number == 0 && i > 0) {
			setChar(i, ' ', addr);
		} else {
			setDigit(i, number % 10, addr);
			number = number / 10;
		}
	}
}
