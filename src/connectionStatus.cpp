#include "shared.h"

void wifiConnected()
{
    
    for (int i = 0; i < 255; i++) {
    my_tube1.set_led(0, i, 0);
    my_tube1.set_led(0, i, 0);
    my_tube1.set_led(0, i, 0);
    }

    for (int i = 255; i > 1; i--) {
    my_tube1.set_led(0, i, 0);
    my_tube1.set_led(0, i, 0);
    my_tube1.set_led(0, i, 0);
    }
}

void wifiFailed()
{

}

void wifiAP()
{
    
}