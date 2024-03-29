# exixeclock
ESP8266-powered, WiFi enabled, automatic time syncing, OTA updatable 4 digit Nixie Clock based on [Exixe driver modules](https://github.com/dekuNukem/exixe) from @dekuNukem

Click below to view Youtube Video

[![Youtube video](https://img.youtube.com/vi/TgXrI-OZ0sM/0.jpg)](https://www.youtube.com/watch?v=TgXrI-OZ0sM)

Has the following features:
*   WifiManager autoconfiguration
*   Automatic daily NTP online time sync
*   Timezones
*   Dynamic tube brightness according to sunrise sunset at location
*   Date and year display
*   Anti-cathode poisoning routine
*   Customizable RGB LED for each tube
*   Web Interface to configure location, dynmic brightess and display settings.

Planned Feature:
*   Geolocation from IP

Hardware:
*   Wemos D1 Mini Pro
*   IN-14 tubes X 4
*   Exixe IN-14 tube drivers x 4
*   NCH6100HV High Voltage DC Power Supply
*   12V AC adapter
*   Female socket for 12V AC
*   12V to 5V DC DC transformer - disasembled car mobile phone charger should work.
*   Wiring
*   Case of some sort - I 3D printed my own design

Using the [Sunrise Sunset API](https://sunrise-sunset.org/api)

Schematic:

![Exixeclock Schematic](https://github.com/eben80/exixeclock/blob/master/circuit/Schematic_ExixeClock.jpg)


![Tubes from Ukraine](https://github.com/eben80/exixeclock/blob/master/circuit/IMG_20191231_193828.jpg)

![Assembly](https://github.com/eben80/exixeclock/blob/master/circuit/IMG_20200328_155927.jpg)
