#include <Arduino.h>
#include "shared.h"
#include "getSunrise.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "processTwilight.h"

void getSunrise() // Get sunrise/sunset from location
{
  if (useDynamicBright)
  {
    if (WL_CONNECTED)
    {
      HTTPClient http;
      http.useHTTP10(true);
      Serial.print("Getting dusk and dawn values...\n");
      Serial.print("[HTTP] begin...\n");

      String urlBuf;
      urlBuf += F("http://api.sunrise-sunset.org/json?lat=");
      urlBuf += String(latitude, 6);
      urlBuf += F("&lng=");
      urlBuf += String(longitude, 6);
      urlBuf += F("&date=today");

      if (http.begin(client, urlBuf))
      { // HTTP

        Serial.print("[HTTP] GET " + urlBuf + "\n");
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

            // Parse response
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, http.getStream());
            JsonStatus = doc["status"].as<String>();
            if (JsonStatus == "OK")
            {
              CTBegin = doc["results"]["civil_twilight_begin"].as<String>();
              CTEnds = doc["results"]["civil_twilight_end"].as<String>();
              Serial.println("Twilight begins: " + CTBegin);
              Serial.println("Twilight ends: " + CTEnds);

              brightTime = processTwilight(CTBegin, false);
              darkTime = processTwilight(CTEnds, true);
            }
          }
        }
        else
        {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          Serial.println("HTTP Error Using static brightness times.");
          brightTime = processTwilight(lightStart, false);
          darkTime = processTwilight(darkStart, true);
        }
      }
      http.end();
    }
    else
    {
      Serial.println("Not Connected Using static brightness times.");
      brightTime = processTwilight(lightStart, false);
      darkTime = processTwilight(darkStart, true);
    }
  }
  else
  {
    Serial.println("Using static brightness times.");
    brightTime = processTwilight(lightStart, false);
    darkTime = processTwilight(darkStart, true);
  }
}