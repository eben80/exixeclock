#include <Arduino.h>
#include "shared.h"
#include "regenerate.h"

void regenerate(int firstDigit[], int secondDigit[], int thirdDigit[], int fourthDigit[])
{
  int firstArraySize = (sizeof(firstDigit) / sizeof(firstDigit[0]));
  int secondArraySize = (sizeof(secondDigit) / sizeof(secondDigit[0]));
  int thirdArraySize = (sizeof(thirdDigit) / sizeof(thirdDigit[0]));
  int fourthArraySize = (sizeof(fourthDigit) / sizeof(fourthDigit[0]));
  Serial.print(" FirstArray: ");
  Serial.println(sizeof(firstDigit));
  Serial.print(" SecondArray: ");
  Serial.println(sizeof(secondDigit));

  count = 0;
  while (count < 12)
  {
    my_tube1.clear();
    my_tube2.clear();
    my_tube3.clear();
    my_tube4.clear();
    count++;
    // Serial.println(sizeof(firstDigit));
    if (firstArraySize > 0 && firstArraySize > count)
    {
      my_tube1.show_digit(firstDigit[count], 127, 1);
    }
    if (secondArraySize > 0 && secondArraySize > count)
    {
      my_tube2.show_digit(secondDigit[count], 127, 1);
    }
    if (thirdArraySize > 0 && thirdArraySize > count)
    {
      my_tube3.show_digit(thirdDigit[count], 127, 1);
    }
    if (fourthArraySize > 0 && fourthArraySize > count)
    {
      my_tube4.show_digit(fourthDigit[count], 127, 1);
    }
    if (firstArraySize > count || secondArraySize > count || thirdArraySize > count || fourthArraySize > count)
    {
      // delay(7200000); //wait 2 hours

      delay(60000);
    }
  }
  count = 0;
}