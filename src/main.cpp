
/** NimBLE_Client Demo:
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 */
// #define BLE_ARR_SIZE 3
// #define THERMO_ARR_SIZE 5

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

const char *TAG = "main";
const char *MyGetIndex::TAG = "MyGetIndex";

const bool MyLog::DEBUG = false;
const bool MyLog::DEBUG2 = false;
const bool MyLog::DEBUG3 = true;
const bool MyLog::DEBUG4 = true;
const bool MyLog::INFO = true;
const bool MyLog::WARN = true;
const bool MyLog::ERROR = true;

MyBLE2 myBleArr[BLE_ARR_SIZE];
// int numberOfConnectedBMS = 0;

MyThermo myThermoArr[THERMO_ARR_SIZE];
// int numberOfConnectedThermo = 0;

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

// std::vector<MyBLE2> *bleDevices;

MyScanCallbacks myScanCallbacks(myBleArr, &myM5, myThermoArr);
MyClientCallbacks myClientCallbacks(myBleArr, myThermoArr);
MyNotification myNotification(myBleArr, &myScanCallbacks, &myClientCallbacks, myThermoArr);
MyMqtt myMqtt(&mqttClient, myBleArr, &voltMater, &myM5, myThermoArr, &myWiFi, &myNotification);

// MyGetIndex myGetIndex(myBleArr, &myScanCallbacks.numberOfBMS, myThermoArr, &myScanCallbacks.numberOfThermo);

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
  myNotification.clear(); // may have bugs for the variable
  // numberOfConnectedThermo = 0; //may have bugs for the variable, 0 not working
  myScanCallbacks.advDevices.clear();
  myScanCallbacks.advThermoDevices.clear();

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
  mySdCard.updatePOI(configJson);
  INFO_PRINT("POI download done\n");

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
  /** Loop here until we find a device we want to connect to */
  if (myScanCallbacks.doRescan)
  {
    myScanCallbacks.doRescan = false;
    scanBle();
  }

  if (myScanCallbacks.doConnect)
  {
    myScanCallbacks.doConnect = false;
    INFO_PRINT("Found %d of BMSs we want to connect to, do it now.\n", myScanCallbacks.numberOfBMS);
    myNotification.numberOfBMS = myScanCallbacks.numberOfBMS;
    if (myNotification.connectToServer())
    {
      DEBUG_PRINT("Success! we should now be getting notifications from BMS(%d).\n", myNotification.numberOfConnectedBMS);
      // myMqtt.mqttDeviceSetup(myNotification.numberOfConnectedBMS);
      myMqtt.bmsSetup();
      myM5.numberOfConnectedBMS = myNotification.numberOfConnectedBMS;
      // myClientCallbacks.numberOfConnectedBMS = myNotification.numberOfConnectedBMS;
      /*
      if (!mqttClient.connected())
      {
        myMqtt.reConnectMqttServer();
      }
      myM5.println("publishing Home assistant discovery");
      myMqtt.publishHaDiscovery();
      myM5.powerSave(1);
      */
    }
    else
    {
      WARN_PRINT("Failed to connect all BMS found, goint to reconnect\n");
      myScanCallbacks.doConnect = true;
      /*
      DEBUG_PRINT("Failed to connect BMS, goint to reset\n");
      myM5.println("Failed to connect BMS, goint to reset");
      delay(2000);
      myM5.reset();
      */
    }
  }

  if (myScanCallbacks.doConnectThermo)
  {
    myScanCallbacks.doConnectThermo = false;
    INFO_PRINT("Found %d of thermomaters we want to connect to, do it now.\n", myScanCallbacks.numberOfThermo);
    myNotification.numberOfThermo = myScanCallbacks.numberOfThermo;
    if (myNotification.connectToThermo())
    {
      DEBUG_PRINT("Success! we should now be getting notifications from Thermometers(%d).\n", myNotification.numberOfConnectedThermo);
      // myMqtt.mqttThermoSetup(myNotification.numberOfConnectedThermo);
      myMqtt.thermoSetup();
      myM5.numberOfConnectedThermo = myNotification.numberOfConnectedThermo;
      // myClientCallbacks.numberOfConnectedThermo = myNotification.numberOfConnectedThermo;
    }
    else
    {
      WARN_PRINT("Failed to connect Thermomaters, goint to reconnect\n");
      myScanCallbacks.doConnectThermo = true;
      /*
      DEBUG_PRINT(", goint to reset\n");
      myM5.println("Failed to connect Thermomater, goint to reset");
      delay(2000);
      myM5.reset();
      */
    }
  }

  for (int bleIndex = 0; bleIndex < myNotification.numberOfBMS; bleIndex++)
  {
    try
    {
      if (myBleArr[bleIndex].pChr_tx)
      {
        // bool timeout = false;
        // if (timeout = myTimerArr[bleIndex].timeout(millis()))
        if (myBleArr[bleIndex].timeout(millis() + bleIndex * 3000))
        {
          if (!myBleArr[bleIndex].connected)
          {
            String msg = "try to publish " + MyGetIndex::bleInfo(myBleArr, bleIndex) + " not connected";
            throw std::runtime_error(std::string(msg.c_str()));
          }
          myBleArr[bleIndex].sendInfoCommand();
        }
        // DEBUG_PRINT("timeout: %d, myBleArr[%d].newPacketReceived: %d\n", timeout, bleIndex, myBleArr[bleIndex].newPacketReceived);
        if (myBleArr[bleIndex].newPacketReceived)
        {
          DEBUG_PRINT("myBleArr[%d].newPacketReceived: %d\n", bleIndex, myBleArr[bleIndex].newPacketReceived);
          myBleArr[bleIndex].newPacketReceived = false;
          myM5.updateBmsInfo(bleIndex, myBleArr[bleIndex].packBasicInfo.Volts, myBleArr[bleIndex].packBasicInfo.Amps,
                             myBleArr[bleIndex].packCellInfo.CellDiff,
                             myBleArr[bleIndex].packBasicInfo.Temp1, myBleArr[bleIndex].packBasicInfo.Temp2,
                             myBleArr[bleIndex].packBasicInfo.CapacityRemainPercent);
          myMqtt.publishJson("stat/" + myBleArr[bleIndex].topic + "STATE", myBleArr[bleIndex].getState(), true);
        }
      }
    }
    catch (const std::runtime_error &e)
    {
      ERROR_PRINT("%s\n", e.what());
      continue;
    }

    /*
    if (!myBleArr[bleIndex].connected)
    {
      /*
      WARN_PRINT("[%d}check myBleArr[%d].connected == false, going to skip the next bleIndex\n", failCount, bleIndex);
      if (failCount > 100)
      {
        DEBUG_PRINT("exceeaded the limit\n");
        delay(2000);
        throw std::runtime_error("BMS disconnected");
        // DEBUG_PRINT(", goint to reconnect\n");
        // delay(2000);
        // myScanCallbacks.doConnect = true; // does not work

        // DEBUG_PRINT(", goint to reset\n");
        // delay(2000);
        // myM5.reset();
      }
      failCount++;
      continue;
      throw std::runtime_error("BMS disconnected");
    }
    */
  }

  /*
  for (int index = 0; index < myNotification.numberOfConnectedBMS; index++)
  {
    try
    {
      if (myBleArr[index].timeout(millis() + index * 3000))
      {
        //WARN_PRINT("myMqtt.publishJson(\"stat/\" + myBleArr[%d].topic + \"STATE\", myBleArr[%d].getState(), true)", index, index);
        myMqtt.publishJson("stat/" + myBleArr[index].topic + "STATE", myBleArr[index].getState(), true);
      }
    }
    catch (const std::runtime_error &e)
    {
      ERROR_PRINT("%s\n", e.what());
      continue;
    }
  }
  */

  if (voltMater.available && voltMater.timeout(millis()))
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

  for (int index = 0; index < myScanCallbacks.numberOfThermo; index++)
  {
    try
    {
      if (myThermoArr[index].timeout(millis() + index * 3000))
      {
        if (!myThermoArr[index].connected)
        {
          String msg = "try to publish " + MyGetIndex::thermoInfo(myThermoArr, index) + " not connected";
          throw std::runtime_error(std::string(msg.c_str()));
        }
        myMqtt.publishJson("stat/" + myThermoArr[index].topic + "STATE", myThermoArr[index].getState(), true);
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