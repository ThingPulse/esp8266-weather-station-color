# esp8266-weather-station-color

ESP8266 Weather Station in Color using ILI9341 TFT 240x320 display

## Hardware Requirements

This code is made for an 240x320 65K ILI9341 display with code running on an ESP8266.
You can buy such a display here: 

[http://www.banggood.com/2_2-Inch-Serial-TFT-SPI-Screen-p-912854.html](http://www.banggood.com/2_2-Inch-Serial-TFT-SPI-LCD-Screen-Module-HD-240-x-320-5110-Compatible-p-912854.html?p=6R31122484684201508S)

## Software Requirements/ Libraries

* Arduino IDE with ESP8266 platform installed
* [Weather Station Library](https://github.com/squix78/esp8266-weather-station) or through Library Manager
* [Adafruit ILI9341](https://github.com/adafruit/Adafruit_ILI9341) or through Library Manager
* [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) or through Library Manager
* [WifiManager](https://github.com/tzapu/WiFiManager)

You also need to get an API key for the Wunderground data: https://www.wunderground.com/

## Wiring

| ILI9341       | NodeMCU      |
| ------------- |:-------------:| 
| MISO          | -             | 
| LED           | 3V3           | 
| SCK           | D5            | 
| MOSI          | D7            |
| DC/RS         | D4            |
| RESET         | RST           |
| CS            | D2            |
| GND           | GND           |
| VCC           | 3V3           |

