
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

// #include "PowerSaving2.hpp"
// #include "MyLcd2.hpp"
//  #include "MyBLE.cpp"
#include "MyBLE2.hpp"
#include "MyThermo.hpp"
// #include "MyScanCallbacks.hpp"
#include "MyNotification.hpp"
#include "MyClientCallBacks.hpp"
#include "MyLog.cpp"
// #include "MyTimer.cpp"
#include "MySdCard.hpp"
#include "MyWiFi.hpp"
#include "MyMqtt.hpp"
#include "VoltMater.hpp"
// #include "LipoMater.hpp"
#include "MyM5.hpp"

#define CONFIG_FILE "/config.json"

const char *TAG = "main";
// MyLcd2 myLcd;
int numberOfBleDevices = 1;
int numberOfThermoDevices = 1;

MyM5 myM5(&numberOfBleDevices);
MySdCard mySdCard(&myM5);
JsonDocument configJson;
// JsonArray deviceList;
// WiFiMulti wifiMulti;
MyWiFi myWiFi(&myM5);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
// PowerSaving2 powerSaving(&myLcd);

// MyTimer myTimer(0, 10000);
MyTimer myTimerArr[3];
// MyTimer myTimerArr2[3];
int timeoutCount = 0;
VoltMater voltMater;
// LipoMater lipoMater;
MyBLE2 myBleArr[3];

MyThermo myThermoArr[2];
// std::vector<MyBLE2> *bleDevices;

MyScanCallbacks myScanCallbacks(myBleArr, &myM5, &numberOfBleDevices, myThermoArr, &numberOfThermoDevices);
MyClientCallbacks myClientCallbacks(myBleArr, &numberOfBleDevices, myThermoArr, &numberOfThermoDevices);
MyNotification myNotification(myBleArr, myTimerArr, &myScanCallbacks, &myClientCallbacks, myThermoArr);
MyMqtt myMqtt(&mqttClient, myBleArr, &numberOfBleDevices, &voltMater, &myM5, myThermoArr, &numberOfThermoDevices);

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
  mySdCard.setup();
  configJson = mySdCard.loadConfig(CONFIG_FILE);
  DEBUG_PRINT("loadConfig() done\n");

  myWiFi.setup(configJson);

  // setup DateTime
  DEBUG_PRINT("Going to setup date\n");
  myM5.println("Going to setup date");
  myWiFi.setupDateTime();
  myM5.lastReset = DateTime.getTime(); //
  myM5.println("setup date done");

  // setup MQTT
  myMqtt.mqttServerSetup(configJson);
  // publishing Homeassistant discovery
  if (!mqttClient.connected())
  {
    myMqtt.reConnectMqttServer();
  }
  myM5.println("publishing Home assistant discovery");
  myMqtt.publishHaDiscovery();

  // download POI
  DEBUG_PRINT("Going to download POI\n");
  mySdCard.updatePOI(configJson);

  // setup BLE and start scanning
  DEBUG_PRINT("Starting NimBLE Client\n");
  myM5.println("Going to setup BLE");
  DEBUG_PRINT("Scanning for BLE devices\n");
  scanBle();
}

int failCount = 0;
void loop()
{
  myM5.detectButton();
  mqttClient.loop();
  /** Loop here until we find a device we want to connect to */
  if (myScanCallbacks.doConnect)
  {
    myScanCallbacks.doConnect = false;
    DEBUG_PRINT("Found a BMS we want to connect to, do it now.\n");
    // myClientCallbacks.numberOfAdvDevices = myScanCallbacks.numberOfAdvDevices;

    DEBUG_PRINT("*myClientCallbacks.numberOfBleDevices: %d\n", *myClientCallbacks.numberOfBleDevices);

    if (myNotification.connectToServer())
    {
      DEBUG_PRINT("Success! we should now be getting notifications from BMS(%d).\n", numberOfBleDevices);
      myMqtt.mqttDeviceSetup();
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
      DEBUG_PRINT("Failed to connect BMS, goint to reset\n");
      myM5.println("Failed to connect BMS, goint to reset");
      delay(2000);
      myM5.reset();
    }
  }

  if (myScanCallbacks.doConnectThermo)
  {
    myScanCallbacks.doConnectThermo = false;
    DEBUG_PRINT("Found a thermomater we want to connect to, do it now.\n");
    DEBUG_PRINT("*myClientCallbacks.numberOfThermoDevices: %d\n", *myClientCallbacks.numberOfThermoDevices);
    if (myNotification.connectToThermo())
    {
      DEBUG_PRINT("Success! we should now be getting notifications from Thermometers(%d).\n", numberOfThermoDevices);
      myMqtt.mqttThermoSetup();
    }
    else
    {
      DEBUG_PRINT("Failed to connect Thermomater");
      /*
      DEBUG_PRINT(", goint to reconnect\n");
      delay(2000);
      myScanCallbacks.doConnectThermo = true;
      //
      DEBUG_PRINT(", goint to reset\n");
      myM5.println("Failed to connect Thermomater, goint to reset");
      delay(2000);
      myM5.reset();
      */
    }
  }

  try
  {
    for (int bleIndex = 0; bleIndex < numberOfBleDevices; bleIndex++)
    {
      if (!myBleArr[bleIndex].connected)
      {
        DEBUG_PRINT("[%d}check myBleArr[%d].connected == false, going to skip the next bleIndex\n", failCount, bleIndex);
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
      }
      if (myBleArr[bleIndex].pChr_tx)
      {
        bool timeout = false;
        if (timeout = myTimerArr[bleIndex].timeout(millis()))
        {
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
  }
  catch (const std::runtime_error &e)
  {
    DEBUG_PRINT("error happened: %s\n", e.what());
    DEBUG_PRINT("reconnect myBleArr\n");
    myScanCallbacks.doConnect = false;
    // DEBUG_PRINT("rescan BLE\n");
    // scanBle();
  }

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
  //
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