
/** NimBLE_Client Demo:
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <StreamUtils.h>
#include <NimBLEDevice.h>
#include <M5Core2.h>

#include "MyBLE2.hpp"
#include "MyThermo.hpp"
// #include "MyScanCallbacks.hpp"
#include "MyNotification.hpp"
#include "MyClientCallBacks.hpp"
#include "MyLog.hpp"
// #include "MyTimer.cpp"
#include "MySdCard.hpp"
#include "MyWiFi.hpp"
#include "MyMqtt.hpp"
#include "VoltMater.hpp"
#include "MyM5.hpp"
#include "MyGetIndex.hpp"

#define CONFIG_FILE "/config.json"
#define FAIL_LIMIT_MAIN 4
#define PUBLISH_LEAD_TIME_BMS 30000
#define PUBLISH_LEAD_TIME_THERMO 10000

const char *TAG = "main";
const char *MyGetIndex::TAG = "MyGetIndex";

const bool MyLog::DEBUG = false;  // White
const bool MyLog::DEBUG2 = false; // Cyan
const bool MyLog::DEBUG3 = true;  // Magenda
const bool MyLog::DEBUG4 = true;  // Blue
const bool MyLog::INFO = true;    // Green
const bool MyLog::WARN = true;    // Yellow
const bool MyLog::ERROR = true;   // Red

int failCount = 0;

// MyBLE2 myBleArr[BLE_ARR_SIZE];
//  int numberOfConnectedBMS = 0;

// MyThermo myThermoArr[THERMO_ARR_SIZE];
//  int numberOfConnectedThermo = 0;

MyM5 myM5;
MySdCard mySdCard(&myM5);
JsonDocument configJson;
// JsonArray deviceList;
// WiFiMulti wifiMulti;
MyWiFi myWiFi(&myM5);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
// PowerSaving2 powerSaving(&myLcd);

// MyTimer myTimer(0, 10000);
// MyTimer myTimerArr[3];
// MyTimer myTimerArr2[3];
// int timeoutCount = 0;
VoltMater voltMater;

MyScanCallbacks myScanCallbacks(&myM5);
MyClientCallbacks myClientCallbacks(&myScanCallbacks, &myM5);
MyNotification myNotification(&myScanCallbacks, &myClientCallbacks, &myM5);
MyMqtt myMqtt(&mqttClient, &voltMater, &myM5, &myWiFi, &myNotification, &myScanCallbacks);

// void loadConfig();
// void saveConfig();
/*
void wifiScann();
int wifiConnect();
void setupDateTime();

void detectButton(int numberOfPages);
void printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE2 myBle);
*/
void scanBle()
{
  // myNotification.clear(); // may have bugs for the variable
  // numberOfConnectedThermo = 0; //may have bugs for the variable, 0 not working
  // myScanCallbacks.advDevices.clear();
  // myScanCallbacks.advThermoDevices.clear();

  /** Initialize NimBLE and set the device name */
  NimBLEDevice::init("NimBLE-Client");

  /**
   * Set the IO capabilities of the device, each option will trigger a different pairing method.
   *  BLE_HS_IO_KEYBOARD_ONLY   - Passkey pairing
   *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
   *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
   */
  // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY); // use passkey
  // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

  /**
   * 2 different ways to set security - both calls achieve the same result.
   *  no bonding, no man in the middle protection, BLE secure connections.
   *  These are the default values, only shown here for demonstration.
   */
  // NimBLEDevice::setSecurityAuth(false, false, true);

  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

  /** Optional: set the transmit power */
  NimBLEDevice::setPower(3); /** 3dbm */
  NimBLEScan *pScan = NimBLEDevice::getScan();

  /** Set the callbacks to call when scan events occur, no duplicates */
  pScan->setScanCallbacks(&myScanCallbacks, false);
  // pScan->setScanCallbacks(&myScanCallbacks, false);

  /** Set scan interval (how often) and window (how long) in milliseconds */
  pScan->setInterval(100);
  pScan->setWindow(100);

  /**
   * Active scan will gather scan response data from advertisers
   *  but will use more energy from both devices
   */
  pScan->setActiveScan(true);

  /** Start scanning for advertisers */
  pScan->start(myScanCallbacks.scanTimeMs);
}

