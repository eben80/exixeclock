
#include "getLoc.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "shared.h"

// Future feature, get location & Time zone from IP
void getLoc()
{
  if (WL_CONNECTED)
  {
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://ip-api.com/csv"))
    { // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
          String payload = http.getString();
          Serial.println(payload);
        }
      }
      else
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    }
    http.end();
  }
  else
  {
    Serial.println("Not Connected");
  }
}