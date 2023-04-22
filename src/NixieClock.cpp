/*
  exixe modules:
  https://github.com/dekuNukem/exixe

  library docs:
  https://github.com/dekuNukem/exixe/tree/master/arduino_library


*/
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
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
// #include "TelnetStream.h"
#include <ESP8266WebServer.h>
#include "antiDote.h"
#include "antiDoteCustom.h"
#include "shared.h"

// #define DEBUG 

ESP8266WebServer server(80);
String DYNBRIGHTSEL = "";
String DISPDATESEL = "";
String DISPYEARSEL = "";
String LAT = "";
String LONG = "";
int checkminute = 0;
int slotdelay = 30;

// Regeneration digits
int firstDigit[] = {};
int secondDigit[] = {0, 9, 8};
int thirdDigit[] = {};
int fourthDigit[] = {};

const String HTTP1_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>Nixie Clock Configuration</title> ";
const String HTTP1_STYLE = "<style>.c{text-align: center;}div,input{padding: 5px; font-size: 1em;}input{width: 90%;}body{text-align: center; font-family: verdana;}button{border: 0; border-radius: 0.6rem; background-color: #1fb3ec; color: #fdd; line-height: 2.4rem; font-size: 1.2rem; width: 100%;}.q{float: right; width: 64px; text-align: right;}.button2{background-color: #008CBA;}.button3{background-color: #f44336;}.button4{background-color: #e7e7e7; color: black;}.button5{background-color: #555555;}.button6{background-color: #4CAF50;}.switch{position: relative; display: inline-block; width: 60px; height: 34px;}.switch input{opacity: 0; width: 0; height: 0;}.slider{position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s;}.slider:before{position: absolute; content: \"\"; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s;}input:checked + .slider{background-color: #2196F3;}input:focus + .slider{box-shadow: 0 0 1px #2196F3;}input:checked + .slider:before{-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px);}/* Rounded sliders */.slider.round{border-radius: 34px;}.slider.round:before{border-radius: 50%;}#opacity-slider{-webkit-appearance: none; height: 4px;}#opacity-slider::-webkit-slider-thumb{-webkit-appearance: none; background-color: #eee; height: 20px; width: 10px; opacity: .7; border-radius: 25px;}</style>";
const String HTTP1_SCRIPT = "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script><script>$(document).ready(function(){$(\'#button2\').change(function(){$(\'#configForm\').submit();}); $(\'#cmd4\').change(function(){$(\'#cmd4Form\').submit();});});</script>";
const String HTTP2_SCRIPT = "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script><script>$(document).ready(function(){$(\'#button2\').change(function(){$(\'#configForm\').submit();}); $(\'#cmd4\').change(function(){$(\'#cmd4Form\').submit();});window.alert('Saved');window.location.replace('/');});</script>";
const String HTTP1_HEAD_END = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const String HOMEPAGE1 = "<form id=\"configForm\" action=\"/configForm\" method=\"get\">";
// String LATLONG = "Latitude: <label class=\"text\"> <input type=\"text\" id=\"latitude\" name=\"latitude\" pattern=\"-?\\d{1,3}\\.\\d+\"></label>Longitude: <label class=\"text\"> <input type=\"text\" id=\"longitude\" name=\"longitude\" pattern=\"-?\\d{1,3}\\.\\d+\"></label><br/>";
const String LATSTART = "Latitude: <label class=\"text\"> <input type=\"text\" id=\"latitude\" name=\"latitude\" value=\"";
const String LATEND = "\" pattern=\"-?\\d{1,3}\\.\\d+\"></label>";
const String LONGSTART = "Longitude: <label class=\"text\"> <input type=\"text\" id=\"longitude\" name=\"longitude\" value=\"";
const String LONGEND = "\" pattern=\"-?\\d{1,3}\\.\\d+\"></label><br/>";
const String DYNBRIGHTSTART = "Dynamic Brightness: <label class=\"switch\"> <input id=\"cmd1\" type=\"checkbox\" name=\"dynbright\" ";
const String DYNBRIGHTEND = "> <span class=\"slider round\"></span></label><br/>";
const String DISPDATESTART = "Display Date: <label class=\"switch\"> <input id=\"cmd2\" type=\"checkbox\" name=\"dispdate\" ";
const String DISPDATEEND = "> <span class=\"slider round\"></span></label><br/>";
const String DISPYEARSTART = " Display Year: <label class=\"switch\"> <input id=\"cmd3\" type=\"checkbox\" name=\"dispyear\" ";
const String DISPYEAREND = "> <span class=\"slider round\"></span></label><br/>";
const String BRIGHTSLIDER = "<input type=\"range\" id=\"bright-slider\" name=\"brightslide\" min=\"0\" max=\"127\" step=\"1\" value=\"\">";
const String HOMEPAGEEND = "<button id=\"button2\"> Save </button></form><br/> <form id=\"cmd4Form\" action=\"/cmd4\" method=\"get\"><button id=\"cmd4\" class=\"button3\">Antidote Sequence</button><br/> </form> </div></body></html>";

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
#define INTERVAL5 7200000  // Get Sunrise time
#define INTERVAL6 55000   // Display Temperature

