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
See more at http://blog.squix.ch
*/

#include <ESP8266WebServer.h>
#include <MiniGrafx.h>

const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input, select{padding:5px;font-size:1em;} input, select{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='post' action='save'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<label for='{i}'>{p}</label><br/><input id='{i}' name='{n}' maxlength={l}  value='{v}' {c}><br/><br/>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form><br/><form action=\"/reset\" method=\"get\"><button>Restart ESP</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_UPDATE_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/update\">Firmware upgrade</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";
const char HTTP_OPTION_ITEM[] PROGMEM     = "<option value=\"{v}\" {s}>{n}</option>";
const char HTTP_WG_LANGUAGES[] PROGMEM    = "See <a href='https://www.wunderground.com/weather/api/d/docs?d=language-support' target=_blank>Language Codes</a> for explanation.";

const char country_0[] PROGMEM = "AF";
const char country_1[] PROGMEM = "AL";
const char country_2[] PROGMEM = "AR";
const char country_3[] PROGMEM = "AZ";
const char country_4[] PROGMEM = "BR";
const char country_5[] PROGMEM = "BU";
const char country_6[] PROGMEM = "BY";
const char country_7[] PROGMEM = "CA";
const char country_8[] PROGMEM = "CH";
const char country_9[] PROGMEM = "CN";
const char country_10[] PROGMEM = "CR";
const char country_11[] PROGMEM = "CY";
const char country_12[] PROGMEM = "CZ";
const char country_13[] PROGMEM = "DK";
const char country_14[] PROGMEM = "DL";
const char country_15[] PROGMEM = "DV";
const char country_16[] PROGMEM = "EN";
const char country_17[] PROGMEM = "EO";
const char country_18[] PROGMEM = "ET";
const char country_19[] PROGMEM = "EU";
const char country_20[] PROGMEM = "FA";
const char country_21[] PROGMEM = "FC";
const char country_22[] PROGMEM = "FI";
const char country_23[] PROGMEM = "FR";
const char country_24[] PROGMEM = "GM";
const char country_25[] PROGMEM = "GN";
const char country_26[] PROGMEM = "GR";
const char country_27[] PROGMEM = "GU";
const char country_28[] PROGMEM = "GZ";
const char country_29[] PROGMEM = "HI";
const char country_30[] PROGMEM = "HT";
const char country_31[] PROGMEM = "HU";
const char country_32[] PROGMEM = "HY";
const char country_33[] PROGMEM = "ID";
const char country_34[] PROGMEM = "IL";
const char country_35[] PROGMEM = "IO";
const char country_36[] PROGMEM = "IR";
const char country_37[] PROGMEM = "IS";
const char country_38[] PROGMEM = "IT";
const char country_39[] PROGMEM = "JI";
const char country_40[] PROGMEM = "JP";
const char country_41[] PROGMEM = "JW";
const char country_42[] PROGMEM = "KA";
const char country_43[] PROGMEM = "KM";
const char country_44[] PROGMEM = "KR";
const char country_45[] PROGMEM = "KU";
const char country_46[] PROGMEM = "LA";
const char country_47[] PROGMEM = "LI";
const char country_48[] PROGMEM = "LT";
const char country_49[] PROGMEM = "LV";
const char country_50[] PROGMEM = "MI";
const char country_51[] PROGMEM = "MK";
const char country_52[] PROGMEM = "MN";
const char country_53[] PROGMEM = "MR";
const char country_54[] PROGMEM = "MT";
const char country_55[] PROGMEM = "MY";
const char country_56[] PROGMEM = "ND";
const char country_57[] PROGMEM = "NL";
const char country_58[] PROGMEM = "NO";
const char country_59[] PROGMEM = "OC";
const char country_60[] PROGMEM = "PA";
const char country_61[] PROGMEM = "PL";
const char country_62[] PROGMEM = "PS";
const char country_63[] PROGMEM = "RO";
const char country_64[] PROGMEM = "RU";
const char country_65[] PROGMEM = "SI";
const char country_66[] PROGMEM = "SK";
const char country_67[] PROGMEM = "SL";
const char country_68[] PROGMEM = "SN";
const char country_69[] PROGMEM = "SP";
const char country_70[] PROGMEM = "SR";
const char country_71[] PROGMEM = "SW";
const char country_72[] PROGMEM = "TH";
const char country_73[] PROGMEM = "TK";
const char country_74[] PROGMEM = "TL";
const char country_75[] PROGMEM = "TR";
const char country_76[] PROGMEM = "TT";
const char country_77[] PROGMEM = "TW";
const char country_78[] PROGMEM = "UA";
const char country_79[] PROGMEM = "UZ";
const char country_80[] PROGMEM = "VU";
const char country_81[] PROGMEM = "YI";

const char* const country_table[] PROGMEM = {country_0, country_1, country_2, country_3, country_4, country_5, country_6, country_7, country_8, country_9, country_10, country_11, country_12, country_13, country_14, country_15, country_16, country_17, country_18, country_19, country_20, country_21, country_22, country_23, country_24, country_25, country_26, country_27, country_28, country_29, country_30, country_31, country_32, country_33, country_34, country_35, country_36, country_37, country_38, country_39, country_40, country_41, country_42, country_43, country_44, country_45, country_46, country_47, country_48, country_49, country_50, country_51, country_52, country_53, country_54, country_55, country_56, country_57, country_58, country_59, country_60, country_61, country_62, country_63, country_64, country_65, country_66, country_67, country_68, country_69, country_70, country_71, country_72, country_73, country_74, country_75, country_76, country_77, country_78, country_79, country_80, country_81};
ESP8266WebServer server (80);

String getFormField(String id, String placeholder, String length, String value, String customHTML) {
    String pitem = FPSTR(HTTP_FORM_PARAM);

    pitem.replace("{i}", id);
    pitem.replace("{n}", id);
    pitem.replace("{p}", placeholder);
    pitem.replace("{l}", length);
    pitem.replace("{v}", value);
    pitem.replace("{c}", customHTML);
  return pitem;
}

#define configFileName "azsmz-tft.conf"

boolean saveConfig() {
  File f = SPIFFS.open(configFileName, "w+");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  f.print("WIFI_SSID=");
  f.println(WIFI_SSID);
  f.print("WIFI_PASS=");
  f.println(WIFI_PASS);
  f.print("WUNDERGROUND_CITY=");
  f.println(WUNDERGROUND_CITY);
  f.print("WUNDERGROUND_COUNTRY=");
  f.println(WUNDERGROUND_COUNTRY);
  f.print("WUNDERGRROUND_LANGUAGE=");
  f.println(WUNDERGRROUND_LANGUAGE);
  f.print("WUNDERGRROUND_API_KEY=");
  f.println(WUNDERGRROUND_API_KEY);

  f.print("TIME_SAVER=");
  f.println(SAVER_INTERVAL_SECS / 60);
  f.print("TIME_SLEEP=");
  f.println(SLEEP_INTERVAL_SECS / 60);  
  
  f.close();
  Serial.println("Saved values");
  return true;
}

boolean loadConfig() {
  File f = SPIFFS.open(configFileName, "r");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  while(f.available()) {
      //Lets read line by line from the file
      String key = f.readStringUntil('=');
      String value = f.readStringUntil('\r');
      f.read();
      Serial.println(key + " = [" + value + "]");
      Serial.println(key.length());
      if (key == "WIFI_SSID") {
        WIFI_SSID = value;
      }
      if (key == "WIFI_PASS") {
        WIFI_PASS = value;
      }
      if (key == "WUNDERGROUND_CITY") {
        WUNDERGROUND_CITY = value;
      }
      if (key == "WUNDERGROUND_COUNTRY") {
        WUNDERGROUND_COUNTRY = value;
      }
      if (key == "WUNDERGRROUND_LANGUAGE") {
        WUNDERGRROUND_LANGUAGE = value;
      }
      if (key == "WUNDERGRROUND_API_KEY") {
        WUNDERGRROUND_API_KEY = value;
      }      
      if (key == "TIME_SAVER") {
        SAVER_INTERVAL_SECS = value.toInt() * 60;
      }
      if (key == "TIME_SLEEP") {
        SLEEP_INTERVAL_SECS = value.toInt() * 60;
      }
  }

  f.close();
  Serial.println("Loaded config");
  return true;
}

void handleRoot() {
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Options");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  //page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += "<h1>AZSMZ TFT Configuration</h1>";

  //page += FPSTR(HTTP_PORTAL_OPTIONS);
  page += FPSTR(HTTP_FORM_START);
  page += getFormField("ssid", "WiFi SSID", "20", WIFI_SSID, "");
  page += getFormField("password", "WiFi Password", "20", WIFI_PASS, "");
  page += getFormField("wundergroundcity", "Wunderground City", "40", WUNDERGROUND_CITY, "");
  page += getFormField("wundergroundstate", "Wunderground State", "40", WUNDERGROUND_COUNTRY, "");
  page += "<label for=\"wundergroundlanguage\">Wunderground Language</label>";
  page += "<select id=\"wundergroundlanguage\" name=\"wundergroundlanguage\">";
  Serial.println(WUNDERGRROUND_LANGUAGE);
  for (int i = 0; i < 82; i++) {
    String option = FPSTR(HTTP_OPTION_ITEM);
    String country = FPSTR(country_table[i]);
    option.replace("{v}", country);
    option.replace("{n}", country);
    if (country == WUNDERGRROUND_LANGUAGE) {
      option.replace("{s}", "selected");
    } else {
      option.replace("{s}", "");
    }
    page += option;
  }
  page += "</select>";
  page += FPSTR(HTTP_WG_LANGUAGES);
  page += "<br/><br/>";
  page += getFormField("wundergroundkey", "Wunderground API Key", "40", WUNDERGRROUND_API_KEY.substring(0,3) + "***"+ WUNDERGRROUND_API_KEY.substring(WUNDERGRROUND_API_KEY.length()-3), "");

  page += getFormField("timeSaver", "Screen saver (after minutes)", "40", String(SAVER_INTERVAL_SECS/60), "");
  page += getFormField("timeSleep", "Deep sleep (after minutes)", "40", String(SLEEP_INTERVAL_SECS/60), "");
  
  page += FPSTR(HTTP_FORM_END);
  page += FPSTR(HTTP_UPDATE_LINK);
  page += FPSTR(HTTP_END);

  server.sendHeader("Content-Length", String(page.length()));
  server.send(200, "text/html", page);
}

void handleSave() {
  WIFI_SSID = server.arg("ssid");
  WIFI_PASS = server.arg("password");
  WUNDERGROUND_CITY = server.arg("wundergroundcity");
  WUNDERGROUND_COUNTRY = server.arg("wundergroundstate");
  WUNDERGRROUND_LANGUAGE = server.arg("wundergroundlanguage");

  String apiKey = server.arg("wundergroundkey");
  if (apiKey.length()> 10) WUNDERGRROUND_API_KEY = server.arg("wundergroundkey");
  
  SAVER_INTERVAL_SECS = server.arg("timeSaver").toInt() * 60;
  SLEEP_INTERVAL_SECS = server.arg("timeSleep").toInt() * 60;
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PASS);
  Serial.println(WUNDERGROUND_CITY);
  Serial.println(WUNDERGROUND_COUNTRY);
  Serial.println(WUNDERGRROUND_LANGUAGE);
  Serial.println(WUNDERGRROUND_API_KEY);
  Serial.println(SAVER_INTERVAL_SECS);
  Serial.println(SLEEP_INTERVAL_SECS);
  saveConfig();
  handleRoot();
}

