#include "exixe.h"
#include "WiFiClient.h"

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