
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
#include "MyBLE.cpp"
#include "MyScanCallBacks.cpp"
#include "MyClientCallBacks.cpp"

#include "MyLog.cpp"
#include "MyTimer.cpp"
#include "MySdCard.hpp"
#include "MyWiFi.cpp"
// #include "MyMqtt.cpp"
#include "VoltMater.hpp"
#include "LipoMater.hpp"

const char *TAG = "main";
JsonDocument configJson;
WiFiMulti wifiMulti;
WiFiClient wifiClient;

PubSubClient mqttClient(wifiClient);

PowerSaving2 powerSaving;
MyLcd2 myLcd;
// MyTimer myTimer(0, 10000);
MyTimer myTimerArr[3];
MyTimer myTimerArr2[3];
VoltMater voltMater;
LipoMater lipoMater;
MyBLE myBleArr[3];
std::vector<MyBLE> *bleDevices;
// MyMqtt myMqtt;

MyScanCallbacks myScanCallbacks(myBleArr, bleDevices);
MyClientCallbacks myClientCallbacks(myBleArr, bleDevices);

void loadConfig();
void saveConfig();

void wifiScann();
int wifiConnect();
void setupDateTime();

bool mqttDisabled;
// PubSubClient *mqttClient;
const int mqttMessageSizeLimit = 256; // const
String mqttServer;
JsonArray mqttServerArr;
int mqttPort;         // const
String mqttUser;      // const
String mqttPass;      // const
String systemTopic;   // const
JsonArray deviceList; // const

// void mqttServerSetup();
// void mqttDeviceSetup();
// void reConnectMqttServer();
void publish(String topic, String message);
// void publishJson(String topic, JsonDocument doc, bool retained);
// void publishHaDiscovery();
// void publishHaDiscovery2(JsonDocument deviceObj);
void mqttCallback(char *topic_, byte *payload, unsigned int length);

void mqttServerSetup()
{
  DEBUG_PRINT("mqttServerSetup() called\n");
  String server = configJson["mqtt"]["server"];
  mqttServer = server;
  mqttServerArr = configJson["mqtt"]["servers"].as<JsonArray>();
  mqttPort = (int)configJson["mqtt"]["port"];
  String user = configJson["mqtt"]["user"];
  mqttUser = user;
  String pass = configJson["mqtt"]["password"];
  mqttPass = pass;
  String topic = configJson["mqtt"]["topic"];
  systemTopic = topic;

  mqttClient.setServer(mqttServer.c_str(), mqttPort);
  mqttClient.setCallback(mqttCallback);
}

void mqttDeviceSetup()
{
  DEBUG_PRINT("mqttDeviceSetup() called\n");
  deviceList = configJson["devices"].as<JsonArray>();
  DEBUG_PRINT("mqttDeviceSetup: deviceList.size() = %d\n", deviceList.size());
  // myBleArr = myBleArr_;
  int numberOfBleDevices = myScanCallbacks.numberOfAdvDevices;
  DEBUG_PRINT("mqttDeviceSetup: numberOfBleDevices = %d\n", numberOfBleDevices);
  for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
  {
    JsonDocument deviceObj = deviceList[deviceIndex];
    String type = deviceObj["type"];
    if (type.equals("BMS"))
    {
      for (int bleIndex = 0; bleIndex < numberOfBleDevices; bleIndex++)
      {
        // DEBUG_PRINT("bleIndex = %d deviceIndex = %d\n", bleIndex, deviceIndex);
        String mac = deviceObj["mac"];
        DEBUG_PRINT("mqttDeviceSetup: myBleArr[%d].mac = %s, deviceList[%d][\"mac\"] = %s\n", bleIndex, myBleArr[bleIndex].mac.c_str(), deviceIndex, mac.c_str());
        if (myBleArr[bleIndex].mac.equals(mac))
        {
          String topic = deviceObj["mqtt"]["topic"];
          myBleArr[bleIndex].topic = topic;
          myBleArr[bleIndex].numberOfTemperature = (int)deviceObj["numberOfTemperature"];
          DEBUG_PRINT("mqttDeviceSetup: myBleArr[%d].topic = %s, numberOfTemperature = %d\n", bleIndex,
                      myBleArr[bleIndex].topic.c_str(), myBleArr[bleIndex].numberOfTemperature);
          break;
        }
      }
    }
    else if (type.equals("VAMater"))
    {
      DEBUG_PRINT("setting up volt mater\n");
      voltMater.setup(deviceObj);
    }
    else if (type.equals("Lipo"))
    {
      DEBUG_PRINT("setting up lipo mater\n");
      lipoMater.setup(deviceObj);
    }
  }
}

