
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

#include "PowerSaving2.hpp"
#include "MyLcd2.hpp"
// #include "MyBLE.cpp"
#include "MyBLE2.hpp"
// #include "MyScanCallbacks.hpp"
#include "MyNotification.hpp"
#include "MyClientCallBacks.hpp"
#include "MyLog.cpp"
// #include "MyTimer.cpp"
#include "MySdCard.hpp"
#include "MyWiFi.cpp"
#include "MyMqtt.hpp"
#include "VoltMater.hpp"
#include "LipoMater.hpp"

#define CONFIG_FILE "/config.json"

const char *TAG = "main";
MyLcd2 myLcd;
MySdCard mySdCard(&myLcd);
JsonDocument configJson;
// JsonArray deviceList;
WiFiMulti wifiMulti;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
PowerSaving2 powerSaving;
// MyTimer myTimer(0, 10000);
MyTimer myTimerArr[3];
// MyTimer myTimerArr2[3];
int timeoutCount = 0;
VoltMater voltMater;
LipoMater lipoMater;
MyBLE2 myBleArr[3];
int numberOfBleDevices = 1;
// std::vector<MyBLE2> *bleDevices;

MyScanCallbacks myScanCallbacks(myBleArr, &myLcd, &numberOfBleDevices);
MyClientCallbacks myClientCallbacks(myBleArr, &numberOfBleDevices);
MyNotification myNotification(myBleArr, myTimerArr, &myScanCallbacks, &myClientCallbacks);
MyMqtt myMqtt(&mqttClient, myBleArr, &numberOfBleDevices, &voltMater, &lipoMater);

// void loadConfig();
// void saveConfig();

void wifiScann();
int wifiConnect();
void setupDateTime();

void detectButton(int numberOfPages);
void printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE2 myBle);

void setup()
{
  M5.begin(); // Init M5Core2.
  Serial.begin(9600);
  // MySdCard::deleteFile(SD, "/log.txt");
  mySdCard.setup();
  // loadConfig();
  configJson = mySdCard.loadConfig(CONFIG_FILE);
  DEBUG_PRINT("loadConfig() done\n");
  myMqtt.mqttServerSetup(configJson);

  // setup WiFi //
  WiFi.mode(WIFI_STA);
  WiFi.hostname("JunBMS");

  // Add list of wifi networks
  for (int i = 0; i < configJson["wifi"].size(); i++)
  {
    wifiMulti.addAP(configJson["wifi"][i]["ssid"], configJson["wifi"][i]["pass"]);
  }

  DEBUG_PRINT("going to scann WiFi\n");
  wifiScann();

  DEBUG_PRINT("going to connect WiFi\n");
  if (wifiConnect() != 0)
  {
    DEBUG_PRINT("failed to connect WiFi and exiting\n");
    exit(-1);
  }
  DEBUG_PRINT("WiFi setup done\n");
  myLcd.println("WiFi connected");

  // setup DateTime
  DEBUG_PRINT("Going to setup date\n");
  myLcd.println("Going to setup date");
  setupDateTime();
  myLcd.println("setup date done");

  powerSaving.disable();

  DEBUG_PRINT("Starting NimBLE Client\n");
  myLcd.println("Going to setup BLE");

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
  DEBUG_PRINT("Scanning for peripherals\n");
}

void loop()
{
  detectButton(myScanCallbacks.numberOfAdvDevices);
  mqttClient.loop();
  /** Loop here until we find a device we want to connect to */
  if (myScanCallbacks.doConnect)
  {
    myScanCallbacks.doConnect = false;
    DEBUG_PRINT("Found a device we want to connect to, do it now.\n");
    // myClientCallbacks.numberOfAdvDevices = myScanCallbacks.numberOfAdvDevices;
    DEBUG_PRINT("*myClientCallbacks.numberOfBleDevices: %d\n", *myClientCallbacks.numberOfBleDevices);
    if (myNotification.connectToServer())
    {
      DEBUG_PRINT("Success! we should now be getting notifications from devices(%d).\n", numberOfBleDevices);
      myMqtt.mqttDeviceSetup();
      if (!mqttClient.connected())
      {
        myMqtt.reConnectMqttServer();
      }
      myLcd.println("publishing Home assistant discovery\n");
      myMqtt.publishHaDiscovery();
    }
    else
    {
      DEBUG_PRINT("Failed to connect, goint to reset\n");
      myLcd.println("Failed to connect, goint to reset");
      delay(5000);
      M5.shutdown(1);
    }
  }

  for (int bleIndex = 0; bleIndex < myScanCallbacks.numberOfAdvDevices; bleIndex++)
  {
    if (!myBleArr[bleIndex].connected)
    {
      DEBUG_PRINT("check myBleArr[%d].connected == false, going to skip the next bleIndex\n", bleIndex);
      continue;
    }
    if (myBleArr[bleIndex].pChr_tx)
    {
      bool timeout = false;
      if (timeout = myTimerArr[bleIndex].timeout(millis()))
      {
        myBleArr[bleIndex].sendInfoCommand();
      }
      DEBUG_PRINT("timeout: %d, myBleArr[%d].newPacketReceived: %d\n", timeout, bleIndex, myBleArr[bleIndex].newPacketReceived);
      if (myBleArr[bleIndex].newPacketReceived)
      {
        myBleArr[bleIndex].newPacketReceived = false;
        myLcd.updateBmsInfo(bleIndex, myBleArr[bleIndex].packBasicInfo.Volts, myBleArr[bleIndex].packBasicInfo.Amps,
                            myBleArr[bleIndex].packCellInfo.CellDiff,
                            myBleArr[bleIndex].packBasicInfo.Temp1, myBleArr[bleIndex].packBasicInfo.Temp2,
                            myBleArr[bleIndex].packBasicInfo.CapacityRemainPercent);
        myMqtt.publishJson("stat/" + myBleArr[bleIndex].topic + "STATE", myBleArr[bleIndex].getState(), true);
      }
    }
  }
  if (voltMater.available && voltMater.timeout(millis()))
  {
    myLcd.updateVoltMaterInfo(voltMater.calVoltage);
    myMqtt.publishJson("stat/" + voltMater.topic + "STATE", voltMater.getState(), true);
  }
  if (lipoMater.available && lipoMater.timeout(millis()))
  {
    myLcd.updateLipoInfo();
    myMqtt.publishJson("stat/" + lipoMater.topic + "STATE", lipoMater.getState(), true);
  }
}