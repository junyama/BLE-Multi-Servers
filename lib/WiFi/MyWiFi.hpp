#ifndef MY_WIFI_HPP
#define MY_WIFI_HPP

#include <WiFiMulti.h>
#include <ArduinoJson.h>

#include "MyLog.cpp"
#include "MyM5.hpp"

class MyWiFi
{
private:
  const char *TAG = "MyWiFi";
  MyM5 *myM5;
  WiFiMulti wifiMulti;
  const uint32_t connectTimeoutMs = 20000;

public:
  MyWiFi(MyM5 *myM5_);
  void setup(JsonDocument configJson);
  void wifiScann();
  int wifiConnect();
  void setupDateTime();
};

#endif /* MY_WIFI_HPP */