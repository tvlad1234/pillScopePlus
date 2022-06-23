#include "main.h"
#include "st7735.h"
#include "font.h"

#define ST7735_TFTWIDTH_128 128  // for 1.44 and mini
#define ST7735_TFTWIDTH_80 80    // for mini
#define ST7735_TFTHEIGHT_128 128 // for 1.44" display
#define ST7735_TFTHEIGHT_160 160 // for 1.8" and mini display

// Some register settings
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD


SPI_HandleTypeDef *spiPort;
uint8_t _colstart = 0, _rowstart = 0;

uint8_t tabcolor;

int16_t _width;	 ///< Display width as modified by current rotation
int16_t _height; ///< Display height as modified by current rotation

int16_t _xstart = 0; ///< Internal framebuffer X offset
int16_t _ystart = 0; ///< Internal framebuffer Y offset

uint8_t rotation;

uint16_t *frameBuffer = NULL;

void createFramebuf()
{
	frameBuffer = malloc(_width * _height * sizeof(uint16_t));
}
void destroyFramebuf()
{
	free(frameBuffer);
	frameBuffer = NULL;
}

void flushDisplay()
{
	if (frameBuffer != NULL)
	{
		ST7735_Select();
		ST7735_setAddrWindow(0, 0, _width, _height); // Clipped area
		ST7735_RegData();
		HAL_SPI_Transmit(spiPort, frameBuffer, 2 * _width * _height, HAL_MAX_DELAY);
		ST7735_DeSelect();
	}
}

