/**The MIT License (MIT)
Copyright (c) 2015 by Daniel Eichhorn
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
See more at http://blog.squix.ch
*/

#include <Adafruit_ILI9341.h>
#include <FS.h>

#ifndef _GFX_UI_H
#define _GFX_UI_H


#define BUFFPIXEL 20

enum TextAlignment {
  LEFT, CENTER, RIGHT
};

class GfxUi {
  public:
    GfxUi(Adafruit_ILI9341 * tft);
    void drawString(int x, int y, char *text);
    void drawString(int x, int y, String text);
    void setTextAlignment(TextAlignment alignment);
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);
    void drawBmp(String filename, uint8_t x, uint16_t y);
    void drawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percentage, uint16_t frameColor, uint16_t barColor);

  private:
    Adafruit_ILI9341 * _tft;
    TextAlignment _alignment = LEFT;
    uint16_t _textColor;
    uint16_t _backgroundColor;
    uint16_t read16(File &f);
    uint32_t read32(File &f);

};

#endif

