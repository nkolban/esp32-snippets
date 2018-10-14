/*
 * U8G2.h
 *
 *  Created on: May 6, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_U8G2_H_
#define COMPONENTS_CPP_UTILS_U8G2_H_
#include "sdkconfig.h"
#ifdef CONFIG_U8G2_PRESENT
#include <u8g2.h>
#include <driver/gpio.h>
#include <string>

/**
 * @brief Wrapper for the U8G2 display driver library.
 */
class U8G2 {
public:
	U8G2(gpio_num_t sda, gpio_num_t scl, int address);
	virtual ~U8G2();
	void clearBuffer() {
		u8g2_ClearBuffer(&m_u8g2);
	}

	void drawBitmap(uint32_t x, uint32_t y, uint32_t cnt, uint32_t h, const uint8_t* bitmap) {
		u8g2_DrawBitmap(&m_u8g2, x, y, cnt, h, bitmap);
	}

	void drawBox(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
		u8g2_DrawBox(&m_u8g2, x, y, w, h);
	}

	void drawCircle(uint32_t x0, uint32_t y0, uint32_t rad, uint8_t opt) {
		u8g2_DrawCircle(&m_u8g2, x0, y0,  rad, opt);
	}

	void drawDisc(uint32_t x0, uint32_t y0, uint32_t rad, uint8_t opt) {
		u8g2_DrawDisc(&m_u8g2, x0, y0, rad, opt);
	}

	void drawEllipse(uint32_t x0, uint32_t y0, uint32_t rx, uint32_t ry, uint8_t opt) {
		u8g2_DrawEllipse(&m_u8g2, x0, y0, rx, ry, opt);
	}

	void drawFilledEllipse(uint32_t x0, uint32_t y0, uint32_t rx, uint32_t ry, uint8_t opt) {
		u8g2_DrawFilledEllipse(&m_u8g2, x0, y0, rx, ry, opt);
	}

	void drawFrame(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
		u8g2_DrawFrame(&m_u8g2, x, y, w, h);
	}

	void drawGlyph(uint32_t x, uint32_t y, uint16_t encoding) {
		u8g2_DrawGlyph(&m_u8g2, x, y, encoding);
	}

	void drawHLine(uint32_t x, uint32_t y, uint32_t w) {
		u8g2_DrawHLine(&m_u8g2, x, y, w);
	}
	void drawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) {
		u8g2_DrawLine(&m_u8g2, x0, y0, x1, y1);
	}

	void drawPixel(uint32_t x, uint32_t y) {
		u8g2_DrawPixel(&m_u8g2, x, y);
	}

	void drawRBox(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r)	{
		u8g2_DrawRBox(&m_u8g2, x, y, w, h, r);
	}

	void drawRFrame(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r) {
		u8g2_DrawRFrame(&m_u8g2, x, y, w, h, r);
	}

	uint32_t drawStr(uint32_t x, uint32_t y, std::string s) {
		return u8g2_DrawStr(&m_u8g2, x, y, s.c_str());
	}

	void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
		u8g2_DrawTriangle(&m_u8g2, x0, y0, x1, y1, x2, y2);
	}

	u8g2_uint_t drawUTF8(uint32_t x, uint32_t y, std::string s) {
		return u8g2_DrawUTF8(&m_u8g2, x, y, s.c_str());
	}

	void drawVLine(uint32_t x, uint32_t y, uint32_t h) {
		u8g2_DrawVLine(&m_u8g2, x, y, h);
	}

	int8_t getAscent() {
		return u8g2_GetAscent(&m_u8g2);
	}

	int8_t getDescent() {
		return u8g2_GetDescent(&m_u8g2);
	}

	uint32_t getStrWidth(std::string s) {
		return u8g2_GetStrWidth(&m_u8g2, s.c_str());
	}

	void initDisplay() {
		u8g2_InitDisplay(&m_u8g2);
	}

	void sendBuffer() {
		u8g2_SendBuffer(&m_u8g2);
	}

	void setFont(const uint8_t* font) {
		u8g2_SetFont(&m_u8g2, font);
	}

	void setPowerSave(uint8_t is_enable) {
		u8g2_SetPowerSave(&m_u8g2, is_enable); // wake up display
	}

private:
	u8g2_t m_u8g2;

};

#endif // CONFIG_U8G2_PRESENT
#endif /* COMPONENTS_CPP_UTILS_U8G2_H_ */
