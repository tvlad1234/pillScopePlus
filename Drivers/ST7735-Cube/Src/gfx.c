#include "gfx.h"
#include "st7735.h"
#include "font.h"
#include "string.h"

extern uint16_t _width;
extern uint16_t _height;

int16_t cursor_x = 0;				 ///< x location to start print()ing text
int16_t cursor_y = 0;				 ///< y location to start print()ing text
uint16_t textcolor = ST7735_WHITE;	 ///< 16-bit background color for print()
uint16_t textbgcolor = ST7735_BLACK; ///< 16-bit text color for print()
uint16_t textsize_x = 1;
uint16_t textsize_y = 1;

void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t bmap[])
{
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			drawPixel(x0 + i, y0+j, bmap[i + j * _width]);
		}
}

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{

	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	drawPixel(x0, y0 + r, color);
	drawPixel(x0, y0 - r, color);
	drawPixel(x0 + r, y0, color);
	drawPixel(x0 - r, y0, color);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		drawPixel(x0 + x, y0 + y, color);
		drawPixel(x0 - x, y0 + y, color);
		drawPixel(x0 + x, y0 - y, color);
		drawPixel(x0 - x, y0 - y, color);
		drawPixel(x0 + y, y0 + x, color);
		drawPixel(x0 - y, y0 + x, color);
		drawPixel(x0 + y, y0 - x, color);
		drawPixel(x0 - y, y0 - x, color);
	}
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	drawFastHLine(x, y, w, color);
	drawFastHLine(x, y + h - 1, w, color);
	drawFastVLine(x, y, h, color);
	drawFastVLine(x + w - 1, y, h, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	for (int16_t i = x; i < x + w; i++)
		drawFastVLine(i, y, h, color);
}

void clearDisplay()
{
	fillRect(0, 0, _width, _height, ST7735_BLACK);
}

void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
			  uint16_t bg, uint8_t size_x, uint8_t size_y)
{
	if ((x >= _width) ||			  // Clip right
		(y >= _height) ||			  // Clip bottom
		((x + 6 * size_x - 1) < 0) || // Clip left
		((y + 8 * size_y - 1) < 0))	  // Clip top
		return;

	if (c >= 176)
		c++; // Handle 'classic' charset behavior

	for (int8_t i = 0; i < 5; i++)
	{ // Char bitmap = 5 columns
		uint8_t line = font[c * 5 + i];
		for (int8_t j = 0; j < 8; j++, line >>= 1)
		{
			if (line & 1)
			{
				if (size_x == 1 && size_y == 1)
					drawPixel(x + i, y + j, color);
				else
					fillRect(x + i * size_x, y + j * size_y, size_x,
							 size_y, color);
			}
			else if (bg != color)
			{
				if (size_x == 1 && size_y == 1)
					drawPixel(x + i, y + j, bg);
				else
					fillRect(x + i * size_x, y + j * size_y, size_x,
							 size_y, bg);
			}
		}
	}
	if (bg != color)
	{ // If opaque, draw vertical line for last column
		if (size_x == 1 && size_y == 1)
			drawFastVLine(x + 5, y, 7, bg);
		else
			fillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
	}
}

void setCursor(int16_t x, int16_t y)
{
	cursor_x = x;
	cursor_y = y;
}

void setTextColor(uint16_t c, uint16_t bg)
{
	textcolor = c;
	textbgcolor = bg;
}

void setTextSize(uint16_t s)
{
	textsize_x = s;
	textsize_y = s;
}

void writeChar(char c)
{
	if (c == '\n')
	{								// Newline?
		cursor_x = 0;				// Reset x to zero,
		cursor_y += textsize_y * 8; // advance y one line
	}
	else if (c != '\r')
	{ // Ignore carriage returns
		if ((cursor_x + textsize_x * 6) > _width)
		{								// Off right?
			cursor_x = 0;				// Reset x to zero,
			cursor_y += textsize_y * 8; // advance y one line
		}
		drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor,
				 textsize_x, textsize_y);
		cursor_x += textsize_x * 6; // Advance x one char
	}
}

void printString(char s[])
{
	uint8_t n = strlen(s);
	for (int i = 0; i < n; i++)
		writeChar(s[i]);
}

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
	{                       \
		int16_t t = a;      \
		a = b;              \
		b = t;              \
	}
#endif

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	for (int i = y; i <= y + h; i++)
		drawPixel(x, i, color);
}

void drawFastHLine(int16_t x, int16_t y, int16_t l, uint16_t color)
{
	for (int i = x; i <= x + l; i++)
		drawPixel(i, y, color);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
			  uint16_t color)
{

	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep)
	{
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1)
	{
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1)
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	for (; x0 <= x1; x0++)
	{
		if (steep)
		{
			drawPixel(y0, x0, color);
		}
		else
		{
			drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}