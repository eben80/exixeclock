#pragma once
#include "shared.h"

// send an NTP request to the time server at the given address

void sendNTPpacket(IPAddress &address);