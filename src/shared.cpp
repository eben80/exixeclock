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