
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

See more at http://blog.squix.org
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "CalendarParser.h"

CalendarParser::CalendarParser() {

}

void CalendarParser::updateCalendar(String GScriptId) {
  
      
  const char* host = "script.google.com";
  String url = String("/macros/s/") + GScriptId + "/exec";
  JsonStreamingParser parser;
  parser.setListener(this);

  /*const int httpPort = 80;
  if (!client.connect("api.wunderground.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.wunderground.com\r\n" +
               "Connection: close\r\n\r\n");
  int retryCounter = 0;
  while(!client.available()) {
    delay(1000);
    retryCounter++;
    if (retryCounter > 10) {
      return;
    }
  }*/
   // Use HTTPSRedirect class to create a new TLS connection
  int httpsPort = 443;
  HTTPSRedirect *client = new HTTPSRedirect();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(url);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){
    yield();
    Serial.print("X");
    int retval = client->connect("blog.squix.org", httpsPort);
    Serial.print("x");
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
     
  }

  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }
  
  /*if (client->verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }*/


  // fetch spreadsheet data
  client->GET(url, host);

 


  int pos = 0;
  boolean isBody = false;
  char c;

  int size = 0;
  client->setNoDelay(false);
  while(client->connected()) {
    while((size = client->available()) > 0) {
      c = client->read();
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
}

void CalendarParser::whitespace(char c) {
  Serial.println("whitespace");
}

void CalendarParser::startDocument() {
  Serial.println("start document");
}

void CalendarParser::key(String key) {
  currentKey = String(key);

}

void CalendarParser::value(String value) {

}

void CalendarParser::endArray() {

}


void CalendarParser::startObject() {
}

void CalendarParser::endObject() {

}

void CalendarParser::endDocument() {

}

void CalendarParser::startArray() {

}