void reConnectMqttServer()
{
  DEBUG_PRINT("reConnectMqttServer() called\n");
  int i = 1;
  int j = 0;
  while (!mqttClient.connected())
  {
    DEBUG_PRINT("Attempting MQTT connection...\n");
    // Create a random client ID.
    String clientId = "M5Stack-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect.
    String str = mqttServerArr[j];
    mqttServer = str;
    DEBUG_PRINT("mqttServer: %s, mqttUser: %s, mqttPass: %s\n", mqttServer.c_str(), mqttUser.c_str(), mqttPass.c_str());
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    bool isConnected = mqttClient.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str());
    // if (client->connect(clientId.c_str()))
    if (isConnected)
    {
      DEBUG_PRINT("Connected.\n");
      // Once connected, publish an announcement to the topic.
      String topicStr = "stat/" + systemTopic + "STATE";
      publish(topicStr.c_str(), "MQTT reconnected\n");
      // ... and resubscribe.
      topicStr = "cmnd/" + systemTopic + "#";
      mqttClient.subscribe(topicStr.c_str());
      DEBUG_PRINT("topic(%s) subscribed\n", topicStr.c_str());
      for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
      {
        JsonDocument deviceObj = deviceList[deviceIndex];
        String deviceTopic = deviceObj["mqtt"]["topic"];
        topicStr = "cmnd/" + deviceTopic + "#";
        mqttClient.subscribe(topicStr.c_str());
        DEBUG_PRINT("topic(%s) subscribed\n", topicStr.c_str());
      }
      // Home aAssistant discoverry
      // publishHaDiscovery(); //this makes reconnect fail loop
      return;
    }
    else
    {
      String logStr = String(i) + ": ";
      logStr += "failed reconnecting, rc = ";
      logStr += mqttClient.state();
      logStr += "\n";
      DEBUG_PRINT(logStr.c_str());
      if (i == 5)
      {
        DEBUG_PRINT("failed to reConnectMqttServer many times.\n");
        mqttDisabled = true;
        M5.shutdown(1);
      }
      if (i == 3)
      {
        DEBUG_PRINT("failed to reConnectMqttServer a few times. Use the second mqttServer\n");
        String mqttServerConf2 = mqttServerArr[++j];
        if (mqttServerConf2 != "null")
        {
          mqttServer = mqttServerConf2;
          DEBUG_PRINT("MQTT changed mqttServer: %s\n", mqttServer.c_str());
        }
        mqttClient.setServer(mqttServer.c_str(), mqttPort);
      }
      i++;
      DEBUG_PRINT("try to reconnect again in 1 seconds\n");
      delay(1000);
    }
  }
  return;
}

void publish(String topic, String message)
{
  if (mqttDisabled)
    return;
  DEBUG_PRINT("publishing >>>>> topic: %s, message: %s\n", topic.c_str(), message.c_str());
  if (!mqttClient.connected())
  {
    reConnectMqttServer();
  }
  for (int i = 0; i < message.length(); i = i + mqttMessageSizeLimit)
  {
    mqttClient.publish(topic.c_str(), message.substring(i, i + mqttMessageSizeLimit).c_str());
  }
}

