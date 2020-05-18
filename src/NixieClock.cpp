/*
  exixe modules:
  https://github.com/dekuNukem/exixe

  library docs:
  https://github.com/dekuNukem/exixe/tree/master/arduino_library


*/
#include <Arduino.h>
#include "exixe.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Timezone.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h> //For sunrise/sunset api

// Section for configuring your time zones
// Central European Time (Frankfurt, Paris) - Configure yours here.
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   // Central European Standard Time
Timezone ChozenZone(CEST, CET);

// NTP Server Section
unsigned int localPort = 2390; // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char *ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

WiFiClient client;

// Interval definition for loop tasks
#define INTERVAL1 1800000  //  Every 30 minutes Anti cathode poisoning
#define INTERVAL2 86400000 // Every 24 hours NTP time sync
#define INTERVAL3 1000     // Update time display every second
#define INTERVAL4 30000    // Display date

unsigned long time_1 = 0;
unsigned long time_2 = 0;
unsigned long time_3 = 0;
unsigned long time_4 = 0;

// change those to the cs pins you're using
int cs1 = 15;
int cs2 = 16;
int cs3 = 5;
int cs4 = 4;

unsigned char count;
unsigned char digone;
unsigned char digtwo;
unsigned char digthree;
unsigned char digfour;
bool tickDot = true;
int brightness = 90;
int secondTubeBright = 90;
float latitude = 49.052243;  // Configure your latitude here
float longitude = 21.281473; // Configure your longitude here
String JsonStatus = "";
String CTBegin = "";
String CTEnds = "";
long brightTime = 0;
long darkTime = 0;
bool darkTheme = false;
bool dynamicBright = true; // Use web-sourced twilight times or static times.

String lightStart = "6:00:00"; // When to start normal brightness
String darkStart = "18:00:00"; // When to start reduced brightness

// Declare tubes
exixe my_tube1 = exixe(cs1);
exixe my_tube2 = exixe(cs2);
exixe my_tube3 = exixe(cs3);
exixe my_tube4 = exixe(cs4);