#define CONFIG_FILE "/settings.json"

unsigned long time_1 = 0;
unsigned long time_2 = 0;
unsigned long time_3 = 0;
unsigned long time_4 = 0;
unsigned long time_5 = 0;
unsigned long time_6 = 0;

unsigned long entry;

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

bool useDynamicBright = true; // Use web-sourced twilight times or static times.
bool showDate = true;
bool showYear = false;
bool showTemp = true;
bool jsonErrorRead = false;
bool jsonErrorWrite = false;

String lightStart = "6:00:00"; // When to start normal brightness
String darkStart = "18:00:00"; // When to start reduced brightness


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
// send an NTP request to the time server at the given address
// unsigned long sendNTPpacket(IPAddress &address)
void sendNTPpacket(IPAddress &address)
{
  Serial.println("Sending NTP packet...");
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


void regenerate(int firstDigit[], int secondDigit[], int thirdDigit[], int fourthDigit[])
{
  int firstArraySize = (sizeof(firstDigit) / sizeof(firstDigit[0]));
  int secondArraySize = (sizeof(secondDigit) / sizeof(secondDigit[0]));
  int thirdArraySize = (sizeof(thirdDigit) / sizeof(thirdDigit[0]));
  int fourthArraySize = (sizeof(fourthDigit) / sizeof(fourthDigit[0]));
  Serial.print(" FirstArray: ");
  Serial.println(sizeof(firstDigit));
  Serial.print(" SecondArray: ");
  Serial.println(sizeof(secondDigit));

  count = 0;
  while (count < 12)
  {
    my_tube1.clear();
    my_tube2.clear();
    my_tube3.clear();
    my_tube4.clear();
    count++;
    // Serial.println(sizeof(firstDigit));
    if (firstArraySize > 0 && firstArraySize > count)
    {
      my_tube1.show_digit(firstDigit[count], 127, 1);
    }
    if (secondArraySize > 0 && secondArraySize > count)
    {
      my_tube2.show_digit(secondDigit[count], 127, 1);
    }
    if (thirdArraySize > 0 && thirdArraySize > count)
    {
      my_tube3.show_digit(thirdDigit[count], 127, 1);
    }
    if (fourthArraySize > 0 && fourthArraySize > count)
    {
      my_tube4.show_digit(fourthDigit[count], 127, 1);
    }
    if (firstArraySize > count || secondArraySize > count || thirdArraySize > count || fourthArraySize > count)
    {
      // delay(7200000); //wait 2 hours

      delay(60000);
    }
  }
  count = 0;
}



void updateWeather()
{
  const uint16_t port = 80;
  const char *host = "www.mapme.ga"; // ip or dns

  // Use WiFiClient class to create TCP connections
  // WiFiClient client;

  if (client.connect(host, port))
  {
    // Send a HTTP request
    client.println(F("GET /air/generatejson_latest_oled.php HTTP/1.0"));
    client.println(F("Host: www.mapme.ga"));
    client.println(F("Connection: close"));
    if (client.println() == 0)
    {
      Serial.println(F("Failed to send request"));
      return;
    }

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0)
    {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders))
    {
      Serial.println(F("Invalid response"));
      return;
    }
    //Serial.println(client);
    // Allocate the JSON document
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(14) + 190;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    else
    {

      //deserializeJson(doc, json);

      // const char *id = doc["id"];             // "60517"
      // const char *datetime = doc["datetime"]; // "1553085991"
      // const char *AQ = doc["AQ"];             // "0.00"
      // const int PM25 = doc["PM25"];           // "4.30"
      // const int PM10 = doc["PM10"];           // "6.90"
      int temperature = doc["temperature"]; // "11.30"
      // const char *pressure = doc["pressure"]; // "999.86"
      // const char *humidity = doc["humidity"]; // "36.38"
      // const char *location = doc["location"]; // ""
      // const char *Light = doc["Light"];       // "159"
      // Extract values
      // Serial.println(F("Response: "));
      // Serial.println(doc["temperature"].as<float>(), 2);
      // Serial.println(doc["datetime"].as<long>());

      // Disconnect
      client.stop();
      my_tube1.clear();
      my_tube2.clear();
      if (temperature <= 0)
      {
        my_tube3.set_led(0, 0, 254);
        my_tube4.set_led(0, 0, 254);
      }
      else if (temperature > 0 && temperature <= 10)
      {
        my_tube3.set_led(0, 254, 0);
        my_tube4.set_led(0, 254, 0);
      }
      else if (temperature > 10 && temperature < 25)
      {
        my_tube3.set_led(254, 254, 0);
        my_tube4.set_led(254, 254, 0);
      }
      else if (temperature > 25)
      {
        my_tube3.set_led(254, 0, 0);
        my_tube4.set_led(254, 0, 0);
      }
      temperature = abs(temperature);
      my_tube3.show_digit((temperature / 10), brightness, 0);
      my_tube4.show_digit((temperature % 10), brightness, 0);
      delay(2500);
      my_tube3.set_led(0, 0, 0);
      my_tube4.set_led(0, 0, 0);
      my_tube3.clear();
      my_tube4.clear();
    }
  }
  else
  {
    Serial.println("connection failed");
    // Serial.println("wait 5 sec...");
    // delay(5000);
    // return;
  }
}

