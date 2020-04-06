/*
  exixe modules:
  https://github.com/dekuNukem/exixe

  library docs:
  https://github.com/dekuNukem/exixe/tree/master/arduino_library

  Demo 4: Loop digits on two tubes
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

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   // Central European Standard Time
Timezone CE(CEST, CET);

char ssid[] = "Belkin";      //  your network SSID (name)
char pass[] = "Fixui27d69!"; // your network password

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

// #define INTERVAL_MESSAGE1 3600000
#define INTERVAL_MESSAGE1 600000
#define INTERVAL_MESSAGE2 86400000
// #define INTERVAL_MESSAGE2 15000
// #define INTERVAL_MESSAGE3 11000
// #define INTERVAL_MESSAGE4 13000

unsigned long time_1 = 0;
unsigned long time_2 = 0;
// unsigned long time_3 = 0;
// unsigned long time_4 = 0;

// change those to the cs pins you're using
int cs1 = 15;
int cs2 = 16;
int cs3 = 5;
int cs4 = 4;
unsigned char offset = 2;
unsigned char count;
unsigned char digone;
unsigned char digtwo;
unsigned char digthree;
unsigned char digfour;

exixe my_tube1 = exixe(cs1);
exixe my_tube2 = exixe(cs2);
exixe my_tube3 = exixe(cs3);
exixe my_tube4 = exixe(cs4);

bool tickDot = true;

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
  }
  else
  {
    Serial.println("Not Connected");
  }
}

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

void displayCurrentTime()
{

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
    Serial.println("Case 1");
    digtwo = hour() - 20;
  }
  else if (hour() < 20 && hour() > 9)
  {
    Serial.println("Case 2");
    digtwo = hour() - 10;
  }
  else if (hour() < 10 && hour() >= 0)
  {
    Serial.println("Case 3");
    digtwo = hour();
  }

  else
  {
    Serial.println("Case 4");
    digtwo = hour();
  }

  if (minute() < 10)
  {
    Serial.println("Case A");
    digthree = 0;
    digfour = minute();
  }
  else if (minute() > 9 && minute() < 20)
  {
    Serial.println("Case B");
    digthree = 1;
    digfour = minute() - 10;
  }
  else if (minute() > 19 && minute() < 30)
  {
    Serial.println("Case C");
    digthree = 2;
    digfour = minute() - 20;
  }
  else if (minute() > 29 && minute() < 40)
  {
    Serial.println("Case D");
    digthree = 3;
    digfour = minute() - 30;
  }
  else if (minute() > 39 && minute() < 50)
  {
    Serial.println("Case E");
    digthree = 4;
    digfour = minute() - 40;
  }
  else
  {
    Serial.println("Case F");
    digthree = 5;
    digfour = minute() - 50;
  }

  Serial.print(digone);
  Serial.print(digtwo);
  Serial.print(digthree);
  Serial.print(digfour);
  Serial.println();

  int brightness = 90;
  // Serial.println(hour());
  if (hour() >= 18 || hour() < 6)
  {
    brightness = 30;
  }
  else
  {
    brightness = 100;
  }
  // Serial.println(brightness);

  my_tube1.show_digit(digone, brightness, 0);
  my_tube2.show_digit(digtwo, brightness, 0);
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

  delay(1000);
}

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
      localt = CE.toLocal(utc);
      setTime(localt);
    }
  }
}
void setup()
{

  // Serial.begin(74880);
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
  //here  "AutoConnectAP"
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

  // getLoc();

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
    localt = CE.toLocal(utc);
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
  // wait ten seconds before asking for the time again
  // delay(10000);
}

void loop()
{
  ArduinoOTA.handle();

  if (millis() >= time_1 + INTERVAL_MESSAGE1)
  {
    time_1 += INTERVAL_MESSAGE1;
    antiDote();
  }

  if (millis() >= time_2 + INTERVAL_MESSAGE2)
  {
    time_2 += INTERVAL_MESSAGE2;
    syncTime();
  }

  // if (millis() >= time_3 + INTERVAL_MESSAGE3)
  // {
  //   time_3 += INTERVAL_MESSAGE3;
  //   // print_time(time_3);
  //   // Serial.println("My name is Message the third.");
  // }

  // if (millis() >= time_4 + INTERVAL_MESSAGE4)
  // {
  //   time_4 += INTERVAL_MESSAGE4;
  //   // print_time(time_4);
  //   // Serial.println("Message four is in the house!");
  // }

  my_tube1.set_led(0, 0, 0); // purple;
  my_tube2.set_led(0, 0, 0); // yellow;
  my_tube3.set_led(0, 0, 0); // red
  my_tube4.set_led(0, 0, 0); // blue

  displayCurrentTime();
}