void publishJson(String topic, JsonDocument doc, bool retained)
{
  if (mqttDisabled)
    return;
  String jsonStr;
  serializeJson(doc, jsonStr);
  DEBUG_PRINT("publishing Json >>>>> topic: %s, payload: %s\n", topic.c_str(), jsonStr.c_str());
  if (!mqttClient.connected())
  {
    reConnectMqttServer();
  }
  mqttClient.beginPublish(topic.c_str(), measureJson(doc), retained);
  BufferingPrint bufferedClient(mqttClient, 32);
  serializeJson(doc, bufferedClient);
  bufferedClient.flush();
  mqttClient.endPublish();
}

void publishHaDiscovery()
{
  String discoveryTopic;
  JsonDocument discoveryPayload;
  // JsonArray deviceList = configJson["devices"].as<JsonArray>();
  DEBUG_PRINT("loading discovery payload from config.json\n");
  for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
  {
    JsonDocument deviceObj = deviceList[deviceIndex];
    String deviceTopic = deviceObj["mqtt"]["topic"];
    discoveryTopic = "homeassistant/device/" + deviceTopic + "config";
    discoveryPayload = deviceObj["mqtt"]["discoveryPayload"];
    DEBUG_PRINT("%d: publishing for HA discovery.................\n", deviceIndex);
    publishJson(discoveryTopic, discoveryPayload, true);
  }
}

void publishHaDiscovery2(JsonDocument deviceObj)
{
  String discoveryTopic = deviceObj["mqtt"]["topic"];
  discoveryTopic = "homeassistant/device/" + discoveryTopic + "config";
  JsonDocument discoveryPayload = deviceObj["mqtt"]["discoveryPayload"];
  publishJson(discoveryTopic, discoveryPayload, true);
}

