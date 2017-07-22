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

#ifdef HAVE_TOUCHPAD
  #include <XPT2046_Touchscreen.h>
  #include "TouchController.h"
#endif 

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
#include <WundergroundAlerts.h>
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
#define MAX_ALERTS 1

// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_YELLOW, // 2
                      0x7E3C
                      }; //3

int SCREEN_WIDTH = 240;
int SCREEN_HEIGHT = 320;
// Limited to 4 colors due to memory constraints
int BITS_PER_PIXEL = 2; // 2^2 =  4 colors

#ifndef BATT
  ADC_MODE(ADC_VCC);
#endif

ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC);
MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette);
Carousel carousel(&gfx, 0, 0, 240, 100);

#ifdef HAVE_TOUCHPAD
  XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
  TouchController touchController(&ts);

  void calibrationCallback(int16_t x, int16_t y);
  CalibrationCallback calibration = &calibrationCallback;
  
#endif 

WGConditions conditions;
WGForecast forecasts[MAX_FORECASTS];
WGAstronomy astronomy;
WGAlert alerts[MAX_ALERTS];

// Setup simpleDSTadjust Library rules
simpleDSTadjust dstAdjusted(StartRule, EndRule);

void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
void drawAstronomy();
void drawAlert();
void drawCurrentWeatherDetail();
void drawLabelValue(uint8_t line, String label, String value);
void drawForecastTable(uint8_t start);
void drawAbout();
void drawSeparator(uint16_t y);
const char* getMeteoconIconFromProgmem(String iconText);
const char* getMiniMeteoconIconFromProgmem(String iconText);
void drawForecast1(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
void drawForecast2(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
FrameCallback frames[] = { drawForecast1, drawForecast2 };
int frameCount = 2;

// how many different screens do we have?
int screenCount = 5;
long lastDownloadUpdate = millis();

String moonAgeImage = "";
uint16_t screen = 0;
long timerPress;
bool canBtnPress;

void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  //Manual Wifi
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (i>80) i=0;
    drawProgress(i,"Connecting to WiFi");
    i+=10;
    Serial.print(".");
  }
}

void setup() {
  Serial.begin(115200);

  // The LED pin needs to set HIGH
  // Use this pin to save energy
  // Turn on the background LED
  Serial.println(TFT_LED);
  pinMode(TFT_LED, OUTPUT);

  #ifdef TFT_LED_LOW
    digitalWrite(TFT_LED, LOW);    // LOW to Turn on;
  #else
    digitalWrite(TFT_LED, HIGH);    // HIGH to Turn on;
  #endif
  
  gfx.init();
  gfx.fillBuffer(MINI_BLACK);
  gfx.commit();

#ifdef HAVE_TOUCHPAD 
  ts.begin();
  SPIFFS.begin();
  //SPIFFS.remove("/calibration.txt");
  boolean isCalibrationAvailable = touchController.loadCalibration();
  if (!isCalibrationAvailable) {
    Serial.println("Calibration not available");
    touchController.startCalibration(&calibration);
    while (!touchController.isCalibrationFinished()) {
      gfx.fillBuffer(0);
      gfx.setColor(MINI_YELLOW);
      gfx.setTextAlignment(TEXT_ALIGN_CENTER);
      gfx.drawString(120, 160, "Please calibrate\ntouch screen by\ntouch point");
      touchController.continueCalibration();
      gfx.commit();
      yield();
    }
    touchController.saveCalibration();
  }
#endif  

  carousel.setFrames(frames, frameCount);
  carousel.disableAllIndicators();

  // update the weather information
  updateData();
  timerPress = millis();
  canBtnPress = true;
}

long lastDrew = 0;
bool btnClick;

#ifdef LM75
  #include <Wire.h>
  float temperature = 0.0;

  float lm75() {
    unsigned int data[2];

    Wire.begin(SDA_PIN,SCL_PIN);
    Wire.setClock(700000);
  
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select temperature data register
    Wire.write(0x00);
    // Stop I2C Transmission
    Wire.endTransmission();
  
    // Request 2 bytes of data
    Wire.requestFrom(Addr,2);

    // Read 2 bytes of data
    // temp msb, temp lsb
    if(Wire.available()==2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
    } 
    //  Serial.println(data[0]);  
    //  Serial.println(data[1]);
    
    // Convert the data to 9-bits
    int temp = (data[0] * 256 + (data[1] & 0x80)) / 128;
    if (temp > 255){
      temp -= 512;
    }
    float cTemp = temp * 0.5;
    float fTemp = cTemp * 1.8 + 32;
  
    // Output data to serial monitor
    //  Serial.print("Temperature in Celsius:  ");
    //  Serial.print(cTemp);
    //  Serial.println(" C");
    //  Serial.print("Temperature in Fahrenheit:  ");
    //  Serial.print(fTemp);
    //  Serial.println(" F");  
    if (IS_METRIC) 
      return cTemp; 
    else
      return fTemp;    
  }
