#include "shared.h"
#include "regenerate.h"

void regenerate(int firstDigit[], int firstArraySize, 
                 int secondDigit[], int secondArraySize, 
                 int thirdDigit[], int thirdArraySize, 
                 int fourthDigit[], int fourthArraySize) {

  Serial.print(" FirstArray: ");
  Serial.println(firstArraySize); 
  Serial.print(" SecondArray: ");
  Serial.println(secondArraySize);

  count = 0;
  while (count < 12) {
    my_tube1.clear();
    my_tube2.clear();
    my_tube3.clear();
    my_tube4.clear();
    count++;

    if (firstArraySize > 0 && count < firstArraySize) { 
      my_tube1.show_digit(firstDigit[count], 127, 1);
    }
    if (secondArraySize > 0 && count < secondArraySize) {
      my_tube2.show_digit(secondDigit[count], 127, 1);
    }
    if (thirdArraySize > 0 && count < thirdArraySize) {
      my_tube3.show_digit(thirdDigit[count], 127, 1);
    }
    if (fourthArraySize > 0 && count < fourthArraySize) {
      my_tube4.show_digit(fourthDigit[count], 127, 1);
    }
    if (firstArraySize > count || secondArraySize > count || thirdArraySize > count || fourthArraySize > count) {
      delay(60000); // Wait 1 minute (adjust as needed)
    }
  }
  count = 0;
}