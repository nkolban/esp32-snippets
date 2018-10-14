/*
 *    LedControl.h - A library for controling Leds with a MAX7219/MAX7221
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

#ifndef MAX7219_h
#define MAX7219_h

#include <SPI.h>

/**
 * @brief %MAX7219 and MAX7221 controller.
 *
 * This is a port of the Arduino [MAX7219 class](http://playground.arduino.cc/Main/MAX7219) to the ESP32.
 * We can attach multiple IC instances daisy chained together.  Each device has its own *address* with the
 * first device being 0 (the default).
 *
 * All the devices are in shutdown mode by default when we start.  Any devices that are to be used
 * must be taken out of shutdown mode with a call to:
 *
 * @code{.cpp}
 * shutdown(false);
 * @endcode
 *
 * For a given configuration, we are likely going to work with the %MAX7219 as either an 8 digit
 * 7 segment LED driver or an 8x8 led matrix.  Working with the two styles have similarities and
 * distinctions.  For the 7 segment LED, we will typically use setDigit(), setChar() and
 * setNumber() to set digits while in the 8x8 led matrix, we will use setLed(), setRow()
 * and setColumn().
 */
class MAX7219 {
public:
	/**
	 * @brief Create a new %MAX7219 controller
	 *
	 * @param [in] spi %SPI controller.
	 * @param [in] numDevices	maximum number of devices that can be controlled that are
	 * daisy chained together.
	 */
	MAX7219(SPI* spi, int numDevices = 1);


	/**
	 * @brief Switch all Leds on the display off.
	 *
	 * @param [in] addr	Address of the display to control.
	 */
	void clearDisplay(int addr);


	/**
	 * @brief Gets the number of devices attached to this %MAX7219.
	 * @return The number of devices on this %MAX7219.
	 */
	int getDeviceCount();


	/**
	 * @brief Display a character on a 7-Segment display.
	 *
	 * There are only a few characters that make sense here :
	 *	'0','1','2','3','4','5','6','7','8','9','0',
	 *  'A','b','c','d','E','F','H','L','P',
	 *  '.','-','_',' '
	 *
	 * @param [in] digit	The position of the character on the display (0..7).
	 * @param [in] value	The character to be displayed.
	 * @param [in] dp	Sets the decimal point.
	 * @param [in] addr	Address of the display.
	 */
	void setChar(int digit, char value, bool dp=false, int addr = 0);


	/**
	 * @brief Set all 8 Led's in a column to a new state.
	 *
	 * @param [in] col	column which is to be set (0..7).
	 * @param [in] value	each bit set to 1 will light up the corresponding Led.
	 * @param [in] addr	address of the display.
	 */
	void setColumn(int col, uint8_t value, int addr = 0);


	/**
	 * @brief Display a hexadecimal digit on a 7-Segment Display
	 *
	 * @param [in] digit	The position of the digit on the display (0..7).
	 * @param [in] value	The value to be displayed. (0x00..0x0F).
	 * @param [in] dp	Sets the decimal point.
	 * @param [in] addr	Address of the display.
	 */
	void setDigit(int digit, uint8_t value, bool dp=false, int addr = 0);


	/**
	 * @brief Set the brightness of the display.
	 *
	 * @param [in] intensity	the brightness of the display. (0..15).
	 * @param [in] addr		The address of the display to control.
	 */
	void setIntensity(int intensity, int addr = 0);


	/**
	 * @brief Set the status of a single Led.
	 *
	 * @param [in] row	The row of the Led (0..7).
	 * @param [in] col	The column of the Led (0..7).
	 * @param [in] state	If true the led is switched on, if false it is switched off.
	 * @param [in] addr	Address of the display.
	 */
	void setLed(int row, int col, bool state, int addr = 0);


	/**
	 * @brief Display a number on the 7-Segment display.
	 *
	 * A non negative whole number that is 8 digits or less is displayed.
	 *
	 * @param [in] number The number to display.
	 * @param [in] addr	Address of the display.
	 */
	void setNumber(uint32_t number, int addr = 0);


	/**
	 * @brief Set all 8 Led's in a row to a new state
	 *
	 * @param [in] row	Row which is to be set (0..7).
	 * @param [in] value	Each bit set to 1 will light up the corresponding Led.
	 * @param [in] addr	Address of the display.
	 */
	void setRow(int row, uint8_t value, int addr = 0);


	/**
	 * @brief Set the number of digits (or rows) to be displayed.
	 * See datasheet for side effects of the scanlimit on the brightness
	 * of the display.
	 *
	 *
	 * @param [in] limit	Number of digits to be displayed (1..8).
	 * @param [in] addr	Address of the display to control.
	 */
	void setScanLimit(int limit, int addr = 0);


	/**
	 * @brief Set the shutdown (power saving) mode for the device.
	 *
	 * @param [in] status	If true the device goes into power-down mode. Set to false
	 *		for normal operation.
	 * @param [in] addr	The address of the display to control.
	 */
	void shutdown(bool status, int addr = 0);

private:
	/* Send out a single command to the device */
	void spiTransfer(int addr, uint8_t opcode, uint8_t data);

	/* We keep track of the led-status for all 8 devices in this array */
	uint8_t status[64];

	/* The maximum number of devices we use */
	int maxDevices;
	SPI* spi;

};

#endif	// MAX7219.h

