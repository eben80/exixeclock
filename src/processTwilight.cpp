#include "shared.h"
#include "processTwilight.h"
#include <TimeLib.h>

long processTwilight(String twilightTime, bool isPM) //Takes time from twilight API JSON and returns UTC epoch
{
  int firstColon = twilightTime.indexOf(":");
  int secondColon = twilightTime.indexOf(":", firstColon + 1);
  int CTS = twilightTime.substring(secondColon + 1, secondColon + 3).toInt();
  int CTM = twilightTime.substring(firstColon + 1, firstColon + 3).toInt();
  int CTH = twilightTime.substring(0, firstColon).toInt();
  if (isPM)
  {
    CTH = CTH + 12;
  }

  struct tm t;
  time_t t_of_day;

  t.tm_year = year() - 1900; // Year - 1900
  t.tm_mon = month() - 1;    // Month, where 0 = jan
  t.tm_mday = day();         // Day of the month
  t.tm_hour = CTH;
  t.tm_min = CTM;
  t.tm_sec = CTS;
  t.tm_isdst = 0; // Is DST on? 1 = yes, 0 = no, -1 = unknown
  t_of_day = mktime(&t);

  // Serial.printf("seconds since the Epoch: %ld\n", (long)t_of_day);
  // Serial.println(now());

  return t_of_day;
}