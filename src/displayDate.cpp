#include "shared.h"

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