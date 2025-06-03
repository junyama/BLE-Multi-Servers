#include <ArduinoJson.h>
#include "MySdCard.hpp"
#include "MyLcd2.hpp"

#define CONFIG_FILE "config.json"

extern char *TAG;
extern MyLcd2 myLcd;
extern JsonDocument configJson;

void loadConfig()
{
  String fileName = "/";
  fileName += CONFIG_FILE;
  String textStr = "";
  MySdCard::readFile(SD, fileName.c_str(), textStr);
  myLcd.println("loading config file...");
  DEBUG_PRINT("configJsonText: %s", textStr.c_str());
  DeserializationError error = deserializeJson(configJson, textStr.c_str());
  if (error)
  {
    DEBUG_PRINT("Deserialization error.");
    return;
  }
  /*
  deviceList = configJson["devices"].as<JsonArray>();
  
  int deepSleepTimeSec_ = configJson["deepSleepTimeSec"];
  if (deepSleepTimeSec_)
    deepSleepTimeSec = deepSleepTimeSec_;
  int rebootCount_ = configJson["rebootCount"];
  if (rebootCount_)
    rebootCount = rebootCount_;
  int rebootLimit_ = configJson["rebootLimit"];
  if (rebootLimit_)
    rebootLimit = rebootLimit_;
  // numberOfBleDevices = configJson["devices"].size();
  */
}

void saveConfig()
{
  String fileName = "/";
  fileName += CONFIG_FILE;
  String jsonStr;
  serializeJsonPretty(configJson, jsonStr);
  DEBUG_PRINT("writing configuration file: %s", fileName.c_str());
  MySdCard::writeFile(SD, fileName.c_str(), jsonStr.c_str());
}
