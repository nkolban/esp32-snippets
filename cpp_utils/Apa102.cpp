/*
 * Apa102.cpp
 *
 *  Created on: Oct 22, 2017
 *      Author: kolban
 */

#include "Apa102.h"

Apa102::Apa102() {
}

Apa102::~Apa102() {

}


void Apa102::init() {
	mySPI.init();
}

/**
 * @brief Show the pixels on an APA102 device.
 * The pixels that have been set are pushed to the APA102 devices.
 */
void Apa102::show() {
	// We follow the data sheet for the APA102.  To signify a new stream of data
	// we send 32bits of 0 value.  Following that are 4 bytes of color data.  The
	// data is 0b111 nnnnn where `nnnnn` is the brightness of the pixels.
	// The following data is 8 bits for blue, 8 bits for green and 8 bits for red.

	// Send APA102 start.
	mySPI.transferByte(0x0);
	mySPI.transferByte(0x0);
	mySPI.transferByte(0x0);
	mySPI.transferByte(0x0);

	double brigthnessScale = getBrightness() / 100.0;
	// Loop over all the pixels in the pixels array to set the colors.
	for (uint16_t i = 0; i < m_pixelCount; i++) {
		mySPI.transferByte(0xff); // Maximum brightness
		mySPI.transferByte(m_pixels[i].blue * brigthnessScale);
		mySPI.transferByte(m_pixels[i].green * brigthnessScale);
		mySPI.transferByte(m_pixels[i].red * brigthnessScale);
	} // End loop over all the pixels.
} // show