void mqttCallback(char *topic_, byte *payload, unsigned int length)
{
  DEBUG_PRINT("mqttCallback invoked.\n");

  String msgStr = "";
  for (int i = 0; i < length; i++)
  {
    msgStr = msgStr + (char)payload[i];
  }
  String logStr = "Message arrived[";
  logStr += topic_;
  logStr += "] ";
  logStr += msgStr;
  logStr += "\n";
  DEBUG_PRINT(logStr.c_str());
  // LOGLCD(TAG, logStr);

  if (String(topic_).equals("cmnd/" + voltMater.topic + "getState"))
  {
    DEBUG_PRINT("responding to geState of %s\n", voltMater.topic.c_str());
    publishJson(("stat/" + voltMater.topic + "RESULT").c_str(), voltMater.getState(), false);
    return;
  }

  if (String(topic_).equals("cmnd/" + lipoMater.topic + "getState"))
  {
    DEBUG_PRINT("responding to geState of %s\n", lipoMater.topic.c_str());
    publishJson(("stat/" + lipoMater.topic + "RESULT").c_str(), lipoMater.getState(), false);
    return;
  }

  int chargeStatus;
  int dischargeStatus;
  bool connectionStatus;
  for (int bleIndex = 0; bleIndex < myScanCallbacks.numberOfAdvDevices; bleIndex++)
  {
    DEBUG_PRINT("myBleArr[%d].topic = %s\n", bleIndex, myBleArr[bleIndex].topic.c_str());
    // DEBUG_PRINT("myBleArr[%d].available = %d", bleIndex, myBleArr[bleIndex].available);
    /*
    if (!myBleArr[bleIndex].available)
    {
      DEBUG_PRINT("myBleArr[%d] is not available so continue.\n", bleIndex);
      continue;
    }
    */
    String deviceTopic = myBleArr[bleIndex].topic;
    chargeStatus = myBleArr[bleIndex].packBasicInfo.MosfetStatus & 1;
    dischargeStatus = (myBleArr[bleIndex].packBasicInfo.MosfetStatus & 2) >> 1;
    // connectionStatus = myBleArr[bleIndex].isConnected();

    if (String(topic_).equals("cmnd/" + deviceTopic + "getBmsState"))
    {
      DEBUG_PRINT("responding to getBmsState of %s\n", deviceTopic.c_str());
      publishJson(("stat/" + deviceTopic + "RESULT").c_str(), myBleArr[bleIndex].getState(), false);
      return;
    }
    if ((String(topic_).equals("cmnd/" + deviceTopic + "charge")) || ((String(topic_).equals("cmnd/" + deviceTopic + "discharge"))))
    {
      if (String(topic_).equals("cmnd/" + deviceTopic + "charge"))
      {
        DEBUG_PRINT("charge status: %d, discharge status: %d\n", chargeStatus, dischargeStatus);
        if (msgStr.equals(""))
        {
          if (chargeStatus)
            msgStr = "ON";
          else
            msgStr = "OFF";
        }
        else if (msgStr.equals("0"))
        {
          myBleArr[bleIndex].mosfetCtrl(0, dischargeStatus);
          chargeStatus = 0;
          msgStr = "OFF";
        }
        else if (msgStr.equals("1"))
        {
          myBleArr[bleIndex].mosfetCtrl(1, dischargeStatus);
          chargeStatus = 1;
          msgStr = "ON";
        }
        else if (msgStr.equals("toggle"))
        {
          myBleArr[bleIndex].mosfetCtrl((chargeStatus ^ 1), dischargeStatus);
          chargeStatus = chargeStatus ^ 1;
          msgStr = "TOGGLE";
        }
        else
        {
          msgStr = "INVALID";
        }
        DEBUG_PRINT("responding to charge!\n");
        publish(("stat/" + deviceTopic + "CHARGE").c_str(), msgStr.c_str());
      }
      else if (String(topic_).equals("cmnd/" + deviceTopic + "discharge"))
      {
        DEBUG_PRINT("charge status: %d, discharge status: %d\n", chargeStatus, dischargeStatus);
        if (msgStr.equals(""))
        {
          if (dischargeStatus)
            msgStr = "ON";
          else
            msgStr = "OFF";
        }
        else if (msgStr.equals("0"))
        {
          myBleArr[bleIndex].mosfetCtrl(chargeStatus, 0);
          dischargeStatus = 0;
          msgStr = "OFF";
        }
        else if (msgStr.equals("1"))
        {
          myBleArr[bleIndex].mosfetCtrl(chargeStatus, 1);
          dischargeStatus = 1;
          msgStr = "ON";
        }
        else if (msgStr.equals("toggle"))
        {
          myBleArr[bleIndex].mosfetCtrl(chargeStatus, (dischargeStatus ^ 1));
          dischargeStatus = dischargeStatus ^ 1;
          msgStr = "TOGGLE";
        }
        else
        {
          msgStr = "INVALID";
        }
        DEBUG_PRINT("responding to discharge!\n");
        publish(("stat/" + deviceTopic + "DISCARGE").c_str(), msgStr.c_str());
      }
      msgStr = "{\"chargeStatus\": " + String(chargeStatus) + ", \"dischargeStatus\": " + String(dischargeStatus) + "}";
      publish(("stat/" + deviceTopic + "RESULT").c_str(), msgStr.c_str());
      publish(("stat/" + deviceTopic + "STATE").c_str(), msgStr.c_str());
      return;
    }
  }
}

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
int getIndexOfMyBleArr(NimBLEClient *client);

/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer();

void detectButton(int numberOfPages);
void printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE myBle);