#endif

void loop() {
#ifdef HAVE_TOUCHPAD
  if (touchController.isTouched(500)) {
    TS_Point p = touchController.getPoint();
    if (p.y < 80) {
      IS_STYLE_12HR = !IS_STYLE_12HR;
    } else {
      screen = (screen + 1) % screenCount;
    }
  }
#else
  pinMode(BTN_1, INPUT_PULLUP);
  delay(5);
  int btnState = digitalRead(BTN_1);
  
  if (btnState == LOW){
    if(canBtnPress){
      timerPress = millis();
      canBtnPress = false;
    } else {
        if ((!btnClick) && ((millis() - timerPress)>1000)) {
          btnClick = true;
        } 
    }
  }else if(!canBtnPress){
    canBtnPress = true;
    btnClick = false;
    if ((millis() - timerPress)<800) {
        screen = (screen + 1) % screenCount;        
    }
  } 
#endif  

  gfx.fillBuffer(MINI_BLACK);
  if (screen == 0) {
    drawTime();
    int remainingTimeBudget = carousel.update();
    if (remainingTimeBudget > 0) {
      // You can do some work here
      // Don't do stuff if you are below your
      // time budget.
      delay(remainingTimeBudget);
    }
    drawCurrentWeather();
    drawAstronomy();
  } else if (screen == 1) {
    drawCurrentWeatherDetail();
  } else if (screen == 2) {
    drawForecastTable(0);
  } else if (screen == 3) {
    drawForecastTable(6);
  } else if (screen == 4) {
    drawAbout();
  }
  gfx.commit();

  // Check if we should update weather information
  if (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS) {
      updateData();
      lastDownloadUpdate = millis();
  }

  if (screen >0 && millis() - timerPress >= 10000){ // after 2 minutes go to sleep
    screen = 0;
  }

  if (SLEEP_INTERVAL_SECS && millis() - timerPress >= SLEEP_INTERVAL_SECS * 1000){ // after 2 minutes go to sleep
    drawProgress(25,"Going to Sleep!");
    delay(1000);
    drawProgress(50,"Going to Sleep!");
    delay(1000);
    drawProgress(75,"Going to Sleep!");
    delay(1000);    
    drawProgress(100,"Going to Sleep!");
    // go to deepsleep for xx minutes or 0 = permanently
    ESP.deepSleep(0,  WAKE_RF_DEFAULT);                       // 0 delay = permanently to sleep
  }  
}