void handleNotFound() {
  //digitalWrite ( led, 1 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  //digitalWrite ( led, 0 );
}

void startConfigPortal(MiniGrafx *gfx) {

  server.on ( "/", handleRoot );
  server.on ( "/save", handleSave);
  server.on ( "/reset", []() {
     saveConfig();
     ESP.restart();
  } );
  server.onNotFound ( handleNotFound );
  server.begin();

  boolean connected = WiFi.status() == WL_CONNECTED;

  gfx->fillBuffer(1);
  gfx->setColor(0);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialMT_Plain_16);
  
  if (connected) {
      Serial.println ( "Open browser at http://" + WiFi.localIP() );
      gfx->drawString(296 / 2, 10, "ESPaper Setup Mode\nConnected to: " + WiFi.SSID() + "\nOpen browser at\nhttp://" + WiFi.localIP().toString());     
  } else {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(CONFIG_SSID.c_str());
      IPAddress myIP = WiFi.softAPIP();  
      Serial.println(myIP);
      gfx->drawString(296 / 2, 10, "ESPaper Setup Mode\nConnect WiFi to:\n" + CONFIG_SSID + "\nOpen browser at\nhttp://" + myIP.toString());
  }
  gfx->commit();
  Serial.println ( "HTTP server started" );
  while(true) {
    server.handleClient();
    yield();
  }
}
