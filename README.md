# ThingPulse ESP8266 Weather Station Color


[![ThingPulse logo](https://thingpulse.com/assets/ThingPulse-w300.svg)](https://thingpulse.com)

ESP8266 Weather Station Color using ILI9341 240x320 TFT display

## Hardware Requirements

The code in this project supports an ILI9341 240x320 TFT display with code running on an ESP8266. To get you up and running in no time we created a kit which contains all the necessary parts including a custom PCB to connect display and WiFi chip:
[https://thingpulse.com/product/esp8266-wifi-color-display-kit-2-4/](https://thingpulse.com/product/esp8266-wifi-color-display-kit-2-4/)

By buying the kit from us you are supporting future development. Thank you!

[![ThingPulse ESP8266 Color Display Kit](resources/ESP8266ColorDisplayKit.png)](https://thingpulse.com/product/esp8266-wifi-color-display-kit-2-4/)

## Step-by-step tutorial

A complete step-by-step tutorial/guide is available at [https://docs.thingpulse.com/guides/wifi-color-display-kit/](https://docs.thingpulse.com/guides/wifi-color-display-kit/).

## Wiring

The below wiring diagram is only needed when you do _not_ buy the self-contained kit from ThingPulse but rather assemble the components yourself.

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
