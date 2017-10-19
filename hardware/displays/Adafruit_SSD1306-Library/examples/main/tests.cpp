/*
 * tests.cpp
 *
 *  Created on: Oct 9, 2017
 *      Author: chegewara
 */
#define enablePartialUpdate

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
//#include "math.h"
#include "sdkconfig.h"

#include "Adafruit_SSD1306.h"
#define SCLK_PIN GPIO_NUM_18
#define DIN_PIN GPIO_NUM_23
#define DC_PIN GPIO_NUM_16
#define CS_PIN GPIO_NUM_5
#define RST_PIN GPIO_NUM_14

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define delay(x) vTaskDelay(x/portTICK_PERIOD_MS)

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

static const unsigned char logo16_glcd_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

Adafruit_SSD1306 display = Adafruit_SSD1306(DIN_PIN, SCLK_PIN, DC_PIN, RST_PIN, CS_PIN);

void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    //if ((i > 0) && (i % 14 == 0))
      //display.println();
  }
  display.display();
}

void testdrawcircle(void) {
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    display.display();
  }
}

void testfilltriangle(void) {
  uint8_t color = BLACK;
  for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
    display.fillTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawroundrect(void) {
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
  }
}

void testfillroundrect(void) {
  uint8_t color = BLACK;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawrect(void) {
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
  }
}

void testdrawline() {
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
  }
  delay(25);

  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int8_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
  }
  delay(25);

  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
  }
  delay(25);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
    display.display();
  }
  delay(25);
}

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.println((char*)"scroll");
  display.display();

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];

  // initialize
  for (uint8_t f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS] = rand()%display.width();
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = rand()%(5) + 1;
  }

  while (1) {
    // draw each icon
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, WHITE);
    }
    display.display();
    delay(20);

    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
	icons[f][XPOS] = rand()%(display.width());
	icons[f][YPOS] = 0;
	icons[f][DELTAY] = rand()%(5) + 1;
      }
    }
   }
}

void test_task(void*)   {
	while(1){
		display.begin();
		//display.setContrast(50);
		display.display();

		delay(2000);
		display.clearDisplay();   // clears the screen and buffer

		// draw a single pixel
		display.drawPixel(10, 10, WHITE);
		display.display();
		delay(2000);
		display.clearDisplay();

		// draw many lines
		testdrawline();
		display.display();
		delay(2000);
		display.clearDisplay();

		// draw rectangles
		testdrawrect();
		display.display();
		delay(2000);
		display.clearDisplay();

		// draw multiple rectangles
		testfillrect();
		display.display();
		delay(2000);
		display.clearDisplay();

		// draw mulitple circles
		testdrawcircle();
		display.display();
		delay(2000);
		display.clearDisplay();

		// draw a circle, 10 pixel radius
		display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
		display.display();
		delay(2000);
		display.clearDisplay();

		testdrawroundrect();
		delay(2000);
		display.clearDisplay();

		testfillroundrect();
		delay(2000);
		display.clearDisplay();

		testdrawtriangle();
		delay(2000);
		display.clearDisplay();

		testfilltriangle();
		delay(2000);
		display.clearDisplay();

		// draw the first ~12 characters in the font
		testdrawchar();
		display.display();
		delay(2000);
		display.clearDisplay();

		// draw scrolling text
		testscrolltext();
		delay(2000);
		display.clearDisplay();

		// text display tests
		display.setTextSize(2);
		display.setTextColor(WHITE);
		display.setCursor(0,0);
		display.println((char*)"Hello, world!");
		display.setTextColor(WHITE, BLACK); // 'inverted' text
		display.println((char*)"3.141592");
		display.setTextSize(1);
		display.setTextColor(WHITE);
		display.print((char*)"0x");
		display.println((char*)"DEADBEEF");
		display.display();
		delay(2000);

		// rotation example
		display.clearDisplay();
		display.setRotation(1);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
		display.setTextSize(1);
		display.setTextColor(WHITE);
		display.setCursor(0,0);
		display.println((char*)"Rotation");
		display.setTextSize(1);
		display.println((char*)"Example!");
		display.display();
		delay(2000);

		// revert back to no rotation
		display.setRotation(0);

		// miniature bitmap display
		display.clearDisplay();
		display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, WHITE);
		display.display();

		// invert the display
		display.invertDisplay(true);
		delay(1000);
		display.invertDisplay(false);
		delay(1000);

		// draw a bitmap icon and 'animate' movement
		testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_WIDTH, LOGO16_GLCD_HEIGHT);
	}
}