void displayDate()
{

  // Reset tubes
  my_tube1.clear();
  my_tube2.clear();
  my_tube3.clear();
  my_tube4.clear();

  // Set month
  my_tube3.set_dots(brightness, 0);

  my_tube3.show_digit((month() / 10), brightness, 0);
  my_tube4.show_digit((month() % 10), brightness, 0);

  my_tube4.set_dots(0, brightness);

  // Set day
  my_tube1.set_dots(brightness, 0);
  my_tube1.show_digit((day() / 10), brightness, 0);
  my_tube2.show_digit((day() % 10), brightness, 0);

  my_tube2.set_dots(0, brightness);
  delay(2500);
  // Reset dots
  my_tube1.set_dots(0, 0);
  my_tube2.set_dots(0, 0);
  my_tube3.set_dots(0, 0);
  my_tube4.set_dots(0, 0);

  if (showYear)
  {
    // Configure tube LED colours here
    my_tube1.set_led(0, 0, 0); // purple;
    my_tube2.set_led(0, 0, 0); // yellow;
    my_tube3.set_led(0, 0, 0); // red
    my_tube4.set_led(0, 0, 0); // blue
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
}

// Function displaying time
void displayCurrentTime()
{
  if (useDynamicBright)
  {
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
      secondTubeBright = brightness;
    }
  }
  else
  {
    secondTubeBright = brightness;
  }

  digone = (hour() / 10);
  digtwo = (hour() % 10);
  digthree = (minute() / 10);
  digfour = (minute() % 10);
  if (minute() != checkminute)
  {
    while (count < 10)
    {
      count++; // keep count between 0 to 9
      my_tube1.show_digit(count, brightness, 0);
      delay(slotdelay);
    }
    count = 0;
    my_tube1.show_digit(digone, brightness, 0);

    while (count < 10)
    {
      count++; // keep count between 0 to 9
      my_tube2.show_digit(count, brightness, 0);
      delay(slotdelay);
    }
    count = 0;
    my_tube2.show_digit(digtwo, secondTubeBright, 0);

    while (count < 10)
    {
      count++; // keep count between 0 to 9
      my_tube3.show_digit(count, brightness, 0);
      delay(slotdelay);
    }
    count = 0;
    my_tube3.show_digit(digthree, brightness, 0);

    while (count < 10)
    {
      count++; // keep count between 0 to 9
      my_tube4.show_digit(count, brightness, 0);
      delay(slotdelay);
    }
    count = 0;

    my_tube4.show_digit(digfour, brightness, 0);
    checkminute = minute();
  }
  else
  {
    my_tube1.show_digit(digone, brightness, 0);
    my_tube2.show_digit(digtwo, secondTubeBright, 0);
    my_tube3.show_digit(digthree, brightness, 0);
    my_tube4.show_digit(digfour, brightness, 0);
  }

  // Tick dot for seconds
  if (tickDot)
  {
    my_tube2.set_dots(0, 35);
    tickDot = false;
  }
  else
  {
    my_tube2.set_dots(0, 0);
    tickDot = true;
  }

#ifdef DEBUG
  Serial.println(darkTheme);
  Serial.print("Dark theme active: ");
  Serial.println(darkTheme);
  Serial.println("Sunrise: " + CTBegin);
  Serial.println("Sunset: " + CTEnds);
  Serial.print("DynBright: ");
  Serial.println(useDynamicBright);
  Serial.print("Showdate: ");
  Serial.println(showDate);
  Serial.print("Showyear:  ");
  Serial.println(showYear);
  Serial.print("Latitude:  ");
  Serial.println(latitude, 6);
  Serial.print("Longitude:  ");
  Serial.println(longitude, 6);
  Serial.println("DYNBRIGHT=" + DYNBRIGHTSEL);
  Serial.println("DISPDATE=" + DISPDATESEL);
  Serial.println("DISPYEAR=" + DISPYEARSEL);
  Serial.print("JsonErrorRead:  ");
  Serial.println(jsonErrorRead);
  Serial.print("JsonErrorWrite:  ");
  Serial.println(jsonErrorWrite);
  Serial.println("---------------------------------");
  #endif
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

void saveConfiguration()
{
  Serial.println("Start SaveConfig");
  // Read persistent config from JSON on LittleFS
  if (LittleFS.begin())
  {
    Serial.println("LittleFS Start success");

    LittleFS.remove(CONFIG_FILE);
    File jsonFile = LittleFS.open(CONFIG_FILE, "w");
    if (jsonFile)
    {
      Serial.println("Config file create succeeded");
      DynamicJsonDocument jsonBuffer(176);

      jsonBuffer["dynamicbright"] = useDynamicBright;
      jsonBuffer["showdate"] = showDate;
      jsonBuffer["showyear"] = showYear;
      jsonBuffer["location"]["latitude"] = latitude;
      jsonBuffer["location"]["longitude"] = longitude;
      if (serializeJson(jsonBuffer, jsonFile) == 0)
      {
        Serial.println("Serialization failed");
      }
      else
      {
        Serial.println("Serialization done.");
      }
      jsonFile.close();
    }
    else
    {
      Serial.println("Config file create failed.");
    }
  }
  else
  {
    Serial.println("error starting LittleFS");
  }
}

void handleRoot()
{
  String s = HTTP1_HEAD;
  s += HTTP1_STYLE;
  s += HTTP1_SCRIPT;
  s += HTTP1_HEAD_END;
  s += "<H3>Nixie Clock Configuration</H3>";
  s += HOMEPAGE1;
  s += LATSTART;
  s += LAT;
  s += LATEND;
  s += LONGSTART;
  s += LONG;
  s += LONGEND;
  s += DYNBRIGHTSTART;
  s += DYNBRIGHTSEL;
  s += DYNBRIGHTEND;
  s += DISPDATESTART;
  s += DISPDATESEL;
  s += DISPDATEEND;
  s += DISPYEARSTART;
  s += DISPYEARSEL;
  s += DISPYEAREND;
  s += BRIGHTSLIDER;
  s += HOMEPAGEEND;
  server.send(200, "text/html", s);
  Serial.println("Web interface called");
}

void configForm()
{
  // String message = "";
  LAT = server.arg("latitude");
  latitude = LAT.toFloat();
  LONG = server.arg("longitude");
  longitude = LONG.toFloat();

  if (server.arg("dynbright") == "on")
  {
    DYNBRIGHTSEL = "checked";
    useDynamicBright = true;
  }
  else
  {

    DYNBRIGHTSEL = "";
    useDynamicBright = false;
  }

  if (server.arg("dispdate") == "on")
  {
    DISPDATESEL = "checked";
    showDate = true;
  }
  else
  {

    DISPDATESEL = "";
    showDate = false;
  }

  if (server.arg("dispyear") == "on")
  {
    DISPYEARSEL = "checked";
    showYear = true;
  }
  else
  {

    DISPYEARSEL = "";
    showYear = false;
  }

  Serial.println("Brightslide value: " + server.arg("brightslide"));
  if (!useDynamicBright)
  {
    int setBrightness = server.arg("brightslide").toInt();
    brightness = setBrightness;
    Serial.println("Brightness value: " + String(brightness));
  }

  saveConfiguration();
  String s = HTTP1_HEAD;
  s += HTTP1_STYLE;
  s += HTTP2_SCRIPT;
  s += HTTP1_HEAD_END;
  s += "<H3>Nixie Clock Configuration</H3>";
  s += HOMEPAGE1;
  s += LATSTART;
  s += LAT;
  s += LATEND;
  s += LONGSTART;
  s += LONG;
  s += LONGEND;
  s += DYNBRIGHTSTART;
  s += DYNBRIGHTSEL;
  s += DYNBRIGHTEND;
  s += DISPDATESTART;
  s += DISPDATESEL;
  s += DISPDATEEND;
  s += DISPYEARSTART;
  s += DISPYEARSEL;
  s += DISPYEAREND;
  s += BRIGHTSLIDER;
  s += HOMEPAGEEND;
  server.send(200, "text/html", s);
}

void cmd4()
{
  String s = HTTP1_HEAD;
  s += HTTP1_STYLE;
  s += HTTP1_SCRIPT;
  s += HTTP1_HEAD_END;
  s += "<H3>Nixie Clock Configuration</H3>";
  s += HOMEPAGE1;
  s += LATSTART;
  s += LAT;
  s += LATEND;
  s += LONGSTART;
  s += LONG;
  s += LONGEND;
  s += DYNBRIGHTSTART;
  s += DYNBRIGHTSEL;
  s += DYNBRIGHTEND;
  s += DISPDATESTART;
  s += DISPDATESEL;
  s += DISPDATEEND;
  s += DISPYEARSTART;
  s += DISPYEARSEL;
  s += DISPYEAREND;
  s += BRIGHTSLIDER;
  s += HOMEPAGEEND;
  server.send(200, "text/html", s);
  antiDote();
  Serial.println("Antidote Triggered....");
  // antiDoteCustom();
  // regenerate(firstDigit, secondDigit, thirdDigit, fourthDigit);
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