// Update the internet based information and update screen
void updateData() {
  WiFi.mode(WIFI_STA);
  connectWifi();
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
  WundergroundAstronomy *astronomyClient = new WundergroundAstronomy(IS_STYLE_12HR);
  astronomyClient->updateAstronomy(&astronomy, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete astronomyClient;
  astronomyClient = nullptr;
  moonAgeImage = String((char) (65 + 26 * (((15 + astronomy.moonAge.toInt()) % 30) / 30.0)));

  drawProgress(90, "Updating alerts...");
  WundergroundAlerts *alertClient = new WundergroundAlerts();
  alertClient->updateAlerts(alerts, MAX_ALERTS, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete alertClient;
  alertClient = nullptr;

  WiFi.mode(WIFI_OFF);
  delay(1000);
}

// Progress bar helper
void drawProgress(uint8_t percentage, String text) {
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(23, 30, SquixLogo);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 80, "https://blog.squix.org");
  gfx.setColor(MINI_YELLOW);

  gfx.drawString(120, 146, text);
  gfx.setColor(MINI_WHITE);
  gfx.drawRect(10, 168, 240 - 20, 15);
  gfx.setColor(MINI_BLUE);
  gfx.fillRect(12, 170, 216 * percentage / 100, 11);

  gfx.commit();
}

// draws the clock
void drawTime() {

  char *dstAbbrev;
  char time_str[11];
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm * timeinfo = localtime (&now);

  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_WHITE);
  String date = ctime(&now);
  date = date.substring(0,11) + String(1900 + timeinfo->tm_year);
  gfx.drawString(120, 6, date);

  gfx.setFont(ArialRoundedMTBold_36);

  if (IS_STYLE_12HR) {
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
  if (IS_STYLE_12HR) {
    sprintf(time_str, "%s\n%s", dstAbbrev, timeinfo->tm_hour>=12?"PM":"AM");
    gfx.drawString(195, 27, time_str);
  } else {
    sprintf(time_str, "%s", dstAbbrev);
    gfx.drawString(195, 27, time_str);  // Known bug: Cuts off 4th character of timezone abbreviation
  }

  drawWifiQuality();
  #ifdef BATT
    drawBattery();
  #endif 
}

// draws current weather information
void drawCurrentWeather() {
  gfx.setTransparentColor(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(0, 55, getMeteoconIconFromProgmem(conditions.weatherIcon));
  // Weather Text

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_BLUE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(220, 65, DISPLAYED_CITY_NAME);

  gfx.setFont(ArialRoundedMTBold_36);
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  String degreeSign = "°F";
  if (IS_METRIC) {
    degreeSign = "°C";
  }

  #ifdef LM75
    if (canBtnPress) temperature = lm75(); 
    String temp = temperature + degreeSign;
  #else
    String temp = conditions.currentTemp + degreeSign;
  #endif
      
  gfx.drawString(220, 78, temp);

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_YELLOW);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(220, 118, conditions.weatherText);

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

void drawAlert() {

}

void drawCurrentWeatherDetail() {
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 2, "Current Conditions");

  //gfx.setTransparentColor(MINI_BLACK);
  //gfx.drawPalettedBitmapFromPgm(0, 20, getMeteoconIconFromProgmem(conditions.weatherIcon));

  String degreeSign = "°F";
  if (IS_METRIC) {
    degreeSign = "°C";
  }
  // String weatherIcon;
  // String weatherText;
  drawLabelValue(0, "Temperature:", conditions.currentTemp + degreeSign);
  drawLabelValue(1, "Feels Like:", conditions.feelslike + degreeSign);
  drawLabelValue(2, "Dew Point:", conditions.dewPoint + degreeSign);
  drawLabelValue(3, "Wind Speed:", conditions.windSpeed);
  drawLabelValue(4, "Wind Dir:", conditions.windDir);
  drawLabelValue(5, "Humidity:", conditions.humidity);
  drawLabelValue(6, "Pressure:", conditions.pressure);
  drawLabelValue(7, "Precipitation:", conditions.precipitationToday);
  drawLabelValue(8, "UV:", conditions.UV);

  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(15, 185, "Description: ");
  gfx.setColor(MINI_WHITE);
  gfx.drawStringMaxWidth(15, 200, 240 - 2 * 15, forecasts[0].forecastText);
}

void drawLabelValue(uint8_t line, String label, String value) {
  const uint8_t labelX = 15;
  const uint8_t valueX = 150;
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(labelX, 30 + line * 15, label);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(valueX, 30 + line * 15, value);
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if(dbm <= -100) {
      return 0;
  } else if(dbm >= -50) {
      return 100;
  } else {
      return 2 * (dbm + 100);
  }
}

void drawWifiQuality() {
  int8_t quality = getWifiQuality();
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);  
  gfx.drawString(228, 9, String(quality) + "%");
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        gfx.setPixel(230 + 2 * i, 18 - j);
      }
    }
  }
}

void drawBattery() {
  uint8_t percentage = 100;
  float power = analogRead(A0) * 49 / 10240.0;
  if (power > 4.15) percentage = 100;
  else if (power < 3.7) percentage = 0;
  else percentage = (power - 3.7) * 100 / (4.15-3.7);
  
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);  
  gfx.drawString(26, 9, String(percentage) + "%");
  gfx.drawRect(1, 11, 18, 9);
  gfx.drawLine(21,13,21,17);  
  gfx.drawLine(22,13,22,17);  
  gfx.setColor(MINI_BLUE); 
  gfx.fillRect(3, 13, 15 * percentage / 100, 5);
}

