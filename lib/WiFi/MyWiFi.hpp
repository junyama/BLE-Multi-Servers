#ifndef MY_WIFI_HPP
#define MY_WIFI_HPP

#include <WiFiMulti.h>
#include <ArduinoJson.h>

#include "MyLog.cpp"
#include "MyLcd2.hpp"

class MyWiFi
{
private:
  const char *TAG = "MyWiFi";
  MyLcd2 myLcd;
  WiFiMulti wifiMulti;
  const uint32_t connectTimeoutMs = 20000;

public:
  void setup(JsonDocument configJson);
  void wifiScann();
  int wifiConnect();
  void setupDateTime();
};

#endif /* MY_WIFI_HPP */