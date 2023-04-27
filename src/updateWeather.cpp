
#include "shared.h"
#include "updateWeather.h"
#include <ArduinoJson.h>

void updateWeather()
{
  const uint16_t port = 80;
  const char *host = "www.mapme.ga"; // ip or dns

  // Use WiFiClient class to create TCP connections
  // WiFiClient client;

  if (client.connect(host, port))
  {
    // Send a HTTP request
    client.println(F("GET /air/generatejson_latest_oled.php HTTP/1.0"));
    client.println(F("Host: www.mapme.ga"));
    client.println(F("Connection: close"));
    if (client.println() == 0)
    {
      Serial.println(F("Failed to send request"));
      return;
    }

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0)
    {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders))
    {
      Serial.println(F("Invalid response"));
      return;
    }
    //Serial.println(client);
    // Allocate the JSON document
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(14) + 190;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    else
    {

      //deserializeJson(doc, json);

      // const char *id = doc["id"];             // "60517"
      // const char *datetime = doc["datetime"]; // "1553085991"
      // const char *AQ = doc["AQ"];             // "0.00"
      // const int PM25 = doc["PM25"];           // "4.30"
      // const int PM10 = doc["PM10"];           // "6.90"
      int temperature = doc["temperature"]; // "11.30"
      // const char *pressure = doc["pressure"]; // "999.86"
      // const char *humidity = doc["humidity"]; // "36.38"
      // const char *location = doc["location"]; // ""
      // const char *Light = doc["Light"];       // "159"
      // Extract values
      // Serial.println(F("Response: "));
      // Serial.println(doc["temperature"].as<float>(), 2);
      // Serial.println(doc["datetime"].as<long>());

      // Disconnect
      client.stop();
      my_tube1.clear();
      my_tube2.clear();
      if (temperature <= 0)
      {
        my_tube3.set_led(0, 0, 254);
        my_tube4.set_led(0, 0, 254);
      }
      else if (temperature > 0 && temperature <= 10)
      {
        my_tube3.set_led(0, 254, 0);
        my_tube4.set_led(0, 254, 0);
      }
      else if (temperature > 10 && temperature < 25)
      {
        my_tube3.set_led(254, 254, 0);
        my_tube4.set_led(254, 254, 0);
      }
      else if (temperature > 25)
      {
        my_tube3.set_led(254, 0, 0);
        my_tube4.set_led(254, 0, 0);
      }
      temperature = abs(temperature);
      my_tube3.show_digit((temperature / 10), brightness, 0);
      my_tube4.show_digit((temperature % 10), brightness, 0);
      delay(2500);
      my_tube3.set_led(0, 0, 0);
      my_tube4.set_led(0, 0, 0);
      my_tube3.clear();
      my_tube4.clear();
    }
  }
  else
  {
    Serial.println("connection failed");
    // Serial.println("wait 5 sec...");
    // delay(5000);
    // return;
  }
}