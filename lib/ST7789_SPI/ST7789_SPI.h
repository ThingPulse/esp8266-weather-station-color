/**
The MIT License (MIT)
Copyright (c) 2019 by Daniel Eichhorn, ThingPulse
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please note: We are spending a lot of time to write and maintain open source codes
Please support us by buying our products from https://thingpulse.com/shop/

See more at https://thingpulse.com

Many thanks go to various contributors such as Adafruit, Waveshare.
*/

#ifndef _MINIGRAFX_ST7789H_
#define _MINIGRAFX_ST7789H_

#if ARDUINO >= 100
#include "Arduino.h"
#include "Print.h"
#else
#include "WProgram.h"
#endif
#include "DisplayDriver.h"
#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

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

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF
#define ST77XX_RED 0xF800
#define ST77XX_GREEN 0x07E0
#define ST77XX_BLUE 0x001F
#define ST77XX_CYAN 0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW 0xFFE0
#define ST77XX_ORANGE 0xFC00

// Color definitions
#define ST7789_BLACK 0x0000       /*   0,   0,   0 */
#define ST7789_NAVY 0x000F        /*   0,   0, 128 */
#define ST7789_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define ST7789_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define ST7789_MAROON 0x7800      /* 128,   0,   0 */
#define ST7789_PURPLE 0x780F      /* 128,   0, 128 */
#define ST7789_OLIVE 0x7BE0       /* 128, 128,   0 */
#define ST7789_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define ST7789_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define ST7789_BLUE 0x001F        /*   0,   0, 255 */
#define ST7789_GREEN 0x07E0       /*   0, 255,   0 */
#define ST7789_CYAN 0x07FF        /*   0, 255, 255 */
#define ST7789_RED 0xF800         /* 255,   0,   0 */
#define ST7789_MAGENTA 0xF81F     /* 255,   0, 255 */
#define ST7789_YELLOW 0xFFE0      /* 255, 255,   0 */
#define ST7789_WHITE 0xFFFF       /* 255, 255, 255 */
#define ST7789_ORANGE 0xFD20      /* 255, 165,   0 */
#define ST7789_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define ST7789_PINK 0xF81F

#if (TFT_CS == -1) && (ST7789_TFTWIDTH == 240) && (ST7789_TFTHEIGHT == 240)
// this specific CS-less dislpay works only in SPI MODE3
#define SPI_MODE SPI_MODE3
#else
#define SPI_MODE SPI_MODE0
#endif

#ifdef TFT_INVERSION
#define ST77XX_INVSET ST77XX_INVOFF 
#else
#define ST77XX_INVSET ST77XX_INVON
#endif

class ST7789_SPI : public DisplayDriver
{

public:
  ST7789_SPI(int8_t _CS, int8_t _DC, int8_t _MOSI, int8_t _SCLK,
             int8_t _RST, int8_t _MISO);
  ST7789_SPI(int8_t _CS, int8_t _DC, int8_t _RST = -1);

  void init(void);
  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void setRotation(uint8_t r);

  void writeBuffer(BufferInfo *bufferInfo);

  void pushColor(uint16_t color);
  void setFastRefresh(boolean isFastRefreshEnabled);

  void spiwrite(uint8_t);
  void writecommand(uint8_t c);
  void writedata(uint8_t d);

private:
  void displayInit(const uint8_t *addr);
  void sendCommand(uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes);

  boolean hwSPI;
  int32_t _cs, _dc, _rst, _mosi, _miso, _sclk;
};

#endif
