#pragma once

void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawWifiQuality();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
void drawAstronomy();
void drawCurrentWeatherDetail();
void drawLabelValue(uint8_t line, String label, String value);
void drawForecastTable(uint8_t start);
void drawAbout();
void drawSeparator(uint16_t y);
String getTime(time_t *timestamp);
const char *getMeteoconIconFromProgmem(String iconText);
const char *getMiniMeteoconIconFromProgmem(String iconText);
void drawForecast1(MiniGrafx *display, CarouselState *state, int16_t x, int16_t y);
void drawForecast2(MiniGrafx *display, CarouselState *state, int16_t x, int16_t y);
void drawForecast3(MiniGrafx *display, CarouselState *state, int16_t x, int16_t y);
void loadPropertiesFromSpiffs();