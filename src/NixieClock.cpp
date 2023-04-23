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
#include "processTwilight.h"
#include "getSunrise.h"
#include "getLoc.h"
#include "updateWeather.h"
#include "syncTime.h"
#include "sendNTPpacket.h"

// #define DEBUG 

ESP8266WebServer server(80);
String DYNBRIGHTSEL = "";
String DISPDATESEL = "";
String DISPYEARSEL = "";
String LAT = "";
String LONG = "";
int checkminute = 0;
int slotdelay = 30;


const String HTTP1_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>Nixie Clock Configuration</title> ";
const String HTTP1_STYLE = "<style>@import url(https://fonts.googleapis.com/css?family=Lato:300,400,700,900);@import url(https://fonts.googleapis.com/css?family=Raleway:400,300,700,900);H1{text-align: center; font-family:Raleway;line-height: 1.2;}.c{text-align: center;}div,input{padding: 5px; font-size: 1em;}input{width: 90%;}body{text-align: center; font-family: Lato; background-color: #F0FFFF;}button{border: 0; border-radius: 0.6rem; background-color: #1fb3ec; color: #fdd; line-height: 2.4rem; font-size: 1.2rem; width: 90%;}.q{float: right; width: 64px; text-align: right;}.button2{background-color: #008CBA;}.button3{background-color: #f44336;}.button4{background-color: #e7e7e7; color: black;}.button5{background-color: #555555;}.button6{background-color: #4CAF50;}.switch{position: relative; display: inline-block; width: 60px; height: 34px;}.switch input{opacity: 0; width: 0; height: 0;}.slider{position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s;}.slider:before{position: absolute; content: \"\"; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s;}input:checked + .slider{background-color: #2196F3;}input:focus + .slider{box-shadow: 0 0 1px #2196F3;}input:checked + .slider:before{-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px);}/* Rounded sliders */.slider.round{border-radius: 34px;}.slider.round:before{border-radius: 50%;}#opacity-slider{-webkit-appearance: none; height: 4px;}#opacity-slider::-webkit-slider-thumb{-webkit-appearance: none; background-color: #eee; height: 20px; width: 10px; opacity: .7; border-radius: 25px;}</style>";
const String HTTP1_SCRIPT = "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script><script>$(document).ready(function(){$(\'#button2\').change(function(){$(\'#configForm\').submit();}); $(\'#cmd4\').change(function(){$(\'#cmd4Form\').submit();});});</script>";
const String HTTP2_SCRIPT = "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script><script>$(document).ready(function(){var modal=$('<div id=\"modal\" style=\"display:none; position:fixed; top:50%; left:50%; transform:translate(-50%, -50%); background-color:white; padding:20px; font-family:Raleway; border-radius:10px; border:3px solid black;\"></div>');modal.append(\"<p><h3>&#10004; Saved</h3></p>\"),$(\"body\").append(modal);$(\'#button2\').change(function(){$(\'#configForm\').submit();}); $(\'#cmd4\').change(function(){$(\'#cmd4Form\').submit();});modal.show();setTimeout(function() {modal.hide();window.location.replace('/');}, 2000);});</script>";
const String HTTP1_HEAD_END = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;line-height:2.7;'>";
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
const String BRIGHTSLIDER = "<input type=\"range\" id=\"bright-slider\" name=\"brightslide\" min=\"0\" max=\"127\" step=\"1\" value=\"";
const String HOMEPAGEEND = "\"><button id=\"button2\"> Save </button></form><br/> <form id=\"cmd4Form\" action=\"/cmd4\" method=\"get\"><button id=\"cmd4\" class=\"button3\">Antidote Sequence</button><br/> </form> </div></body></html>";


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
int secondTubeBright = 90;
bool darkTheme = false;

bool showDate = true;
bool showYear = false;
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
