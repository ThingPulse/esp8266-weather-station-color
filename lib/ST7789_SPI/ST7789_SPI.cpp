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
#include "ST7789_SPI.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif

#ifndef ARDUINO_STM32_FEATHER
#include "pins_arduino.h"
#include "wiring_private.h"
#endif

#include <limits.h>
#include <SPI.h>

#define FILLARRAY(a, n) a[0] = n, memcpy(((char *)a) + sizeof(a[0]), a, sizeof(a) - sizeof(a[0]));

// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
#ifdef SPI_HAS_TRANSACTION
static inline void spi_begin(void) __attribute__((always_inline));
static inline void spi_begin(void)
{
#if defined(ARDUINO_ARCH_ARC32)
  // max speed!
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE));
#else
  // max speed!
  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE));
#endif
}
static inline void spi_end(void) __attribute__((always_inline));
static inline void spi_end(void)
{
  SPI.endTransaction();
}
#else
#define spi_begin()
#define spi_end()
#endif

// Constructor when using software SPI.  All output pins are configurable.
ST7789_SPI::ST7789_SPI(int8_t cs, int8_t dc, int8_t mosi,
                       int8_t sclk, int8_t rst, int8_t miso) : DisplayDriver(ST7789_TFTWIDTH, ST7789_TFTHEIGHT)
{
  _cs = cs;
  _dc = dc;
  _mosi = mosi;
  _miso = miso;
  _sclk = sclk;
  _rst = rst;
  hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
ST7789_SPI::ST7789_SPI(int8_t cs, int8_t dc, int8_t rst) : DisplayDriver(ST7789_TFTWIDTH, ST7789_TFTHEIGHT)
{
  _cs = cs;
  _dc = dc;
  _rst = rst;
  hwSPI = true;
  _mosi = _sclk = 0;
}

static const uint8_t PROGMEM
    generic_st7789[] = {                 // Init commands for 7789 screens
        9,                               //  9 commands in list:
        ST77XX_SWRESET, ST_CMD_DELAY,    //  1: Software reset, no args, w/delay
        150,                             //     ~150 ms delay
        ST77XX_SLPOUT, ST_CMD_DELAY,     //  2: Out of sleep mode, no args, w/delay
        10,                              //      10 ms delay
        ST77XX_COLMOD, 1 + ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
        0x55,                            //     16-bit color
        10,                              //     10 ms delay
        ST77XX_MADCTL, 1,                //  4: Mem access ctrl (directions), 1 arg:
        0x08,                            //     Row/col addr, bottom-top refresh
        ST77XX_CASET, 4,                 //  5: Column addr set, 4 args, no delay:
        0x00,
        0, //     XSTART = 0
        0,
        240,             //     XEND = 240
        ST77XX_RASET, 4, //  6: Row addr set, 4 args, no delay:
        0x00,
        0, //     YSTART = 0
        320 >> 8,
        320 & 0xFF,                 //     YEND = 320
        ST77XX_INVSET, ST_CMD_DELAY, //  7: hack
        10,
        ST77XX_NORON, ST_CMD_DELAY,  //  8: Normal display on, no args, w/delay
        10,                          //     10 ms delay
        ST77XX_DISPON, ST_CMD_DELAY, //  9: Main screen turn on, no args, delay
        10};                         //    10 ms delay

void ST7789_SPI::spiwrite(uint8_t c)
{

  //Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");

  if (hwSPI)
  {
#if defined(__AVR__)
#ifndef SPI_HAS_TRANSACTION
    uint8_t backupSPCR = SPCR;
    SPCR = mySPCR;
#endif
    SPDR = c;
    while (!(SPSR & _BV(SPIF)))
      ;
#ifndef SPI_HAS_TRANSACTION
    SPCR = backupSPCR;
#endif
#else
    SPI.transfer(c);
#endif
  }
  else
  {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_ARC32)
    for (uint8_t bit = 0x80; bit; bit >>= 1)
    {
      if (c & bit)
      {
        digitalWrite(_mosi, HIGH);
      }
      else
      {
        digitalWrite(_mosi, LOW);
      }
      digitalWrite(_sclk, HIGH);
      digitalWrite(_sclk, LOW);
    }
#else
    // Fast SPI bitbang swiped from LPD8806 library
    for (uint8_t bit = 0x80; bit; bit >>= 1)
    {
      if (c & bit)
      {
        //digitalWrite(_mosi, HIGH);
        *mosiport |= mosipinmask;
      }
      else
      {
        //digitalWrite(_mosi, LOW);
        *mosiport &= ~mosipinmask;
      }
      //digitalWrite(_sclk, HIGH);
      *clkport |= clkpinmask;
      //digitalWrite(_sclk, LOW);
      *clkport &= ~clkpinmask;
    }
#endif
  }
}