void setup()
{
  M5.begin(); // Init M5Core2.
  Serial.begin(9600);
  // MySdCard::deleteFile(SD, "/log.txt");
  MySdCard::setup();
  loadConfig();
  DEBUG_PRINT("loadConfig() done\n");

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

  mqttServerSetup();

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
  /** Loop here until we find a device we want to connect to */
  if (myScanCallbacks.doConnect)
  {
    myScanCallbacks.doConnect = false;
    DEBUG_PRINT("Found a device we want to connect to, do it now.\n");
    myClientCallbacks.numberOfAdvDevices = myScanCallbacks.numberOfAdvDevices;
    if (connectToServer())
    {
      DEBUG_PRINT("Success! we should now be getting notifications.\n");
      mqttDeviceSetup();
      if (!mqttClient.connected())
      {
        reConnectMqttServer();
      }
      publishHaDiscovery();
    }
    else
    {
      DEBUG_PRINT("Failed to connect, goint to reset\n");
      myLcd.println("Failed to connect, goint to reset");
      // NimBLEDevice::getScan()->start(myScanCallbacks.scanTimeMs, false, true);
      delay(5000);
      M5.shutdown(1);
    }
  }
  else
  {
    // DEBUG_PRINT("Not Found a device yet and chck again or already foundd and skip to send commands\n");
  }
  // delay(3000);

  // DEBUG_PRINT("\n");

  //{
  for (int bleIndex = 0; bleIndex < myScanCallbacks.numberOfAdvDevices; bleIndex++)
  {
    if (!myBleArr[bleIndex].connected)
    {
      DEBUG_PRINT("myBleArr[%d].connected == false\n", bleIndex);
      continue;
    }
    /*
    if (myTimer.timeout(millis())) // timeout to process a command and show status updated by asynchronus notifications
    {
      myBleArr[bleIndex].processCtrlCommand();

      printBatteryInfo(bleIndex, myScanCallbacks.numberOfAdvDevices, myBleArr[bleIndex]);
      myLcd.bmsInfoArr[bleIndex].deviceName = myBleArr[bleIndex].deviceName;
      DEBUG_PRINT("loop: myLcd.bmsInfoArr[%d].deviceName: %s\n", bleIndex, myLcd.bmsInfoArr[bleIndex].deviceName.c_str());
      myLcd.bmsInfoArr[bleIndex].mac = myBleArr[bleIndex].mac;
      myLcd.updateBmsInfo(bleIndex, myBleArr[bleIndex].packBasicInfo.Volts, myBleArr[bleIndex].packBasicInfo.Amps,
                          myBleArr[bleIndex].packCellInfo.CellDiff,
                          myBleArr[bleIndex].packBasicInfo.Temp1, myBleArr[bleIndex].packBasicInfo.Temp2,
                          myBleArr[bleIndex].packBasicInfo.CapacityRemainPercent);
      publishJson("stat/" + myBleArr[bleIndex].topic + "STATE", myBleArr[bleIndex].getState(), true);
    }
    */

    if (myBleArr[bleIndex].pChr_tx)
    {
      DEBUG_PRINT("loop(): myBleArr[%d].newPacketReceived: %d\n", bleIndex, myBleArr[bleIndex].newPacketReceived);
      bool timeout = false;
      if (timeout = myTimerArr[bleIndex].timeout(millis()) || myBleArr[bleIndex].newPacketReceived) // timeout to send commands
      {
        myBleArr[bleIndex].processCtrlCommand();
        //}
        if (timeout)
        {
          //
          printBatteryInfo(bleIndex, myScanCallbacks.numberOfAdvDevices, myBleArr[bleIndex]);
          myLcd.bmsInfoArr[bleIndex].deviceName = myBleArr[bleIndex].deviceName;
          DEBUG_PRINT("loop(): myLcd.bmsInfoArr[%d].deviceName: %s\n", bleIndex, myLcd.bmsInfoArr[bleIndex].deviceName.c_str());
          myLcd.bmsInfoArr[bleIndex].mac = myBleArr[bleIndex].mac;
          myLcd.updateBmsInfo(bleIndex, myBleArr[bleIndex].packBasicInfo.Volts, myBleArr[bleIndex].packBasicInfo.Amps,
                              myBleArr[bleIndex].packCellInfo.CellDiff,
                              myBleArr[bleIndex].packBasicInfo.Temp1, myBleArr[bleIndex].packBasicInfo.Temp2,
                              myBleArr[bleIndex].packBasicInfo.CapacityRemainPercent);
          //
          // publishJson("stat/" + myBleArr[bleIndex].topic + "STATE", myBleArr[bleIndex].getState(), true);

          /*
          char buff[256];
          sprintf(buff, "command to %s: Service = %s, Charastaric = %s\n",
                  myBleArr[bleIndex].pChr_tx->getClient()->getPeerAddress().toString().c_str(),
                  myBleArr[bleIndex].pChr_tx->getRemoteService()->getUUID().toString().c_str(),
                  myBleArr[bleIndex].pChr_tx->getUUID().toString().c_str());
          // myBleArr[bleIndex].bmsGetInfo5();
          //String string = "Send bmsGetInfo5 command to " + String(buff) + "\n";
          // DEBUG_PRINT(string.c_str());
          // delay(1000);
          // DEBUG_PRINT("\n");
          String string;
          if (myBleArr[bleIndex].toggle)
          {
            myBleArr[bleIndex].bmsGetInfo3();
            string = "Send bmsGetInfo3 command to " + String(buff) + "\n";
            DEBUG_PRINT(string.c_str());
          }
          else
          {
            myBleArr[bleIndex].bmsGetInfo4();
            string = "Send bmsGetInfo4 command to " + String(buff) + "\n";
            DEBUG_PRINT(string.c_str());
          }
          */
        }
        else
        {
          // publishJson("stat/" + myBleArr[bleIndex].topic + "STATE", myBleArr[bleIndex].getState(), true);
        }
      }
      else
      {
        DEBUG_PRINT("No timeout && No new packet\n");
        if (!myBleArr[bleIndex].newPacketReceived)
        {
          // publishJson("stat/" + myBleArr[bleIndex].topic + "STATE", myBleArr[bleIndex].getState(), true);
        }
      }

      /*if (timeout = myTimerArr2[bleIndex].timeout(millis())) // timeout to send commands
      {
        printBatteryInfo(bleIndex, myScanCallbacks.numberOfAdvDevices, myBleArr[bleIndex]);
        myLcd.bmsInfoArr[bleIndex].deviceName = myBleArr[bleIndex].deviceName;
        DEBUG_PRINT("loop(): myLcd.bmsInfoArr[%d].deviceName: %s\n", bleIndex, myLcd.bmsInfoArr[bleIndex].deviceName.c_str());
        myLcd.bmsInfoArr[bleIndex].mac = myBleArr[bleIndex].mac;
        myLcd.updateBmsInfo(bleIndex, myBleArr[bleIndex].packBasicInfo.Volts, myBleArr[bleIndex].packBasicInfo.Amps,
                            myBleArr[bleIndex].packCellInfo.CellDiff,
                            myBleArr[bleIndex].packBasicInfo.Temp1, myBleArr[bleIndex].packBasicInfo.Temp2,
                            myBleArr[bleIndex].packBasicInfo.CapacityRemainPercent);
        publishJson("stat/" + myBleArr[bleIndex].topic + "STATE", myBleArr[bleIndex].getState(), true);
      }*/
    }
    else
    {
      DEBUG_PRINT("myBleArr[%d].pChr_tx == null\n", bleIndex);
    }
  }
  if (voltMater.available && voltMater.timeout(millis()))
  {
    myLcd.updateVoltMaterInfo(voltMater.calVoltage);
    publishJson("stat/" + voltMater.topic + "STATE", voltMater.getState(), true);
  }
  if (lipoMater.available && lipoMater.timeout(millis()))
  {
    myLcd.updateLipoInfo();
    publishJson("stat/" + lipoMater.topic + "STATE", lipoMater.getState(), true);
  }
  //}
  // else
  // myScanCallbacks.doConnect = true;
  mqttClient.loop();
}