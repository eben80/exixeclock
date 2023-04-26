#include <Arduino.h>
#include "shared.h"
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "antiDote.h"

#define CONFIG_FILE "/settings.json"

// ESP8266WebServer server(80);

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