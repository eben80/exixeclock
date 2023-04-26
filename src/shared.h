#include "exixe.h"
#include "WiFiClient.h"
#include <WiFiUdp.h>
#include <Timezone.h>
#include <ESP8266WebServer.h>

#define CONFIG_FILE "/settings.json"

extern unsigned char count;
//Cable Select for Tubes
extern int cs1;
extern int cs2;
extern int cs3;
extern int cs4;
// Declare tubes
extern exixe my_tube1;
extern exixe my_tube2;
extern exixe my_tube3;
extern exixe my_tube4;

extern WiFiClient client;
extern ESP8266WebServer server;
extern String twilightTime;

// Regeneration digits
extern int firstDigit[];
extern int secondDigit[];
extern int thirdDigit[];
extern int fourthDigit[];

extern bool useDynamicBright;
extern float latitude;  // Configure your latitude here
extern float longitude; // Configure your longitude here
extern String JsonStatus;
extern String CTBegin;
extern String CTEnds;
extern long brightTime;
extern long darkTime;
extern String lightStart; // When to start normal brightness
extern String darkStart; // When to start reduced brightness;
extern int brightness;

extern String DYNBRIGHTSEL;
extern String DISPDATESEL;
extern String DISPYEARSEL;
extern String LAT;
extern String LONG;
extern bool showDate;
extern bool showYear;

extern unsigned char digone;
extern unsigned char digtwo;
extern unsigned char digthree;
extern unsigned char digfour;
extern bool tickDot;
extern int secondTubeBright;
extern bool darkTheme;
extern int checkminute;
extern int slotdelay;

// A UDP instance to let us send and receive packets over UDP
extern WiFiUDP udp;

// Section for configuring your time zones
// Central European Time (Frankfurt, Paris) - Configure yours here.
extern TimeChangeRule CEST; // Central European Summer Time
extern TimeChangeRule CET;   // Central European Standard Time
extern Timezone ChozenZone;
// NTP Server Section
extern unsigned int localPort; // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
extern IPAddress timeServerIP; // time.nist.gov NTP server address
extern const char *ntpServerName;

extern const int NTP_PACKET_SIZE; // NTP time stamp is in the first 48 bytes of the message

extern byte packetBuffer[]; //buffer to hold incoming and outgoing packets