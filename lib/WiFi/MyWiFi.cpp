#include <MyWiFi.hpp>

MyWiFi::MyWiFi(MyM5 *myM5_) : myM5(myM5_)
{
}

void MyWiFi::setup(JsonDocument configJson)
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname("JunBMS");
  INFO_PRINT("WiFi MAC Address: %s\n", WiFi.macAddress().c_str());

  // Add list of wifi networks
  for (int i = 0; i < configJson["wifi"].size(); i++)
  {
    wifiMulti.addAP(configJson["wifi"][i]["ssid"], configJson["wifi"][i]["pass"]);
  }

  DEBUG_PRINT("going to scann WiFi\n");
  scan();

  DEBUG_PRINT("going to connect WiFi\n");
  myM5->println("Going to connect WiFi");
  if (connect() != 0)
  {
    DEBUG_PRINT("failed to connect WiFi and exiting\n");
    exit(-1);
  }
  DEBUG_PRINT("WiFi setup done\n");
  myM5->println("WiFi connected");
}

void MyWiFi::scan()
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

int MyWiFi::connect()
{
  DEBUG_PRINT("Connecting Wifi...\n");
  myM5->println("Connecting Wifi..."); // Serial port format output string.
  // if the connection to the stongest hotstop is lost, it will connect to the next network on the list
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED)
  {
    DEBUG_PRINT("WiFi connected to %s(%d)\n", WiFi.SSID().c_str(), WiFi.RSSI());
    DEBUG_PRINT("IP address: %s\n", WiFi.localIP().toString().c_str());
    // Serial.println(WiFi.localIP());
    return 0;
  }
  else
  {
    DEBUG_PRINT("WiFi not connected!\n");
    return -1;
  }
}

void MyWiFi::disconnect()
{
  DEBUG_PRINT("disconnecting WiFi\n");
  WiFi.disconnect();
}

bool MyWiFi::isConnected()
{
  return WiFi.isConnected();
}

void MyWiFi::setupDateTime()
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