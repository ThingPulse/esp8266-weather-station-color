/**The MIT License (MIT)
Copyright (c) 2017 by Daniel Eichhorn
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
See more at https://blog.squix.org
*/

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>

/***
 * Install the following libraries through Arduino Library Manager
 * - Mini Grafx by Daniel Eichhorn
 * - ESP8266 WeatherStation by Daniel Eichhorn
 * - Json Streaming Parser by Daniel Eichhorn
 * - simpleDSTadjust by neptune2
 ***/

#include <JsonListener.h>
#include <WundergroundConditions.h>
#include <WundergroundForecast.h>
#include <WundergroundAstronomy.h>
#include <MiniGrafx.h>
#include <Carousel.h>
#include <ILI9341_SPI.h>

#include "ArialRounded.h"
#include "moonphases.h"
#include "weathericons.h"

/*****************************
 * Important: see settings.h to configure your settings!!!
 * ***************************/
#include "settings.h"


#define MINI_BLACK 0
#define MINI_WHITE 1
#define MINI_YELLOW 2
#define MINI_BLUE 3

#define MAX_FORECASTS 12

// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_YELLOW, // 2
                      0x7E3C/*ILI9341_BLUE*/}; //3

int SCREEN_WIDTH = 240;
int SCREEN_HEIGHT = 320;
// Limited to 4 colors due to memory constraints
int BITS_PER_PIXEL = 2; // 2^2 =  4 colors

// HOSTNAME for OTA update
#define HOSTNAME "ESP8266-OTA-"


ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC);
MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette);
Carousel carousel(&gfx, 0, 0, 240, 100);

WGConditions conditions;
WGForecast forecasts[MAX_FORECASTS];
WGAstronomy astronomy;

// Setup simpleDSTadjust Library rules
simpleDSTadjust dstAdjusted(StartRule, EndRule);

