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
#define WIFI_SSID "Your-SSID"
#define WIFI_PASS "YoUrSc3RtPASS"

#define HOST "esp8266-MeteoStation"

const int UPDATE_INTERVAL_SECS = 10 * 60; // Update every 10 minutes
const int SLEEP_INTERVAL_SECS = 0;   // Going to Sleep after idle times, set 0 for dont sleep
const int SCREENSVR_INTERVAL_SECS = 30;   // Off screen after idle times, set 0 for dont off


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
// http://api.wunderground.com/api/027189a1354cd237/conditions/q/SK/Dubnica%20nad%20vahom.json

const String DISPLAYED_CITY_NAME = "Dubnica nad Vahom";
const String WUNDERGRROUND_API_KEY = "000a00003212";
const String WUNDERGRROUND_LANGUAGE = "SK";
const String WUNDERGROUND_COUNTRY = "SK";
const String WUNDERGROUND_CITY = "Dubnica%20nad%20vahom";

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
#define NTP_SERVERS "0.europe.pool.ntp.org", "1.europe.pool.ntp.org", "2.europe.pool.ntp.org"
// #define NTP_SERVERS "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"

// Transliteration map
const String special[] = {"Ľ" , "Š" , "Č" , "Ť" , "Ž" , "Ý" , "Á" , "Í" , "É" , "Ô" ,
 "Ď", "ľ" , "š" , "č" , "ť" , "ž" , "ý" , "á" , "í" , "é" , "ô", "ď" ,
 "а","б","в","г","д","е","ё","ж","з","и","й","к","л","м","н","о","п","р","с",
 "т","у","ф","х","ц","ч","ш","щ","ъ","ы","ь","э","ю","я","А","Б","В","Г","Д",
 "Е","Ё","Ж","З","И","Й","К","Л","М","Н","О","П","Р","С","Т","У","Ф","Х","Ц",
 "Ч","Ш","Щ","Ъ","Ы","Ь","Э","Ю","Я" };
const String latin[] = {"L" , "S" , "C" , "T" , "Z" , "Y" , "A" , "I" , "E" , "O",
 "D",  "l" , "s" , "c" , "t" , "z" , "y" , "a" , "i" , "e" , "o","d" ,
 "a","b","v","g","d","e","io","zh","z","i","y","k","l","m","n","o","p",
 "r","s","t","u","f","h","ts","ch","sh","sht","a","i","y","e","yu","ya",
 "A","B","V","G","D","E","Io","Zh","Z","I","Y","K","L","M","N","O","P",
 "R","S","T","U","F","H","Ts","Ch","Sh","Sht","A","I","Y","e","Yu","Ya"};

/***************************
 * End Settings
 **************************/
