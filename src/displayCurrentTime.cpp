#include "shared.h"

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