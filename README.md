# esp8266-weather-station-color

ESP8266 Weather Station in Color using ILI9341 TFT 240x320 display

## Hardware Requirements

This code is made for an 240x320 65K ILI9341 display with code running on an ESP8266.
Since it was sometimes complicated to find the right parts I created a kit which contains all the parts including the connector PCB:
https://blog.squix.org/product/esp8266-wifi-color-display-kit-2-4
By buying the kit from me you are supporting future development. Thank you!

[![Squix ESP8266 Color Display Kit](resources/ESP8266ColorDisplayKit.png)](https://blog.squix.org/product/esp8266-wifi-color-display-kit-2-4)


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
 * Timezone and daytime saving options
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
