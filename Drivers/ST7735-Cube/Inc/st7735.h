#ifndef __ST7735_H
#define __ST7735_H

#include "main.h"

#define INITR_GREENTAB 0x00
#define INITR_REDTAB 0x01
#define INITR_BLACKTAB 0x02
#define INITR_18GREENTAB INITR_GREENTAB
#define INITR_18REDTAB INITR_REDTAB
#define INITR_18BLACKTAB INITR_BLACKTAB
#define INITR_144GREENTAB 0x01
#define INITR_MINI160x80 0x04
#define INITR_HALLOWING 0x05

// Some ready-made 16-bit ('565') color settings:
#define ST7735_BLACK 0x0000
#define ST7735_WHITE 0xFFFF
#define ST7735_RED 0xF800
#define ST7735_GREEN 0x07E0
#define ST7735_BLUE 0x001F
#define ST7735_CYAN 0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW 0xFFE0
#define ST7735_ORANGE 0xFC00

void ST7735_initR(uint8_t options, SPI_HandleTypeDef *port);
void createFramebuf();
void destroyFramebuf();
void setRotation(uint8_t m);
void drawPixel(int16_t x, int16_t y, uint16_t color);
void flushDisplay();

#endif
