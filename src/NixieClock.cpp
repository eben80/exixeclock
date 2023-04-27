/*
  exixe modules:
  https://github.com/dekuNukem/exixe

  library docs:
  https://github.com/dekuNukem/exixe/tree/master/arduino_library


*/
#include <Arduino.h>
#include "shared.h"
// #include <FS.h>
#include <LittleFS.h>
// #include "exixe.h"
// #include <ESP8266WiFi.h>
// #include <WiFiUdp.h>
// #include <TimeLib.h>
// #include <ESP8266HTTPClient.h>
// #include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
// #include <Timezone.h>
//needed for library
// #include <DNSServer.h>
// #include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h> //For sunrise/sunset api
// #include "TelnetStream.h"
#include "antiDote.h"
#include "antiDoteCustom.h"
#include "processTwilight.h"
#include "getSunrise.h"
#include "getLoc.h"
#include "updateWeather.h"
#include "syncTime.h"
#include "sendNTPpacket.h"
#include "http.h"
#include "displayCurrentTime.h"
#include "displayDate.h"

// #define DEBUG 


// Interval definition for loop tasks
#define INTERVAL1 1800000  //  Every 30 minutes Anti cathode poisoning
#define INTERVAL2 86400000 // Every 24 hours NTP time sync
#define INTERVAL3 1000     // Update time display every second
#define INTERVAL4 30000    // Display date
#define INTERVAL5 7200000  // Get Sunrise time
#define INTERVAL6 55000   // Display Temperature


unsigned long time_1 = 0;
unsigned long time_2 = 0;
unsigned long time_3 = 0;
unsigned long time_4 = 0;
unsigned long time_5 = 0;
unsigned long time_6 = 0;

unsigned long entry;


bool showTemp = true;
bool jsonErrorRead = false;
bool jsonErrorWrite = false;



// file io
File GetFile(String fileName)
{
  File textFile;
  if (LittleFS.exists(fileName))
  {
    textFile = LittleFS.open(fileName, "r");
  }
  return textFile;
}




void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  // wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectNx"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectNx");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  //if you get here you have connected to the WiFi
  Serial.println("connected");

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("NixieClockAP");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_LittleFS
      type = "filesystem";
    }

    // NOTE: if updating LittleFS this would be the place to unmount LittleFS using LittleFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("Done!");
    Serial.println("Rebooting..");
    //Clear tubes
    my_tube1.clear();
    my_tube2.clear();
    my_tube3.clear();
    my_tube4.clear();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Updating: % u % % \r", (progress / (total / 100)));
    // Tube progress display
    my_tube1.clear();
    if (((progress / (total / 100)) / 100) > 0)
    {
      my_tube2.show_digit(1, 127, 1);
    }
    else
    {
      my_tube2.clear();
    }
    my_tube3.show_digit((progress / (total / 100)) / 10, 127, 1);
    my_tube4.show_digit((progress / (total / 100)) % 10, 127, 1);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[ % u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  // TelnetStream.begin();
  ArduinoOTA.begin();

  // Read persistent config from JSON on LittleFS
  if (LittleFS.begin())
  {
    Serial.println("Mounted file system");

    // parse json config file
    File jsonFile = GetFile(CONFIG_FILE);
    if (jsonFile)
    {
      Serial.println("JSON file found");
      // Allocate a buffer to store contents of the file.
      size_t size = jsonFile.size();
      std::unique_ptr<char[]> jsonBuf(new char[size]);
      jsonFile.readBytes(jsonBuf.get(), size);

      DynamicJsonDocument jsonBuffer(176);
      auto error = deserializeJson(jsonBuffer, jsonBuf.get());
      // JsonObject& json = jsonBuffer.parseObject(jsonBuf.get());
      if (!error)
      {
        useDynamicBright = jsonBuffer["dynamicbright"];
        showDate = jsonBuffer["showdate"];
        showYear = jsonBuffer["showyear"];
        latitude = jsonBuffer["location"]["latitude"].as<float>();
        longitude = jsonBuffer["location"]["longitude"].as<float>();
        if (useDynamicBright)
        {
          DYNBRIGHTSEL = "checked";
        }
        if (showDate)
        {
          DISPDATESEL = "checked";
        }
        if (showYear)
        {
          DISPYEARSEL = "checked";
        }
        LAT = String(latitude, 6);
        LONG = String(longitude, 6);
        // strcpy(cloudmqtt_pass, json["cloudmqtt_pass"]);
        Serial.println("JSON config load success");
      }
      else
      {
        jsonErrorRead = true;
        Serial.println("Failed to load json config");
      }
      jsonFile.close();
    }
    else
    {
      saveConfiguration();
    }
  }

  // ONLY CALL THIS ONCE
  my_tube1.spi_init();

  my_tube1.clear();
  my_tube2.clear();
  my_tube3.clear();
  my_tube4.clear();

  // Future feature for geo-location based on IP
  // getLoc();

  // NTP time sync initial run
  syncTime(); 
  getSunrise();
  server.on("/", handleRoot);
  server.on("/configForm", configForm);
  server.on("/cmd4", cmd4);
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  ArduinoOTA.handle(); // OTA handler

  if (millis() >= time_1 + INTERVAL1) // Anti cathode poisoning task
  {
    time_1 += INTERVAL1;
    antiDote();
  }

  if (millis() >= time_2 + INTERVAL2) // Regular NTP time sync task
  {
    time_2 += INTERVAL2;
    syncTime();
  }

  if (millis() >= time_3 + INTERVAL3) // Display time
  {
    time_3 += INTERVAL3;
    displayCurrentTime();
  }

  if (millis() >= time_4 + INTERVAL4) // Display date
  {
    time_4 += INTERVAL4;
    if (showDate)
    {
      displayDate();
    }
  }

  if (millis() >= time_5 + INTERVAL5) // Update sunrise time.
  {
    time_5 += INTERVAL5;
    getSunrise();
  }

  if (millis() >= time_6 + INTERVAL6) // Display Temp.
  {
    time_6 += INTERVAL6;
    if (showTemp)
    {
      updateWeather();
    }
  }

  server.handleClient();
}