void drawForecastTable(uint8_t start) {
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 2, "Forecasts");
  uint16_t y = 0;

  String degreeSign = "°F";
  if (IS_METRIC) {
    degreeSign = "°C";
  }
  for (uint8_t i = start; i < start + 6; i++) {
    gfx.setTextAlignment(TEXT_ALIGN_LEFT);
    y = 30 + (i - start) * 45;
    if (y > 320) {
      break;
    }
    gfx.drawPalettedBitmapFromPgm(0, y, getMiniMeteoconIconFromProgmem(forecasts[i].forecastIcon));

    gfx.setColor(MINI_YELLOW);
    gfx.setFont(ArialRoundedMTBold_14);

    gfx.drawString(50, y, forecasts[i].forecastTitle);
    gfx.setColor(MINI_WHITE);
    gfx.drawString(50, y + 15, getShortText(forecasts[i].forecastIcon));
    gfx.setColor(MINI_WHITE);
    gfx.setTextAlignment(TEXT_ALIGN_RIGHT);

    String temp = "";
    if (i % 2 == 0) {
      temp = forecasts[i].forecastHighTemp;
    } else {
      temp = forecasts[i - 1].forecastLowTemp;
    }
    gfx.drawString(235, y, temp + degreeSign);
    /*gfx.setColor(MINI_WHITE);
    gfx.drawString(x + 25, y, forecasts[dayIndex].forecastLowTemp + "|" + forecasts[dayIndex].forecastHighTemp);

    gfx.drawPalettedBitmapFromPgm(x, y + 15, getMiniMeteoconIconFromProgmem(forecasts[dayIndex].forecastIcon));*/
    gfx.setColor(MINI_BLUE);
    gfx.drawString(235, y + 15, forecasts[i].PoP + "%");

  }
}

void drawAbout() {
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(23, 30, SquixLogo);

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 80, "https://blog.squix.org");

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  drawLabelValue(7, "Heap Mem:", String(ESP.getFreeHeap() / 1024)+"kb");
  drawLabelValue(8, "Flash Mem:", String(ESP.getFlashChipRealSize() / 1024 / 1024) + "MB");
  drawLabelValue(9, "WiFi Strength:", String(WiFi.RSSI()) + "dB");
  drawLabelValue(10, "Chip ID:", String(ESP.getChipId()));
  
  #ifdef BATT
    drawLabelValue(11, "Battery: ", String(analogRead(A0) * 49 / 10240.0) +"V");
  #else
    drawLabelValue(11, "VCC: ", String(ESP.getVcc() / 1024.0) +"V");
  #endif     
  
  drawLabelValue(12, "CPU Freq.: ", String(ESP.getCpuFreqMHz()) + "MHz");
  char time_str[15];
  const uint32_t millis_in_day = 1000 * 60 * 60 * 24;
  const uint32_t millis_in_hour = 1000 * 60 * 60;
  const uint32_t millis_in_minute = 1000 * 60;
  uint8_t days = millis() / (millis_in_day);
  uint8_t hours = (millis() - (days * millis_in_day)) / millis_in_hour;
  uint8_t minutes = (millis() - (days * millis_in_day) - (hours * millis_in_hour)) / millis_in_minute;
  sprintf(time_str, "%2dd%2dh%2dm", days, hours, minutes);
  drawLabelValue(13, "Uptime: ", time_str);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(15, 250, "Last Reset: ");
  gfx.setColor(MINI_WHITE);
  gfx.drawStringMaxWidth(15, 265, 240 - 2 * 15, ESP.getResetInfo());
}

void calibrationCallback(int16_t x, int16_t y) {
  gfx.setColor(1);
  gfx.fillCircle(x, y, 10);
}

// Helper function, should be part of the weather station library and should disappear soon
const char* getMeteoconIconFromProgmem(String iconText) {

  if (iconText == "chanceflurries") return chanceflurries;
  if (iconText == "chancerain") return chancerain;
  if (iconText == "chancesleet") return chancesleet;
  if (iconText == "chancesnow") return chancesnow;
  if (iconText == "chancetstorms") return chancestorms;
  if (iconText == "clear") return clear;
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
// Helper function, should be part of the weather station library and should disappear soon
const String getShortText(String iconText) {

  if (iconText == "chanceflurries") return "Chance of Flurries";
  if (iconText == "chancerain") return "Chance of Rain";
  if (iconText == "chancesleet") return "Chance of Sleet";
  if (iconText == "chancesnow") return "Chance of Snow";
  if (iconText == "chancetstorms") return "Chance of Storms";
  if (iconText == "clear") return "Clear";
  if (iconText == "cloudy") return "Cloudy";
  if (iconText == "flurries") return "Flurries";
  if (iconText == "fog") return "Fog";
  if (iconText == "hazy") return "Hazy";
  if (iconText == "mostlycloudy") return "Mostly Cloudy";
  if (iconText == "mostlysunny") return "Mostly Sunny";
  if (iconText == "partlycloudy") return "Partly Couldy";
  if (iconText == "partlysunny") return "Partly Sunny";
  if (iconText == "sleet") return "Sleet";
  if (iconText == "rain") return "Rain";
  if (iconText == "snow") return "Snow";
  if (iconText == "sunny") return "Sunny";
  if (iconText == "tstorms") return "Storms";


  return "-";
}