const uint8_t Bcmd[] = {			  // Init commands for 7735B screens
	18,								  // 18 commands in list:
	ST77XX_SWRESET, ST_CMD_DELAY,	  //  1: Software reset, no args, w/delay
	50,								  //     50 ms delay
	ST77XX_SLPOUT, ST_CMD_DELAY,	  //  2: Out of sleep mode, no args, w/delay
	255,							  //     255 = max (500 ms) delay
	ST77XX_COLMOD, 1 + ST_CMD_DELAY,  //  3: Set color mode, 1 arg + delay:
	0x05,							  //     16-bit color
	10,								  //     10 ms delay
	ST7735_FRMCTR1, 3 + ST_CMD_DELAY, //  4: Frame rate control, 3 args + delay:
	0x00,							  //     fastest refresh
	0x06,							  //     6 lines front porch
	0x03,							  //     3 lines back porch
	10,								  //     10 ms delay
	ST77XX_MADCTL, 1,				  //  5: Mem access ctl (directions), 1 arg:
	0x08,							  //     Row/col addr, bottom-top refresh
	ST7735_DISSET5, 2,				  //  6: Display settings #5, 2 args:
	0x15,							  //     1 clk cycle nonoverlap, 2 cycle gate
									  //     rise, 3 cycle osc equalize
	0x02,							  //     Fix on VTL
	ST7735_INVCTR, 1,				  //  7: Display inversion control, 1 arg:
	0x0,							  //     Line inversion
	ST7735_PWCTR1, 2 + ST_CMD_DELAY,  //  8: Power control, 2 args + delay:
	0x02,							  //     GVDD = 4.7V
	0x70,							  //     1.0uA
	10,								  //     10 ms delay
	ST7735_PWCTR2, 1,				  //  9: Power control, 1 arg, no delay:
	0x05,							  //     VGH = 14.7V, VGL = -7.35V
	ST7735_PWCTR3, 2,				  // 10: Power control, 2 args, no delay:
	0x01,							  //     Opamp current small
	0x02,							  //     Boost frequency
	ST7735_VMCTR1, 2 + ST_CMD_DELAY,  // 11: Power control, 2 args + delay:
	0x3C,							  //     VCOMH = 4V
	0x38,							  //     VCOML = -1.1V
	10,								  //     10 ms delay
	ST7735_PWCTR6, 2,				  // 12: Power control, 2 args, no delay:
	0x11, 0x15,
	ST7735_GMCTRP1, 16,		// 13: Gamma Adjustments (pos. polarity), 16 args + delay:
	0x09, 0x16, 0x09, 0x20, //     (Not entirely necessary, but provides
	0x21, 0x1B, 0x13, 0x19, //      accurate colors)
	0x17, 0x15, 0x1E, 0x2B, 0x04, 0x05, 0x02, 0x0E,
	ST7735_GMCTRN1, 16 + ST_CMD_DELAY,					// 14: Gamma Adjustments (neg. polarity), 16 args + delay:
	0x0B, 0x14, 0x08, 0x1E,								//     (Not entirely necessary, but provides
	0x22, 0x1D, 0x18, 0x1E,								//      accurate colors)
	0x1B, 0x1A, 0x24, 0x2B, 0x06, 0x06, 0x02, 0x0F, 10, //     10 ms delay
	ST77XX_CASET, 4,									// 15: Column addr set, 4 args, no delay:
	0x00, 0x02,											//     XSTART = 2
	0x00, 0x81,											//     XEND = 129
	ST77XX_RASET, 4,									// 16: Row addr set, 4 args, no delay:
	0x00, 0x02,											//     XSTART = 1
	0x00, 0x81,											//     XEND = 160
	ST77XX_NORON, ST_CMD_DELAY,							// 17: Normal display on, no args, w/delay
	10,													//     10 ms delay
	ST77XX_DISPON, ST_CMD_DELAY,						// 18: Main screen turn on, no args, delay
	255},												//     255 = max (500 ms) delay

	Rcmd1[] = {						  // 7735R init, part 1 (red or green tab)
		15,							  // 15 commands in list:
		ST77XX_SWRESET, ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
		150,						  //     150 ms delay
		ST77XX_SLPOUT, ST_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
		255,						  //     500 ms delay
		ST7735_FRMCTR1, 3,			  //  3: Framerate ctrl - normal mode, 3 arg:
		0x01, 0x2C, 0x2D,			  //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR2, 3,			  //  4: Framerate ctrl - idle mode, 3 args:
		0x01, 0x2C, 0x2D,			  //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR3, 6,			  //  5: Framerate - partial mode, 6 args:
		0x01, 0x2C, 0x2D,			  //     Dot inversion mode
		0x01, 0x2C, 0x2D,			  //     Line inversion mode
		ST7735_INVCTR, 1,			  //  6: Display inversion ctrl, 1 arg:
		0x07,						  //     No inversion
		ST7735_PWCTR1, 3,			  //  7: Power control, 3 args, no delay:
		0xA2, 0x02,					  //     -4.6V
		0x84,						  //     AUTO mode
		ST7735_PWCTR2, 1,			  //  8: Power control, 1 arg, no delay:
		0xC5,						  //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
		ST7735_PWCTR3, 2,			  //  9: Power control, 2 args, no delay:
		0x0A,						  //     Opamp current small
		0x00,						  //     Boost frequency
		ST7735_PWCTR4, 2,			  // 10: Power control, 2 args, no delay:
		0x8A,						  //     BCLK/2,
		0x2A,						  //     opamp current small & medium low
		ST7735_PWCTR5, 2,			  // 11: Power control, 2 args, no delay:
		0x8A, 0xEE, ST7735_VMCTR1, 1, // 12: Power control, 1 arg, no delay:
		0x0E, ST77XX_INVOFF, 0,		  // 13: Don't invert display, no args
		ST77XX_MADCTL, 1,			  // 14: Mem access ctl (directions), 1 arg:
		0xC8,						  //     row/col addr, bottom-top refresh
		ST77XX_COLMOD, 1,			  // 15: set color mode, 1 arg, no delay:
		0x05},						  //     16-bit color

	Rcmd2green[] = {		// 7735R init, part 2 (green tab only)
		2,					//  2 commands in list:
		ST77XX_CASET, 4,	//  1: Column addr set, 4 args, no delay:
		0x00, 0x02,			//     XSTART = 0
		0x00, 0x7F + 0x02,	//     XEND = 127
		ST77XX_RASET, 4,	//  2: Row addr set, 4 args, no delay:
		0x00, 0x01,			//     XSTART = 0
		0x00, 0x9F + 0x01}, //     XEND = 159

	Rcmd2red[] = {		 // 7735R init, part 2 (red tab only)
		2,				 //  2 commands in list:
		ST77XX_CASET, 4, //  1: Column addr set, 4 args, no delay:
		0x00, 0x00,		 //     XSTART = 0
		0x00, 0x7F,		 //     XEND = 127
		ST77XX_RASET, 4, //  2: Row addr set, 4 args, no delay:
		0x00, 0x00,		 //     XSTART = 0
		0x00, 0x9F},	 //     XEND = 159

	Rcmd2green144[] = {	 // 7735R init, part 2 (green 1.44 tab)
		2,				 //  2 commands in list:
		ST77XX_CASET, 4, //  1: Column addr set, 4 args, no delay:
		0x00, 0x00,		 //     XSTART = 0
		0x00, 0x7F,		 //     XEND = 127
		ST77XX_RASET, 4, //  2: Row addr set, 4 args, no delay:
		0x00, 0x00,		 //     XSTART = 0
		0x00, 0x7F},	 //     XEND = 127

	Rcmd2green160x80[] = { // 7735R init, part 2 (mini 160x80)
		2,				   //  2 commands in list:
		ST77XX_CASET, 4,   //  1: Column addr set, 4 args, no delay:
		0x00, 0x00,		   //     XSTART = 0
		0x00, 0x4F,		   //     XEND = 79
		ST77XX_RASET, 4,   //  2: Row addr set, 4 args, no delay:
		0x00, 0x00,		   //     XSTART = 0
		0x00, 0x9F},	   //     XEND = 159

	Rcmd3[] = {																		// 7735R init, part 3 (red or green tab)
		4,																			//  4 commands in list:
		ST7735_GMCTRP1, 16,															//  1: Gamma Adjustments (pos. polarity), 16 args + delay:
		0x02, 0x1c, 0x07, 0x12,														//     (Not entirely necessary, but provides
		0x37, 0x32, 0x29, 0x2d,														//      accurate colors)
		0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, ST7735_GMCTRN1, 16,			//  2: Gamma Adjustments (neg. polarity), 16 args + delay:
		0x03, 0x1d, 0x07, 0x06,														//     (Not entirely necessary, but provides
		0x2E, 0x2C, 0x29, 0x2D,														//      accurate colors)
		0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, ST77XX_NORON, ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
		10,																			//     10 ms delay
		ST77XX_DISPON, ST_CMD_DELAY,												//  4: Main screen turn on, no args w/delay
		100};																		//     100 ms delay