void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
void drawAstronomy();
void drawSeparator(uint16_t y);
const char* getMeteoconIconFromProgmem(String iconText);
const char* getMiniMeteoconIconFromProgmem(String iconText);
void drawForecast1(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
void drawForecast2(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
FrameCallback frames[] = { drawForecast1, drawForecast2 };
int frameCount = 2;
long lastDownloadUpdate = millis();

void updateCalendar();

String moonAgeImage = "";

void setup() {
  Serial.begin(115200);

  // The LED pin needs to set HIGH
  // Use this pin to save energy
  // Turn on the background LED
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  gfx.init();
  gfx.fillBuffer(MINI_BLACK);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_YELLOW);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(120, 160, "Connecting to WiFi");
  gfx.commit();
  
  carousel.setFrames(frames, frameCount);
  carousel.disableAllIndicators();

  //Manual Wifi
  WiFi.begin("yourssid", "yourpassw0rd");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  // update the weather information
  updateData();
}

long lastDrew = 0;
void loop() {
  gfx.fillBuffer(MINI_BLACK);
  drawTime();
  drawCurrentWeather();
  int remainingTimeBudget = carousel.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
  drawAstronomy();
  gfx.commit();


  // Check if we should update weather information
  if (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS) {
      updateData();
      lastDownloadUpdate = millis();
  }
}

// Update the internet based information and update screen
void updateData() {
 
  gfx.fillBuffer(MINI_BLACK);
  gfx.setFont(ArialRoundedMTBold_14);
  
  drawProgress(10, "Updating time...");
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);
  
  drawProgress(50, "Updating conditions...");
  WundergroundConditions *conditionsClient = new WundergroundConditions(IS_METRIC);
  conditionsClient->updateConditions(&conditions, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete conditionsClient;
  conditionsClient = nullptr;
  
  drawProgress(70, "Updating forecasts...");
  WundergroundForecast *forecastClient = new WundergroundForecast(IS_METRIC);
  forecastClient->updateForecast(forecasts, MAX_FORECASTS, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete forecastClient;
  forecastClient = nullptr;
  drawProgress(80, "Updating astronomy...");

  WundergroundAstronomy *astronomyClient = new WundergroundAstronomy(STYLE_12HR);
  astronomyClient->updateAstronomy(&astronomy, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete astronomyClient;
  astronomyClient = nullptr;
  moonAgeImage = String((char) (65 + 26 * (((15 + astronomy.moonAge.toInt()) % 30) / 30.0)));

  delay(1000);
}

// Progress bar helper
void drawProgress(uint8_t percentage, String text) {
  gfx.fillBuffer(MINI_BLACK);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_YELLOW);

  gfx.drawString(120, 146, text);
  gfx.setColor(MINI_WHITE);
  gfx.drawRect(10, 165, 240 - 20, 15);
  gfx.setColor(MINI_BLUE);
  gfx.fillRect(12, 167, 216 * percentage / 100, 11);

  gfx.commit();
}

// draws the clock
void drawTime() {

  /*gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.setFont(ArialRoundedMTBold_14);
  String date = conditions.date;
  gfx.drawString(120, 6, date);
  
  gfx.setFont(ArialRoundedMTBold_36);
  String time = timeClient.getHours() + ":" + timeClient.getMinutes() + ":" + timeClient.getSeconds();
  gfx.drawString(120, 20, time);*/
  char *dstAbbrev;
  char time_str[11];
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm * timeinfo = localtime (&now);
  
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setFont(ArialRoundedMTBold_14);
  String date = ctime(&now);
  date = date.substring(0,11) + String(1900 + timeinfo->tm_year);
  gfx.drawString(120, 6, date);
  
  gfx.setFont(ArialRoundedMTBold_36);
  
  if (STYLE_12HR) {
    int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d\n",hour, timeinfo->tm_min, timeinfo->tm_sec);
    gfx.drawString(120, 20, time_str);
  } else {
    sprintf(time_str, "%02d:%02d:%02d\n",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    gfx.drawString(120, 20, time_str);
  }

  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setColor(MINI_BLUE);
  if (STYLE_12HR) {
    sprintf(time_str, "%s\n%s", dstAbbrev, timeinfo->tm_hour>=12?"PM":"AM");
    gfx.drawString(195, 27, time_str);
  } else {
    sprintf(time_str, "%s", dstAbbrev);
    gfx.drawString(195, 27, time_str);  // Known bug: Cuts off 4th character of timezone abbreviation
  }
}

// draws current weather information
void drawCurrentWeather() {
  gfx.setTransparentColor(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(0, 55, getMeteoconIconFromProgmem(conditions.weatherIcon));
  // Weather Text
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_YELLOW);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(220, 76, conditions.weatherText);

  gfx.setFont(ArialRoundedMTBold_36);
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  String degreeSign = "°F";
  if (IS_METRIC) {
    degreeSign = "°C";
  }
  String temp = conditions.currentTemp + degreeSign;
  gfx.drawString(220, 89, temp);

}

void drawForecast1(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y) {
  drawForecastDetail(x + 10, y + 165, 0);
  drawForecastDetail(x + 95, y + 165, 2);
  drawForecastDetail(x + 180, y + 165, 4);
}

void drawForecast2(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y) {
  drawForecastDetail(x + 10, y + 165, 6);
  drawForecastDetail(x + 95, y + 165, 8);
  drawForecastDetail(x + 180, y + 165, 10); 
}


// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex) {
  gfx.setColor(MINI_YELLOW);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  String day = forecasts[dayIndex].forecastTitle.substring(0, 3);
  day.toUpperCase();
  gfx.drawString(x + 25, y - 15, day);

  gfx.setColor(MINI_WHITE);
  gfx.drawString(x + 25, y, forecasts[dayIndex].forecastLowTemp + "|" + forecasts[dayIndex].forecastHighTemp);

  gfx.drawPalettedBitmapFromPgm(x, y + 15, getMiniMeteoconIconFromProgmem(forecasts[dayIndex].forecastIcon));
  gfx.setColor(MINI_BLUE);
  gfx.drawString(x + 25, y + 60, forecasts[dayIndex].PoP + "%");
}

// draw moonphase and sunrise/set and moonrise/set
void drawAstronomy() {
  
  gfx.setFont(MoonPhases_Regular_36);
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(120, 275, moonAgeImage);
  
  gfx.setColor(MINI_WHITE);
  gfx.setFont(ArialRoundedMTBold_14); 
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(120, 250, astronomy.moonPhase); 
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(5, 250, "Sun");
  gfx.setColor(MINI_WHITE);
  gfx.drawString(5, 276, astronomy.sunriseTime);
  gfx.drawString(5, 291, astronomy.sunsetTime);

  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(235, 250, "Moon");
  gfx.setColor(MINI_WHITE);
  gfx.drawString(235, 276, astronomy.moonriseTime);
  gfx.drawString(235, 291, astronomy.moonsetTime);
  
}

// Helper function, should be part of the weather station library and should disappear soon
const char* getMeteoconIconFromProgmem(String iconText) {
  
  if (iconText == "chanceflurries") return chanceflurries;
  if (iconText == "chancerain") return chancerain;
  if (iconText == "chancesleet") return chancesleet;
  if (iconText == "chancesnow") return chancesnow;
  if (iconText == "chancetstorms") return chancestorms;
  if (iconText == "cloudy") return clear;
  if (iconText == "cloudy") return cloudy;
  if (iconText == "flurries") return flurries;
  if (iconText == "fog") return fog;
  if (iconText == "hazy") return hazy;
  if (iconText == "mostlycloudy") return mostlycloudy;
  if (iconText == "mostlysunny") return mostlysunny;
  if (iconText == "partlycloudy") return partlycloudy;
  if (iconText == "partlysunny") return partlysunny;
  if (iconText == "sleet") return sleet;
  if (iconText == "rain") return rain;
  if (iconText == "snow") return snow;
  if (iconText == "sunny") return sunny;
  if (iconText == "tstorms") return tstorms;
  

  return unknown;
}
const char* getMiniMeteoconIconFromProgmem(String iconText) {
  if (iconText == "chanceflurries") return minichanceflurries;
  if (iconText == "chancerain") return minichancerain;
  if (iconText == "chancesleet") return minichancesleet;
  if (iconText == "chancesnow") return minichancesnow;
  if (iconText == "chancetstorms") return minichancestorms;
  if (iconText == "clear") return miniclear;
  if (iconText == "cloudy") return minicloudy;
  if (iconText == "flurries") return miniflurries;
  if (iconText == "fog") return minifog;
  if (iconText == "hazy") return minihazy;
  if (iconText == "mostlycloudy") return minimostlycloudy;
  if (iconText == "mostlysunny") return minimostlysunny;
  if (iconText == "partlycloudy") return minipartlycloudy;
  if (iconText == "partlysunny") return minipartlysunny;
  if (iconText == "sleet") return minisleet;
  if (iconText == "rain") return minirain;
  if (iconText == "snow") return minisnow;
  if (iconText == "sunny") return minisunny;
  if (iconText == "tstorms") return minitstorms;
  

  return miniunknown;
}


