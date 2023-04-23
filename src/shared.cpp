#include "shared.h"
#include "exixe.h"

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