// send an NTP request to the time server at the given address
// unsigned long sendNTPpacket(IPAddress &address)
void sendNTPpacket(IPAddress &address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

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

long processTwilight(String twilightTime, bool isPM) //Takes time from twilight API JSON and returns UTC epoch
{
  int firstColon = twilightTime.indexOf(":");
  int secondColon = twilightTime.indexOf(":", firstColon + 1);
  int CTS = twilightTime.substring(secondColon + 1, secondColon + 3).toInt();
  int CTM = twilightTime.substring(firstColon + 1, firstColon + 3).toInt();
  int CTH = twilightTime.substring(0, firstColon).toInt();
  if (isPM)
  {
    CTH = CTH + 12;
  }

  struct tm t;
  time_t t_of_day;

  t.tm_year = year() - 1900; // Year - 1900
  t.tm_mon = month() - 1;    // Month, where 0 = jan
  t.tm_mday = day();         // Day of the month
  t.tm_hour = CTH;
  t.tm_min = CTM;
  t.tm_sec = CTS;
  t.tm_isdst = 0; // Is DST on? 1 = yes, 0 = no, -1 = unknown
  t_of_day = mktime(&t);

  // Serial.printf("seconds since the Epoch: %ld\n", (long)t_of_day);
  // Serial.println(now());

  return t_of_day;
}

void getSunrise() // Get sunrise/sunset from location
{
  if (dynamicBright)
  {
    if (WL_CONNECTED)
    {
      HTTPClient http;
      http.useHTTP10(true);
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

// Anti cathode poisoning loop
void antiDote()
{
  while (count < 10)
  {
    count++; // keep count between 0 to 9
    my_tube1.show_digit(count, 127, 0);
    my_tube2.show_digit(count, 127, 0);
    my_tube3.show_digit(count, 127, 0);
    my_tube4.show_digit(count, 127, 0);
    my_tube1.set_led(127, 0, 127); // purple;
    my_tube2.set_led(127, 127, 0); // yellow;
    my_tube3.set_led(127, 0, 0);   // yellow;
    my_tube4.set_led(0, 0, 127);   // yellow;
    delay(100);
  }
  count = 0;

  while (count < 10)
  {
    count++; // keep count between 0 to 9
    my_tube1.show_digit(count, 127, 0);
    my_tube2.show_digit(count, 127, 0);
    my_tube3.show_digit(count, 127, 0);
    my_tube4.show_digit(count, 127, 0);
    my_tube1.set_led(127, 0, 127); // purple;
    my_tube2.set_led(127, 127, 0); // yellow;
    my_tube3.set_led(127, 0, 0);   // yellow;
    my_tube4.set_led(0, 0, 127);   // yellow;
    delay(100);
  }
  count = 0;

  int delay1 = 50;
  my_tube1.set_dots(127, 0);
  delay(delay1);
  my_tube1.set_dots(0, 127);
  delay(delay1);
  my_tube1.clear();
  my_tube2.set_dots(127, 0);
  delay(delay1);
  my_tube2.set_dots(0, 127);
  delay(delay1);
  my_tube2.clear();
  my_tube3.set_dots(127, 0);
  delay(delay1);
  my_tube3.set_dots(0, 127);
  delay(delay1);
  my_tube3.clear();
  my_tube4.set_dots(127, 0);
  delay(delay1);
  my_tube4.set_dots(0, 127);
  delay(delay1);
  my_tube4.clear();
  my_tube4.set_dots(0, 127);
  delay(delay1);
  my_tube4.set_dots(127, 0);
  delay(delay1);
  my_tube4.clear();
  my_tube3.set_dots(0, 127);
  delay(delay1);
  my_tube3.set_dots(127, 0);
  delay(delay1);
  my_tube3.clear();
  my_tube2.set_dots(0, 127);
  delay(delay1);
  my_tube2.set_dots(127, 0);
  delay(delay1);
  my_tube2.clear();
  my_tube1.set_dots(0, 127);
  delay(delay1);
  my_tube1.set_dots(127, 0);
  delay(delay1);
  my_tube1.clear();
  delay(750);
}

void displayDate()
{

    // Reset dots
  my_tube1.set_dots(0, 0);
  my_tube2.set_dots(0, 0);
  my_tube3.set_dots(0, 0);
  my_tube4.set_dots(0, 0);

  // Configure tube LED colours here
  my_tube1.set_led(255, 0, 0); // purple;
  my_tube2.set_led(255, 0, 0); // yellow;
  my_tube3.set_led(0, 0, 255); // red
  my_tube4.set_led(0, 0, 255); // blue

  // Set month
  my_tube1.set_dots(brightness, 0);
  if (month() < 10)
  {
    my_tube1.show_digit(0, brightness, 0);
    my_tube2.show_digit(month(), brightness, 0);
  }
  else
  {
    my_tube1.show_digit(1, brightness, 0);
    my_tube2.show_digit(month() - 10, brightness, 0);
  }
  my_tube2.set_dots(0, brightness);

  // Set day
  my_tube3.set_dots(brightness, 0);
  if (day() > 29)
  {
    my_tube3.show_digit(3, brightness, 0);
    my_tube4.show_digit(day() - 30, brightness, 0);
  }
  else if (day() > 19)
  {
    my_tube3.show_digit(2, brightness, 0);
    my_tube4.show_digit(day() - 20, brightness, 0);
  }
  else if (day() > 9)
  {
    my_tube3.show_digit(1, brightness, 0);
    my_tube4.show_digit(day() - 10, brightness, 0);
  }
  else
  {
    my_tube3.show_digit(0, brightness, 0);
    my_tube4.show_digit(day(), brightness, 0);
  }
  my_tube4.set_dots(0, brightness);
  delay(2500);
  // Reset dots
  my_tube1.set_dots(0, 0);
  my_tube2.set_dots(0, 0);
  my_tube3.set_dots(0, 0);
  my_tube4.set_dots(0, 0);

    // Configure tube LED colours here
  my_tube1.set_led(255, 255, 255); // purple;
  my_tube2.set_led(255, 255, 255); // yellow;
  my_tube3.set_led(255, 255, 255); // red
  my_tube4.set_led(255, 255, 255); // blue
  // Set year
  int ones = (year() % 10);
  int tens = ((year() / 10) % 10);
  int hundreds = ((year() / 100) % 10);
  int thousands = (year() / 1000);

  my_tube1.set_dots(brightness, 0);

  my_tube1.show_digit(thousands, brightness, 0);
  my_tube2.show_digit(hundreds, brightness, 0);
  my_tube3.show_digit(tens, brightness, 0);
  my_tube4.show_digit(ones, brightness, 0);
  my_tube4.set_dots(0, brightness);
  delay(2500);
  // Reset dots
  my_tube1.set_dots(0, 0);
  my_tube2.set_dots(0, 0);
  my_tube3.set_dots(0, 0);
  my_tube4.set_dots(0, 0);
}

// Function displaying time
void displayCurrentTime()
{
  if (hour() == 0 && minute() == 0 && second() == 0)
  { //Get fresh twilight times just after midnight
    getSunrise();
  }

  if (hour() > 19 && hour() < 24)
  {
    digone = 2;
  }
  else if (hour() < 20 && hour() > 9)
  {
    digone = 1;
  }
  else
  {
    digone = 0;
  }

  if (hour() > 19 && hour() < 24)
  {
    digtwo = hour() - 20;
  }
  else if (hour() < 20 && hour() > 9)
  {

    digtwo = hour() - 10;
  }
  else if (hour() < 10 && hour() >= 0)
  {

    digtwo = hour();
  }

  else
  {

    digtwo = hour();
  }

  if (minute() < 10)
  {

    digthree = 0;
    digfour = minute();
  }
  else if (minute() > 9 && minute() < 20)
  {

    digthree = 1;
    digfour = minute() - 10;
  }
  else if (minute() > 19 && minute() < 30)
  {

    digthree = 2;
    digfour = minute() - 20;
  }
  else if (minute() > 29 && minute() < 40)
  {

    digthree = 3;
    digfour = minute() - 30;
  }
  else if (minute() > 39 && minute() < 50)
  {

    digthree = 4;
    digfour = minute() - 40;
  }
  else
  {

    digthree = 5;
    digfour = minute() - 50;
  }

  // Change brightness according to time of day

  if (ChozenZone.toUTC(now()) >= darkTime || ChozenZone.toUTC(now()) < brightTime)
  {
    darkTheme = true;
    brightness = 30;
    if (tickDot)
    {
      secondTubeBright = 35;
    }
    else
    {
      secondTubeBright = 30;
    }
  }
  else
  {
    darkTheme = false;
    brightness = 100;
  }

  my_tube1.show_digit(digone, brightness, 0);
  my_tube2.show_digit(digtwo, secondTubeBright, 0);
  // Tick dot for seconds
  if (tickDot)
  {
    my_tube2.set_dots(0, 20);
    tickDot = false;
  }
  else
  {
    my_tube2.set_dots(0, 0);
    tickDot = true;
  }
  my_tube3.show_digit(digthree, brightness, 0);
  my_tube4.show_digit(digfour, brightness, 0);
  Serial.println(darkTheme);
}

// Function for regular NTP time sync
void syncTime()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Starting UDP");
    udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(udp.localPort());

    //get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP);

    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);

    int cb = udp.parsePacket();
    if (!cb)
    {
      Serial.println("no packet yet");
    }
    else
    {
      Serial.print("packet received, length=");
      Serial.println(cb);
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      // Serial.print("Seconds since Jan 1 1900 = ");
      // Serial.println(secsSince1900);

      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      // print Unix time:
      Serial.println(epoch);
      time_t utc = epoch;
      time_t localt;
      localt = ChozenZone.toLocal(utc);
      setTime(localt);
    }
  }
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
  ArduinoOTA.setHostname("NixieClock");

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
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("Done!");
    Serial.println("Rebooting..");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Updating: % u % % \r", (progress / (total / 100)));
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

  ArduinoOTA.begin();

  // ONLY CALL THIS ONCE
  my_tube1.spi_init();

  my_tube1.clear();
  my_tube2.clear();
  my_tube3.clear();
  my_tube4.clear();

  // Future feature for geo-location based on IP
  // getLoc();

  // NTP time sync initial run
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb)
  {
    Serial.println("no packet yet");
  }
  else
  {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
    time_t utc = epoch;
    time_t localt;
    localt = ChozenZone.toLocal(utc);
    setTime(localt);

    // print the hour, minute and second:
    Serial.print("The UTC time is ");      // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10)
    {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10)
    {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  getSunrise();
}

void loop()
{
  ArduinoOTA.handle(); // OTA handler
                       // Anti cathode poisoning task
  if (millis() >= time_1 + INTERVAL1)
  {
    time_1 += INTERVAL1;
    antiDote();
  }
  // Regular NTP time sync task
  if (millis() >= time_2 + INTERVAL2)
  {
    time_2 += INTERVAL2;
    syncTime();
  }

  if (millis() >= time_3 + INTERVAL3)
  {
    time_3 += INTERVAL3;
    displayCurrentTime();
  }

  if (millis() >= time_4 + INTERVAL4)
  {
    time_4 += INTERVAL4;
    displayDate();
  }

  // Configure tube LED colours here
  my_tube1.set_led(0, 0, 0); // purple;
  my_tube2.set_led(0, 0, 0); // yellow;
  my_tube3.set_led(0, 0, 0); // red
  my_tube4.set_led(0, 0, 0); // blue
}
