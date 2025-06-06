#include <WiFiMulti.h>
#include "MyLog.cpp"
#include "MyLcd2.hpp"

extern const char *TAG;
extern MyLcd2 myLcd;
extern WiFiMulti wifiMulti;

const uint32_t connectTimeoutMs = 20000;

void wifiScann()
{
  int n = WiFi.scanNetworks();
  DEBUG_PRINT("scan done, %d networks found\n", n);
  if (n == 0)
  {
    DEBUG_PRINT("no networks found\n");
  }
  else
  {
    DEBUG_PRINT("networks found\n");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      String enc = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
      DEBUG_PRINT("%d: %s (%d) %s\n", i + 1, WiFi.SSID(i).c_str(),
                  WiFi.RSSI(i), enc.c_str());
      delay(10);
    }
  }
}

int wifiConnect()
{
  DEBUG_PRINT("Connecting Wifi...\n");
  myLcd.println("Connecting Wifi..."); // Serial port format output string.
  // if the connection to the stongest hotstop is lost, it will connect to the next network on the list
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED)
  {
    DEBUG_PRINT("WiFi connected to %s(%d)\n", WiFi.SSID().c_str(), WiFi.RSSI());
    DEBUG_PRINT("IP address: %s\n", WiFi.localIP().toString().c_str());
    //Serial.println(WiFi.localIP());
    return 0;
  }
  else
  {
    DEBUG_PRINT("WiFi not connected!\n");
    return -1;
  }
}

void setupDateTime()
{
  // setup this after wifi connected
  // you can use custom timeZone,server and timeout
  // DateTime.setTimeZone("CST-8");
  DateTime.setTimeZone("JST-9");
  DateTime.setServer("ntp.nict.jp");
  // DateTime.begin(15 * 1000);
  // from
  /** changed from 0.2.x **/
  DateTime.begin(15 * 1000 /* timeout param */);
  if (DateTime.isTimeValid())
  {
    DEBUG_PRINT("DateTime setup done\n");
  }
  else
    DEBUG_PRINT("Failed to get time from mqttServer.\n");
}