void ST7789_SPI::setFastRefresh(boolean isFastRefreshEnabled)
{
  // Not enabled at the moment
}

void ST7789_SPI::writecommand(uint8_t c)
{
#if defined(USE_FAST_PINIO)
  *dcport &= ~dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, LOW);
  digitalWrite(_sclk, LOW);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(c);

#if defined(USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif
}

void ST7789_SPI::writedata(uint8_t c)
{
#if defined(USE_FAST_PINIO)
  *dcport |= dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(c);

#if defined(USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif
}

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80

void ST7789_SPI::sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                             uint8_t numDataBytes)
{
  writecommand(commandByte);

  for (int i = 0; i < numDataBytes; i++)
  {
    writedata(pgm_read_byte(dataBytes++));
  }
}

void ST7789_SPI::displayInit(const uint8_t *addr)
{

  uint8_t numCommands, cmd, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++); // Number of commands to follow
  while (numCommands--)
  {                                  // For each command...
    cmd = pgm_read_byte(addr++);     // Read command
    numArgs = pgm_read_byte(addr++); // Number of args to follow
    ms = numArgs & ST_CMD_DELAY;     // If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;        // Mask out delay bit
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;

    if (ms)
    {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if (ms == 255)
        ms = 500; // If 255, delay for 500 ms
      delay(ms);
    }
  }
}

void ST7789_SPI::init(void)
{
  if (_rst > 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
  }

  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);

#if defined(USE_FAST_PINIO)
  csport = portOutputRegister(digitalPinToPort(_cs));
  cspinmask = digitalPinToBitMask(_cs);
  dcport = portOutputRegister(digitalPinToPort(_dc));
  dcpinmask = digitalPinToBitMask(_dc);
#endif

  if (hwSPI)
  { // Using hardware SPI
    SPI.begin();

#ifndef SPI_HAS_TRANSACTION
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE);
#if defined(_AVR__)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
    mySPCR = SPCR;
#elif defined(TEENSYDUINO)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
#elif defined(__arm__)
    SPI.setClockDivider(11); // 8-ish MHz (full! speed!)
#endif
#endif
  }
  else
  {
    pinMode(_sclk, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);

#if defined(USE_FAST_PINIO)
    clkport = portOutputRegister(digitalPinToPort(_sclk));
    clkpinmask = digitalPinToBitMask(_sclk);
    mosiport = portOutputRegister(digitalPinToPort(_mosi));
    mosipinmask = digitalPinToBitMask(_mosi);
    *clkport &= ~clkpinmask;
    *mosiport &= ~mosipinmask;
#endif
  }

  // toggle RST low to reset
  if (_rst > 0)
  {
    digitalWrite(_rst, HIGH);
    delay(5);
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(150);
  }

  if (hwSPI)
    spi_begin();

  displayInit(generic_st7789);

  if (hwSPI)
    spi_end();
}

void ST7789_SPI::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                               uint16_t y1)
{
  writecommand(ST77XX_CASET); // Column addr set
  writedata(x0 >> 8);
  writedata(x0 & 0xFF); // XSTART
  writedata(x1 >> 8);
  writedata(x1 & 0xFF); // XEND

  writecommand(ST77XX_RASET); // Row addr set
  writedata(y0 >> 8);
  writedata(y0); // YSTART
  writedata(y1 >> 8);
  writedata(y1); // YEND

  writecommand(ST77XX_RAMWR); // write to RAM
}

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

