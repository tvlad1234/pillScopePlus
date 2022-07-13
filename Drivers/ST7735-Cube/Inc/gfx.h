#ifndef _GFX_H
#define _GFX_H

#include "main.h"

void clearDisplay();


void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t bmap[]);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void drawFastHLine(int16_t x, int16_t y, int16_t l, uint16_t color);

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
			  uint16_t bg, uint8_t size_x, uint8_t size_y);
void setCursor(int16_t x, int16_t y);
void setTextColor(uint16_t c, uint16_t bg);
void setTextSize(uint16_t s);

void writeChar(char c);
void printString(char s[]);

#endif /* _GFX_H */
