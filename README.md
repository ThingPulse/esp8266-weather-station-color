# esp8266-weather-station-color

ESP8266 Weather Station in Color using ILI9341 TFT 240x320 display

## Hardware Requirements

1. SQUIX

This code is made for an 240x320 65K ILI9341 display with code running on an ESP8266.
Since it was sometimes complicated to find the right parts I created a kit which contains all the parts including the connector PCB:
https://blog.squix.org/product/esp8266-wifi-color-display-kit-2-4
By buying the kit from me you are supporting future development. Thank you!

[![Squix ESP8266 Color Display Kit](resources/ESP8266ColorDisplayKit.png)](https://blog.squix.org/product/esp8266-wifi-color-display-kit-2-4)

2. AZSMZ TFT

cxandy created a nice adaptation of this code to run with the hardware from AZSMZ:
https://github.com/cxandy/esp8266-weather-station-color



## Software Requirements/ Libraries
 * Mini Grafx by Daniel Eichhorn
 * ESP8266 WeatherStation by Daniel Eichhorn
 * Json Streaming Parser by Daniel Eichhorn
 * simpleDSTadjust by neptune2

You also need to get an API key for the Wunderground data: https://www.wunderground.com/

## Settings
Please have a good look at the settings.h file. There you can:
 * set your location for the weather information
 * Set the clock mode: 12hour (am/pm) or 24hour mode
 * Metric system for temperature
 * Time zone and daytime saving options
 * API key for the wunderground service

## Wiring

![Wiring](resources/PlaneSpotterWiring.png)

| Wemos D1 Mini | ILI9341      |
| ------------- |:-------------:|
| D4            | T_IRQ         |
| D6            | T_DO          |
| D7            | T_DIN         |
| D3            | T_CS          |
| D5            | T_CLK         |
| D6            | SDO           |
| D8            | LED           |
| D5            | SCK           |
| D7            | SDI           |
| D2            | D/C           |
| RST           | RESET         |
| D1            | CS            |
| GND           | GND           |
| 3V3           | VCC           |


## OTA Updating
There is OTA Web based updater built in, see settings.h
  #define HOST "esp8266-MeteoStation"
  #define UPDATE_PATH "/update"
  #define UPDATE_USERNAME "admin"
  #define UPDATE_PASSWORD "admin"

open browser and head to esp8266-MeteoStation/update or IP/update after entering correct username/password combination you will be able to load new firmware and update it remotely.
Keep in mind that overall sketch should be less than 50% of device memory.

## Handling multi language texts
There are language files containing strings which are used in UI.
There are still some strings hardcoded to the weather station library.
For now there is original English and Slovak file.
Just include the correct file at the top of code.
  #include <language_EN.h>

As I am not able to reproduce steps for converting custom font files for extended language sets as Latin or Cyrillic, I used function to transliterate these characters which font cannot show to show basic version of there characters.

You can adjust your characters by editing array in function
  String Translit(String str) {
      String lat_up[] = {"Ľ" , "Š" , "Č" , "Ť" , "Ž" , "Ý" , "Á" , "Í" , "É" , "Ô" , "Ď"  };
      String lat_low[] = {"ľ" , "š" , "č" , "ť" , "ž" , "ý" , "á" , "í" , "é" , "ô", "ď"};
      String trans_up[] = {"L" , "S" , "C" , "T" , "Z" , "Y" , "A" , "I" , "E" , "O", "D"};
      String trans_low[] = { "l" , "s" , "c" , "t" , "z" , "y" , "a" , "i" , "e" , "o","d"};

      for (int i = 0; i < 11 ; i++) {
        str.replace(lat_up[i],trans_up[i]);
        str.replace(lat_low[i],trans_low[i]);
      }

      return str;
  }

After editing the array just use the correct number of elements in array in for() function.
