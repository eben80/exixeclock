
#include "antiDote.h"
#include "shared.h"



// Anti cathode poisoning loop
void antiDote()
{
  while (count < 10)
  {
    count++; // keep count between 0 to 9
    my_tube1.show_digit(count, 127, 1);
    my_tube2.show_digit(count, 127, 1);
    my_tube3.show_digit(count, 127, 1);
    my_tube4.show_digit(count, 127, 1);
    my_tube1.set_led(127, 0, 127); // purple;
    my_tube2.set_led(127, 127, 0); // yellow;
    my_tube3.set_led(127, 0, 0);   // yellow;
    my_tube4.set_led(0, 0, 127);   // yellow;
    delay(100);
  }
  count = 0;

  while (count < 10)
  {
    count++; // keep count between 0 to 9
    my_tube1.show_digit(count, 127, 1);
    my_tube2.show_digit(count, 127, 1);
    my_tube3.show_digit(count, 127, 1);
    my_tube4.show_digit(count, 127, 1);
    my_tube1.set_led(127, 0, 127); // purple;
    my_tube2.set_led(127, 127, 0); // yellow;
    my_tube3.set_led(127, 0, 0);   // yellow;
    my_tube4.set_led(0, 0, 127);   // yellow;
    delay(100);
  }
  count = 0;

  int delay1 = 50;
  my_tube1.set_dots(127, 0);
  delay(delay1);
  my_tube1.set_dots(0, 127);
  delay(delay1);
  my_tube1.clear();
  my_tube2.set_dots(127, 0);
  delay(delay1);
  my_tube2.set_dots(0, 127);
  delay(delay1);
  my_tube2.clear();
  my_tube3.set_dots(127, 0);
  delay(delay1);
  my_tube3.set_dots(0, 127);
  delay(delay1);
  my_tube3.clear();
  my_tube4.set_dots(127, 0);
  delay(delay1);
  my_tube4.set_dots(0, 127);
  delay(delay1);
  my_tube4.clear();
  my_tube4.set_dots(0, 127);
  delay(delay1);
  my_tube4.set_dots(127, 0);
  delay(delay1);
  my_tube4.clear();
  my_tube3.set_dots(0, 127);
  delay(delay1);
  my_tube3.set_dots(127, 0);
  delay(delay1);
  my_tube3.clear();
  my_tube2.set_dots(0, 127);
  delay(delay1);
  my_tube2.set_dots(127, 0);
  delay(delay1);
  my_tube2.clear();
  my_tube1.set_dots(0, 127);
  delay(delay1);
  my_tube1.set_dots(127, 0);
  delay(delay1);
  my_tube1.clear();
  delay(750);
}