void setup()
{
  M5.begin(); // Init M5Core2.
  Serial.begin(9600);
  // MySdCard::deleteFile(SD, "/log.txt");
  INFO_PRINT("Starting setup\n");
  INFO_PRINT("loading config file...\n");
  mySdCard.setup();
  configJson = mySdCard.loadConfig(CONFIG_FILE);
  INFO_PRINT("loading done\n");

  INFO_PRINT("Setting up WiFi...\n");
  myWiFi.setup(configJson);
  INFO_PRINT("WiFi setup done\n");

  // setup DateTime
  INFO_PRINT("Setting up date...\n");
  myM5.println("Setting up date");
  myWiFi.setupDateTime();
  myM5.lastReset = DateTime.getTime(); //
  myM5.println("date setup done");

  // setup MQTT
  INFO_PRINT("Setting up MQTT\n");
  myMqtt.mqttServerSetup(configJson);
  myMqtt.deviceSetup();
  // publishing Homeassistant discovery
  if (!mqttClient.connected())
  {
    myMqtt.reConnectMqttServer();
  }
  myM5.println("publishing HA discovery");
  myMqtt.publishHaDiscovery();
  INFO_PRINT("MQTT setup done\n");

  // download POI
  INFO_PRINT("Going to download POI...\n");
  if (mySdCard.updatePOI(configJson["poiURL"]))
  {
    INFO_PRINT("POI download successfully done\n");
  }
  else
  {
    ERROR_PRINT("1st POI download failed. try 2nd POI\n");
    if (mySdCard.updatePOI(configJson["poiURL2"]))
    {
      INFO_PRINT("POI download successfully done\n");
    }
    else
    {
      ERROR_PRINT("2nd POI download failed\n");
    }
  }

  // setup BLE and start scanning
  INFO_PRINT("Disconnecting WiFi\n");
  myWiFi.disconnect();
  INFO_PRINT("Starting NimBLE Client\n");
  myM5.println("Setting up BLE");
  INFO_PRINT("Scanning for BLE devices\n");
  scanBle();
}