void ST7735_Reset()
{
	HAL_GPIO_WritePin(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_SET);
}

void ST7735_Select()
{
	HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
}

void ST7735_DeSelect()
{
	HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_SET);
}

void ST7735_RegCommand()
{
	HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_RESET);
}

void ST7735_RegData()
{
	HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
}

void ST7735_WriteCommand(uint8_t cmd)
{
	ST7735_RegCommand();
	HAL_SPI_Transmit(spiPort, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

void ST7735_WriteData(uint8_t *buff, size_t buff_size)
{
	ST7735_RegData();
	HAL_SPI_Transmit(spiPort, buff, buff_size, HAL_MAX_DELAY);
}

void ST7735_SendCommand(uint8_t commandByte, uint8_t *dataBytes,
						uint8_t numDataBytes)
{
	ST7735_Select();

	ST7735_WriteCommand(commandByte);
	ST7735_WriteData(dataBytes, numDataBytes);

	ST7735_DeSelect();
}

void ST7735_displayInit(const uint8_t *addr)
{

	uint8_t numCommands, cmd, numArgs;
	uint16_t ms;

	numCommands = *(addr++); // Number of commands to follow
	while (numCommands--)
	{								 // For each command...
		cmd = *(addr++);			 // Read command
		numArgs = *(addr++);		 // Number of args to follow
		ms = numArgs & ST_CMD_DELAY; // If hibit set, delay follows args
		numArgs &= ~ST_CMD_DELAY;	 // Mask out delay bit
		ST7735_SendCommand(cmd, addr, numArgs);
		addr += numArgs;

		if (ms)
		{
			ms = *(addr++); // Read post-command delay time (ms)
			if (ms == 255)
				ms = 500; // If 255, delay for 500 ms
			HAL_Delay(ms);
		}
	}
}

void setRotation(uint8_t m)
{
	uint8_t madctl = 0;

	rotation = m & 3; // can't be higher than 3

	// For ST7735 with GREEN TAB (including HalloWing)...
	if ((tabcolor == INITR_144GREENTAB) || (tabcolor == INITR_HALLOWING))
	{
		// ..._rowstart is 3 for rotations 0&1, 1 for rotations 2&3
		_rowstart = (rotation < 2) ? 3 : 1;
	}

	switch (rotation)
	{
	case 0:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
		{
			madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
		}
		else
		{
			madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR;
		}

		if (tabcolor == INITR_144GREENTAB)
		{
			_height = ST7735_TFTHEIGHT_128;
			_width = ST7735_TFTWIDTH_128;
		}
		else if (tabcolor == INITR_MINI160x80)
		{
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_80;
		}
		else
		{
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_128;
		}
		_xstart = _colstart;
		_ystart = _rowstart;
		break;
	case 1:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
		{
			madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
		}
		else
		{
			madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
		}

		if (tabcolor == INITR_144GREENTAB)
		{
			_width = ST7735_TFTHEIGHT_128;
			_height = ST7735_TFTWIDTH_128;
		}
		else if (tabcolor == INITR_MINI160x80)
		{
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_80;
		}
		else
		{
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_128;
		}
		_ystart = _colstart;
		_xstart = _rowstart;
		break;
	case 2:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
		{
			madctl = ST77XX_MADCTL_RGB;
		}
		else
		{
			madctl = ST7735_MADCTL_BGR;
		}

		if (tabcolor == INITR_144GREENTAB)
		{
			_height = ST7735_TFTHEIGHT_128;
			_width = ST7735_TFTWIDTH_128;
		}
		else if (tabcolor == INITR_MINI160x80)
		{
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_80;
		}
		else
		{
			_height = ST7735_TFTHEIGHT_160;
			_width = ST7735_TFTWIDTH_128;
		}
		_xstart = _colstart;
		_ystart = _rowstart;
		break;
	case 3:
		if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
		{
			madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
		}
		else
		{
			madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
		}

		if (tabcolor == INITR_144GREENTAB)
		{
			_width = ST7735_TFTHEIGHT_128;
			_height = ST7735_TFTWIDTH_128;
		}
		else if (tabcolor == INITR_MINI160x80)
		{
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_80;
		}
		else
		{
			_width = ST7735_TFTHEIGHT_160;
			_height = ST7735_TFTWIDTH_128;
		}
		_ystart = _colstart;
		_xstart = _rowstart;
		break;
	}

	ST7735_SendCommand(ST77XX_MADCTL, &madctl, 1);
}

void ST7735_initR(uint8_t options, SPI_HandleTypeDef *port)
{

	spiPort = port;
	ST7735_Select();
	ST7735_Reset();
	ST7735_displayInit(Rcmd1);
	if (options == INITR_GREENTAB)
	{
		ST7735_displayInit(Rcmd2green);
		_colstart = 2;
		_rowstart = 1;
	}
	else if ((options == INITR_144GREENTAB) || (options == INITR_HALLOWING))
	{
		_height = ST7735_TFTHEIGHT_128;
		_width = ST7735_TFTWIDTH_128;
		ST7735_displayInit(Rcmd2green144);
		_colstart = 2;
		_rowstart = 3; // For default rotation 0
	}
	else if (options == INITR_MINI160x80)
	{
		_height = ST7735_TFTWIDTH_80;
		_width = ST7735_TFTHEIGHT_160;
		ST7735_displayInit(Rcmd2green160x80);
		_colstart = 24;
		_rowstart = 0;
	}
	else
	{
		// colstart, rowstart left at default '0' values
		ST7735_displayInit(Rcmd2red);
	}
	ST7735_displayInit(Rcmd3);

	// Black tab, change MADCTL color filter
	if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80))
	{
		uint8_t data = 0xC0;
		ST7735_SendCommand(ST77XX_MADCTL, &data, 1);
	}

	if (options == INITR_HALLOWING)
	{
		// Hallowing is simply a 1.44" green tab upside-down:
		tabcolor = INITR_144GREENTAB;
		setRotation(2);
	}
	else
	{
		tabcolor = options;
		setRotation(0);
	}
}