void ST7789_SPI::setRotation(uint8_t m)
{

  if (hwSPI)
    spi_begin();

  // writecommand(ST7789_MADCTL);
  // rotation = m % 4; // can't be higher than 3
  // switch (rotation) {
  //  case 0:
  //    writedata(MADCTL_MX | MADCTL_BGR);
  //    _width  = ST7789_TFTWIDTH;
  //    _height = ST7789_TFTHEIGHT;
  //    break;
  //  case 1:
  //    writedata(MADCTL_MV | MADCTL_BGR);
  //    _width  = ST7789_TFTHEIGHT;
  //    _height = ST7789_TFTWIDTH;
  //    break;
  // case 2:
  //   writedata(MADCTL_MY | MADCTL_BGR);
  //    _width  = ST7789_TFTWIDTH;
  //    _height = ST7789_TFTHEIGHT;
  //   break;
  //  case 3:
  //    writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
  //    _width  = ST7789_TFTHEIGHT;
  //    _height = ST7789_TFTWIDTH;
  //    break;
  // }

  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  switch (rotation)
  {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    _width = ST7789_TFTWIDTH;
    _height = ST7789_TFTHEIGHT;
    break;
  case 1:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _width = ST7789_TFTHEIGHT;
    _height = ST7789_TFTWIDTH;
    break;
  case 2:
    madctl = ST77XX_MADCTL_RGB;
    _width = ST7789_TFTWIDTH;
    _height = ST7789_TFTHEIGHT;
    break;
  case 3:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _width = ST7789_TFTHEIGHT;
    _height = ST7789_TFTWIDTH;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);

  if (hwSPI)
    spi_end();
}

void ST7789_SPI::pushColor(uint16_t color)
{
  if (hwSPI)
    spi_begin();

  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);

  spiwrite(color >> 8);
  spiwrite(color);

  digitalWrite(_cs, HIGH);

  if (hwSPI)
    spi_end();
}

void ST7789_SPI::writeBuffer(BufferInfo *bufferInfo)
{
  if (hwSPI)
    spi_begin();
  uint8_t *buffer = bufferInfo->buffer;
  uint16_t bufferWidth = bufferInfo->bufferWidth;
  uint16_t bufferHeight = bufferInfo->bufferHeight;
  uint16_t xPos = bufferInfo->targetX;
  uint16_t yPos = bufferInfo->targetY;
  uint16_t *palette = bufferInfo->palette;
  uint8_t bitsPerPixel = bufferInfo->bitsPerPixel;

  setAddrWindow(xPos, yPos, xPos + bufferWidth - 1, yPos + bufferHeight - 1);

  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
  if (bitsPerPixel == 16)
  {
    SPI.writeBytes(buffer, bufferWidth * bufferHeight * 2);
    /*for (uint16_t y = 0; y < bufferHeight; y++) {
        for (uint16_t x = 0; x < bufferWidth; x++) {

        }
      }*/
  }
  else
  {
    // line buffer is in 16bit target format
    uint8_t lineBuffer[_width * 2];
    uint16_t pos;
    uint8_t bufferByte;
    uint8_t paletteEntry;
    uint16_t color;
    uint8_t shift;
    uint8_t mask = (1 << bitsPerPixel) - 1;
    uint8_t packagesPerBytes = 8 / bitsPerPixel;
    uint16_t bytePos = 0;
    uint16_t pixelCounter = 0;
    uint16_t bufferSize = bufferWidth * bufferHeight / packagesPerBytes;
    uint8_t bytesPerLine = bufferWidth / packagesPerBytes;
    uint16_t x = 0;
    for (uint16_t y = 0; y < bufferHeight; y++)
    {

      for (uint16_t b = 0; b < bytesPerLine; b++)
      {

        for (uint8_t p = 0; p < packagesPerBytes; p++)
        {
          x = b * packagesPerBytes + p;
          bufferByte = buffer[bytePos];
          shift = p * bitsPerPixel;
          paletteEntry = (bufferByte >> shift) & mask;
          color = palette[paletteEntry];
          lineBuffer[x * 2] = color >> 8;
          lineBuffer[x * 2 + 1] = color;
        }
        bytePos++;
      }
      SPI.writeBytes(lineBuffer, bufferWidth * 2);
    }
  }
  digitalWrite(_cs, HIGH);

  if (hwSPI)
    spi_end();
}
