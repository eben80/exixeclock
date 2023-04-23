#include "exixe.h"
#include "WiFiClient.h"
#include <WiFiUdp.h>
#include <Timezone.h>

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