// int failCount = 0;
void loop()
{
  myM5.detectButton();
  mqttClient.loop();
  unsigned long currentTime;
  /** Loop here until we find a device we want to connect to */

  if (myScanCallbacks.doConnect)
  {
    myScanCallbacks.doConnect = false;
    INFO_PRINT("Found %d of BMSs we want to connect to, do it now.\n", myScanCallbacks.bleDevices.size());
    myMqtt.bmsSetup();
    // myNotification.numberOfBMS = myScanCallbacks.bleDevices.size();
    if (myNotification.connectToServer())
    {
      INFO_PRINT("Success! we should now be getting notifications from BMS(%d).\n", myClientCallbacks.numberOfConnectedBMS);
      // myMqtt.mqttDeviceSetup(myNotification.numberOfConnectedBMS);
      // myMqtt.bmsSetup();
      // myM5.numberOfConnectedBMS = myClientCallbacks.numberOfConnectedBMS;
      for (int index = 0; index < myScanCallbacks.bleDevices.size(); index++)
      {
        myScanCallbacks.bleDevices[index].lastMeasurment = millis() + PUBLISH_LEAD_TIME_BMS + 4800 * index;
        INFO_PRINT("myScanCallbacks.bleDevices[%d].lastMeasurment: %lu\n", index, myScanCallbacks.bleDevices[index].lastMeasurment);
      }
    }
    else
    {
      WARN_PRINT("[%d/%d] Failed to connect BMS found[%d] %s\n",
                 ++failCount, FAIL_LIMIT_MAIN, myScanCallbacks.bleDevices.size());
      if (failCount <= FAIL_LIMIT_MAIN)
      {
        WARN_PRINT("rescan\n");
        myM5.println("Failed to connect BMS rescan");
        myNotification.clearResources();

        //myScanCallbacks.bleDevices.clear();
        //myScanCallbacks.thermoDevices.clear();
        //myM5.bmsInfoVec.clear();
        myM5.numberOfScan++;
        NimBLEDevice::getScan()->start(myScanCallbacks.scanTimeMs, false, true);
      }
      else
      {
        WARN_PRINT("Exceeded the fail limit (%d) of main. Continue\n", FAIL_LIMIT_MAIN);
        // myMqtt.bmsSetup();
        // myM5.numberOfConnectedBMS = myClientCallbacks.numberOfConnectedBMS;
        for (int index = 0; index < myScanCallbacks.bleDevices.size(); index++)
        {
          myScanCallbacks.bleDevices[index].lastMeasurment = millis() + PUBLISH_LEAD_TIME_BMS + 4800 * index;
          INFO_PRINT("myScanCallbacks.bleDevices[%d].lastMeasurment: %lu\n", index, myScanCallbacks.bleDevices[index].lastMeasurment);
        }
      }
    }
  }

  if (myScanCallbacks.doConnectThermo)
  {
    myScanCallbacks.doConnectThermo = false;
    INFO_PRINT("Found %d of thermomaters we want to connect to, do it now.\n", myScanCallbacks.advThermoDevices.size());
    myMqtt.thermoSetup();
    // myNotification.numberOfThermo = myScanCallbacks.advThermoDevices.size();
    if (myNotification.connectToThermo())
    {
      INFO_PRINT("Success! we should now be getting notifications from Thermometers(%d).\n", myClientCallbacks.numberOfConnectedThermo);
      // myMqtt.mqttThermoSetup(myNotification.numberOfConnectedThermo);
      // myMqtt.thermoSetup();
      // myM5.numberOfConnectedThermo = myClientCallbacks.numberOfConnectedThermo;
      // myClientCallbacks.numberOfConnectedThermo = myNotification.numberOfConnectedThermo;
      for (int index = 0; index < myScanCallbacks.advThermoDevices.size(); index++)
      {
        myScanCallbacks.thermoDevices[index].lastMeasurment = millis() + PUBLISH_LEAD_TIME_THERMO + 6500 * index;
        INFO_PRINT("myScanCallbacks.thermoDevices[%d].lastMeasurment: %lu\n", index, myScanCallbacks.thermoDevices[index].lastMeasurment);
      }
    }
    else
    {
      WARN_PRINT("[%d/%d] Failed to connect thermomaters found[%d] %s\n",
                 ++failCount, FAIL_LIMIT_MAIN, myScanCallbacks.advThermoDevices.size());
      if (failCount <= FAIL_LIMIT_MAIN)
      {
        WARN_PRINT("rescan\n");
        myM5.println("Failed to connect thermo rescan");
        myNotification.clearResources();

        //myScanCallbacks.bleDevices.clear();
        //myScanCallbacks.thermoDevices.clear();
        myM5.numberOfScan++;
        NimBLEDevice::getScan()->start(myScanCallbacks.scanTimeMs, false, true);
      }
      else
      {
        WARN_PRINT("Exceeded the fail limit (%d) of main. Continue\n", FAIL_LIMIT_MAIN);
        // myMqtt.thermoSetup();
        // myM5.numberOfConnectedThermo = myClientCallbacks.numberOfConnectedThermo;
        for (int index = 0; index < myScanCallbacks.advThermoDevices.size(); index++)
        {
          myScanCallbacks.thermoDevices[index].lastMeasurment = millis() + PUBLISH_LEAD_TIME_THERMO + 6500 * index;
          INFO_PRINT("myScanCallbacks.thermoDevices[%d].lastMeasurment: %lu\n", index, myScanCallbacks.thermoDevices[index].lastMeasurment);
        }
      }
    }
  }

  currentTime = millis();
  for (int bleIndex = 0; bleIndex < myScanCallbacks.bleDevices.size(); bleIndex++)
  {
    try
    {
      if (myScanCallbacks.bleDevices[bleIndex].pChr_tx)
      {
        if (myScanCallbacks.bleDevices[bleIndex].timeout(currentTime))
        {
          if (!myScanCallbacks.bleDevices[bleIndex].connected)
          {
            myMqtt.publish("stat/" + myScanCallbacks.bleDevices[bleIndex].topic + "STATE", "{\"connected\": 0}");
            String msg = MyGetIndex::bleInfo(&myScanCallbacks.bleDevices, bleIndex) + " not connected";
            throw std::runtime_error(std::string(msg.c_str()));
          }
          // myBleArr[bleIndex].sendInfoCommand();
          // INFO_PRINT("[%lu] myBleArr[%d].sendInfoCommand()\n", millis(), bleIndex);

          myScanCallbacks.bleDevices[bleIndex].sendInfoCommand();
          INFO_PRINT("[%lu] myScanCallbacks.bleDevices[%d].sendInfoCommand()\n", millis(), bleIndex);
        }
        // DEBUG_PRINT("timeout: %d, myBleArr[%d].newPacketReceived: %d\n", timeout, bleIndex, myBleArr[bleIndex].newPacketReceived);

        if (myScanCallbacks.bleDevices[bleIndex].newPacketReceived)
        {
          DEBUG_PRINT("myScanCallbacks.bleDevices[%d].newPacketReceived: %d\n", bleIndex, myScanCallbacks.bleDevices[bleIndex].newPacketReceived);
          myScanCallbacks.bleDevices[bleIndex].newPacketReceived = false;

          DEBUG_PRINT("myScanCallbacks.bleDevices[%d].newPacketReceived: %d\n", bleIndex, myScanCallbacks.bleDevices[bleIndex].newPacketReceived);
          myScanCallbacks.bleDevices[bleIndex].newPacketReceived = false;

          myM5.updateBmsInfo(bleIndex, myScanCallbacks.bleDevices[bleIndex].packBasicInfo.Volts, myScanCallbacks.bleDevices[bleIndex].packBasicInfo.Amps,
                             myScanCallbacks.bleDevices[bleIndex].packCellInfo.CellDiff,
                             myScanCallbacks.bleDevices[bleIndex].packBasicInfo.Temp1, myScanCallbacks.bleDevices[bleIndex].packBasicInfo.Temp2,
                             myScanCallbacks.bleDevices[bleIndex].packBasicInfo.CapacityRemainPercent);
          myMqtt.publishJson("stat/" + myScanCallbacks.bleDevices[bleIndex].topic + "STATE", myScanCallbacks.bleDevices[bleIndex].getState(), true);
        }
      }
    }
    catch (const std::runtime_error &e)
    {
      WARN_PRINT("%s\n", e.what());
      continue;
    }
  }

  if (voltMater.connected && voltMater.timeout(millis()))
  {
    myM5.updateVoltMaterInfo(voltMater.calVoltage);
    myMqtt.publishJson("stat/" + voltMater.topic + "STATE", voltMater.getState(), true);
  }

  if (myM5.timeout(millis()))
  {
    myM5.updateLipoInfo();
    myMqtt.publishJson("stat/" + myM5.topic + "STATE", myM5.getState(), true);
    myM5.powerSave(1);
  }

  currentTime = millis();
  for (int index = 0; index < myScanCallbacks.thermoDevices.size(); index++)
  {
    try
    {
      if (myScanCallbacks.thermoDevices[index].timeout(currentTime))
      {
        if (!myScanCallbacks.thermoDevices[index].connected)
        {
          myMqtt.publish("stat/" + myScanCallbacks.thermoDevices[index].topic + "STATE", "{\"connected\": 0}");
          String msg = MyGetIndex::thermoInfo(&myScanCallbacks.thermoDevices, index) + " not connected";
          throw std::runtime_error(std::string(msg.c_str()));
        }
        JsonDocument thermoInfoJson = myScanCallbacks.thermoDevices[index].getState();
        myM5.updateThermoInfo(index, thermoInfoJson["temperature"], thermoInfoJson["humidity"]);
        myMqtt.publishJson("stat/" + myScanCallbacks.thermoDevices[index].topic + "STATE", thermoInfoJson, true);
      }
    }
    catch (const std::runtime_error &e)
    {
      WARN_PRINT("%s\n", e.what());
      continue;
    }
  }

  if (myM5.resetTimeout(DateTime.getTime()))
  {
    DEBUG_PRINT("reset once everyday\n");
    myM5.println("reset once everyday");
    delay(2000);
    // myM5.shutdown(1);
    myM5.reset();
  }
  //
}