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

#include <simpleDSTadjust.h>

// Setup
#define WIFI_SSID "yourssid"
#define WIFI_PASS "yourpassw0rd"

const int UPDATE_INTERVAL_SECS = 15 * 60; // Update every 10 minutes
const int SLEEP_INTERVAL_SECS = 0;   // Going to Sleep after idle times, set 0 for dont sleep


// Pins for the ILI9341
#define TFT_DC D2
#define TFT_CS D1
#define TFT_LED D8

#define HAVE_TOUCHPAD
#define TOUCH_CS D3
#define TOUCH_IRQ  D4
    


// Wunderground Settings
// To check your settings first try them out in your browser:
// http://api.wunderground.com/api/WUNDERGROUND_API_KEY/conditions/q/WUNDERGROUND_COUNTTRY/WUNDERGROUND_CITY.json
// e.g. http://api.wunderground.com/api/808ba87ed77c4511/conditions/q/CH/Zurich.json
// e.g. http://api.wunderground.com/api/808ba87ed77c4511/conditions/q/CA/SAN_FRANCISCO.json <- note that in the US you use the state instead of country code

const String DISPLAYED_CITY_NAME = "Zurich";
const String WUNDERGRROUND_API_KEY = "<YOUR_WUNDERGROUND_API_KEY>";
const String WUNDERGRROUND_LANGUAGE = "EN";
const String WUNDERGROUND_COUNTRY = "CH";
const String WUNDERGROUND_CITY = "Zurich";

#define UTC_OFFSET + 1
struct dstRule StartRule = {"CEST", Last, Sun, Mar, 2, 3600}; // Central European Summer Time = UTC/GMT +2 hours
struct dstRule EndRule = {"CET", Last, Sun, Oct, 2, 0};       // Central European Time = UTC/GMT +1 hour

// Settings for Boston
// #define UTC_OFFSET -5
// struct dstRule StartRule = {"EDT", Second, Sun, Mar, 2, 3600}; // Eastern Daylight time = UTC/GMT -4 hours
// struct dstRule EndRule = {"EST", First, Sun, Nov, 1, 0};       // Eastern Standard time = UTC/GMT -5 hour

// values in metric or imperial system?
bool IS_METRIC = true;

// Change for 12 Hour/ 24 hour style clock
bool IS_STYLE_12HR = false;

// change for different ntp (time servers)
#define NTP_SERVERS "0.ch.pool.ntp.org", "1.ch.pool.ntp.org", "2.ch.pool.ntp.org"
// #define NTP_SERVERS "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"

/***************************
 * End Settings
 **************************/

