#include "shared.h"
#include "exixe.h"
#include <Timezone.h>

unsigned char count;

// Cable Select GPIO for Tubes
int cs1 = 15;
int cs2 = 16;
int cs3 = 5;
int cs4 = 4;

// Declare tubes
exixe my_tube1 = exixe(cs1);
exixe my_tube2 = exixe(cs2);
exixe my_tube3 = exixe(cs3);
exixe my_tube4 = exixe(cs4);

// Wifi
WiFiClient client;
WiFiUDP udp;

String twilightTime;

// Regeneration digits
int firstDigit[] = {};
int secondDigit[] = {0, 9, 8};
int thirdDigit[] = {};
int fourthDigit[] = {};

bool useDynamicBright = true; // Use web-sourced twilight times or static times.
float latitude = 49.052243;  // Configure your latitude here
float longitude = 21.281473; // Configure your longitude here
String JsonStatus = "";
String CTBegin = "";
String CTEnds = "";
long brightTime = 0;
long darkTime = 0;
String lightStart = "6:00:00"; // When to start normal brightness
String darkStart = "18:00:00"; // When to start reduced brightness
int brightness = 90;

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