void ST7735_setAddrWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{

	x += _xstart;
	y += _ystart;

	uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
	uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

	xa = __builtin_bswap32(xa);
	ya = __builtin_bswap32(ya);

	ST7735_WriteCommand(ST77XX_CASET);
	ST7735_WriteData(&xa, sizeof(xa));

	// row address set
	ST7735_WriteCommand(ST77XX_RASET);
	ST7735_WriteData(&ya, sizeof(ya));

	// write to RAM
	ST7735_WriteCommand(ST77XX_RAMWR);
}

void SPI_WRITE16(uint16_t d)
{
	uint8_t data[] = {d >> 8, d & 0xFF};
	HAL_SPI_Transmit(spiPort, data, sizeof(data), HAL_MAX_DELAY);
}

void ST7735_writePixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height))
	{
		// THEN set up transaction (if needed) and draw...
		// ST7735_Select();
		ST7735_setAddrWindow(x, y, 1, 1);
		ST7735_RegData();
		SPI_WRITE16(color);
		// ST7735_DeSelect();
	}
}

void drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height))
	{
		if (frameBuffer != NULL)
		{
			frameBuffer[x + y * _width] = (color >> 8) | (color << 8);
		}
		else
		{
			ST7735_Select();
			ST7735_writePixel(x, y, color);
			ST7735_DeSelect();
		}
	}
}
