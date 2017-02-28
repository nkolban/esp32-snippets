/**
 * Driver for WS2812/NeoPixel data
 */

#ifndef MAIN_WS2812_H_
#define MAIN_WS2812_H_
#include <stdint.h>
#include <driver/rmt.h>
typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} pixel_t;

class WS2812 {
public:
	WS2812(int channel, int gpioNum, uint16_t pixelCount);
	void show();
	void setColorOrder(char *order);
	void setPixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);
	void setPixel(uint16_t index, pixel_t pixel);
	void setPixel(uint16_t index, uint32_t pixel);
	void clear();
	virtual ~WS2812();
private:
	char *colorOrder;
	uint16_t pixelCount;
	rmt_channel_t channel;
	rmt_item32_t *items;
	pixel_t *pixels;
};

#endif /* MAIN_WS2812_H_ */
