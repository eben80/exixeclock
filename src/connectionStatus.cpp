#include "shared.h"

void wifiConnected()
{
    for (int i = 0; i < 2; i++)
    {
        for (int i = 0; i < 126; i++)
        {
            my_tube1.set_led(0, i, 0);
            my_tube2.set_led(0, i, 0);
            my_tube3.set_led(0, i, 0);
            my_tube4.set_led(0, i, 0);
            delay(5);
        }

        for (int i = 126; i > 0; i--)
        {
            my_tube1.set_led(0, i, 0);
            my_tube2.set_led(0, i, 0);
            my_tube3.set_led(0, i, 0);
            my_tube4.set_led(0, i, 0);
            delay(5);
        }
    }
    my_tube1.clear();
    my_tube2.clear();
    my_tube3.clear();
    my_tube4.clear();
}

void wifiFailed()
{
    for (int i = 0; i < 2; i++)
    {
        for (int i = 0; i < 126; i++)
        {
            my_tube1.set_led(i, 0, 0);
            my_tube2.set_led(i, 0, 0);
            my_tube3.set_led(i, 0, 0);
            my_tube4.set_led(i, 0, 0);
            delay(5);
        }

        for (int i = 126; i > 0; i--)
        {
            my_tube1.set_led(i, 0, 0);
            my_tube2.set_led(i, 0, 0);
            my_tube3.set_led(i, 0, 0);
            my_tube4.set_led(i, 0, 0);
            delay(5);
        }
    }
    my_tube1.clear();
    my_tube2.clear();
    my_tube3.clear();
    my_tube4.clear();
}

void wifiAP()
{
    for (int i = 0; i < 2; i++)
    {
        for (int i = 0; i < 126; i++)
        {
            my_tube1.set_led(0, 0, i);
            my_tube2.set_led(0, 0, i);
            my_tube3.set_led(0, 0, i);
            my_tube4.set_led(0, 0, i);
            delay(5);
        }

        for (int i = 126; i > 0; i--)
        {
            my_tube1.set_led(0, 0, i);
            my_tube2.set_led(0, 0, i);
            my_tube3.set_led(0, 0, i);
            my_tube4.set_led(0, 0, i);
            delay(5);
        }
    }
    for (int i = 0; i < 126; i++)
    {
        my_tube1.set_led(0, 0, i);
        my_tube2.set_led(0, 0, i);
        my_tube3.set_led(0, 0, i);
        my_tube4.set_led(0, 0, i);
        delay(5);
    }
}