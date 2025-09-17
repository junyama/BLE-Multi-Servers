#include "MyMqtt.hpp"

MyMqtt::MyMqtt(PubSubClient *mqttClient_,  VoltMater *voltMater_,
               MyM5 *myM5_, MyWiFi *myWiFi_, MyNotification *myNotification_, MyScanCallbacks *myScanCallbacks_)
    : mqttClient(mqttClient_), voltMater(voltMater_),
      myM5(myM5_), myWiFi(myWiFi_), myNotification(myNotification_), myScanCallbacks(myScanCallbacks_)
{
}

void MyMqtt::mqttServerSetup(JsonDocument configJson_)
{
  DEBUG_PRINT("mqttServerSetup() called\n");
  configJson = configJson_;
  deviceList = configJson["devices"].as<JsonArray>();
  DEBUG_PRINT("mqttServerSetup: deviceList.size(): %d\n", deviceList.size());
  String server = configJson["mqtt"]["server"];
  mqttServer = server;
  mqttServerArr = configJson["mqtt"]["servers"].as<JsonArray>();
  mqttPort = (int)configJson["mqtt"]["port"];
  String user = configJson["mqtt"]["user"];
  mqttUser = user;
  String pass = configJson["mqtt"]["password"];
  mqttPass = pass;
  // String topic = configJson["mqtt"]["topic"];
  // systemTopic = topic;

  mqttClient->setServer(mqttServer.c_str(), mqttPort);
  mqttClient->setCallback([this](char *topic_, byte *payload, unsigned int length)
                          { mqttCallback(topic_, payload, length); });
}

void MyMqtt::deviceSetup()
{
  bool controllerFound = false;
  String mac = WiFi.macAddress();
  for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
  {
    JsonDocument deviceObj = deviceList[deviceIndex];
    String type = deviceObj["type"];
    if (type.equals("VAMater"))
    {
      INFO_PRINT("setting up volt mater\n");
      voltMater->setup(deviceObj);
    }
    else if (type.equals("Controller"))
    {
      if (mac == deviceObj["mac"])
      {
        controllerFound = true;
        INFO_PRINT("Controller with Address: %s found in config file\n", mac.c_str());
        DEBUG_PRINT("setting up Controller\n");
        myM5->setup(deviceObj);
      }
    }
  }
  if (!controllerFound)
    ERROR_PRINT("Controller with Address: %s not found in config file\n", mac.c_str());
}

void MyMqtt::bmsSetup()
{
  for (int bmsIndex = 0; bmsIndex < myScanCallbacks->advDevices.size(); bmsIndex++)
  {
    try
    {
      bool deviceFound = false;
      for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
      {
        JsonDocument deviceObj = deviceList[deviceIndex];
        String type = deviceObj["type"];
        if (type.equals("BMS"))
        {
          String mac = deviceObj["mac"];
          // DEBUG_PRINT("thermoSetup: myBleArr[%d].mac: %s, deviceList[%d][\"mac\"]: %s\n",
          // bmsIndex, myBleArr[bmsIndex].mac.c_str(), deviceIndex, mac.c_str());
          //if (NimBLEAddress(myBleArr[bmsIndex].mac.c_str(), 0).equals(NimBLEAddress(mac.c_str(), 0)))
          if (myScanCallbacks->bleDevices[bmsIndex].peerAddress == NimBLEAddress(mac.c_str(), 0))
          {
            String topic = deviceObj["mqtt"]["topic"];
            //myBleArr[bmsIndex].topic = topic;
            
            myScanCallbacks->bleDevices[bmsIndex].topic = topic;

            DEBUG2_PRINT("bmsSetup: myScanCallbacks->bleDevices[%d].mac found at config and set topic: %s\n", bmsIndex,
                         myScanCallbacks->bleDevices[bmsIndex].topic.c_str());
            // myBleArr[bmsIndex].available = true;
            // DEBUG_PRINT("mqttDeviceSetup: myBleArr[%d].available = true\n", bmsIndex);
            deviceFound = true;
            break;
          }
        }
      }
      if (!deviceFound)
      {
        char buff[256];
        sprintf(buff, "myScanCallbacks->bleDevices[%d].mac: %s not found at config",
                myScanCallbacks->bleDevices[bmsIndex].mac.c_str(), bmsIndex);
        throw std::runtime_error(buff);
      }
    }
    catch (const std::runtime_error &e)
    {
      ERROR_PRINT("%s\n", e.what());
      continue;
    }
  }
}

void MyMqtt::thermoSetup()
{
  for (int thermoIndex = 0; thermoIndex < myScanCallbacks->thermoDevices.size(); thermoIndex++)
  {
    try
    {
      bool deviceFound = false;
      for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
      {
        JsonDocument deviceObj = deviceList[deviceIndex];
        String type = deviceObj["type"];
        if (type.equals("Thermomater"))
        {
          String mac = deviceObj["mac"];
          // DEBUG_PRINT("thermoSetup: myThermoArr[%d].mac: %s, deviceList[%d][\"mac\"]: %s\n",
          // thermoIndex, myThermoArr[thermoIndex].mac.c_str(), deviceIndex, mac.c_str());
          // if (NimBLEAddress(myThermoArr[thermoIndex].mac.c_str(), 0) == (NimBLEAddress(mac.c_str(), 0)))
          if (myScanCallbacks->thermoDevices.at(thermoIndex).peerAddress == NimBLEAddress(mac.c_str(), 0))
          {
            String topic = deviceObj["mqtt"]["topic"];
            //myThermoArr[thermoIndex].topic = topic;
            myScanCallbacks->thermoDevices.at(thermoIndex).topic = topic;
            DEBUG2_PRINT("thermoSetup: thermoDevices[%d].mac found at config and set topic: %s\n", thermoIndex,
                         //myThermoArr[thermoIndex].topic.c_str());
                         myScanCallbacks->thermoDevices.at(thermoIndex).topic.c_str());
            // myThermoArr[thermoIndex].available = true;
            // DEBUG_PRINT("mqttDeviceSetup: myThermoArr[%d].available = true\n", thermoIndex);
            deviceFound = true;
            break;
          }
        }
      }
      if (!deviceFound)
      {
        char buff[256];
        sprintf(buff, "thermoDevices[%d].mac: %s not found at config",
                myScanCallbacks->thermoDevices[thermoIndex].mac.c_str(), thermoIndex);
        throw std::runtime_error(buff);
      }
    }
    catch (const std::runtime_error &e)
    {
      ERROR_PRINT("%s\n", e.what());
      continue;
    }
  }
}

void MyMqtt::reConnectMqttServer()
{
  if (!myWiFi->isConnected())
  {
    DEBUG_PRINT("reconnecting WiFi\n");
    myWiFi->connect();
  }
  DEBUG_PRINT("reConnectMqttServer() called\n");
  int i = 1;
  int j = 0;
  while (!mqttClient->connected())
  {
    DEBUG_PRINT("Attempting MQTT connection...\n");
    // Create a random client ID.
    String clientId = "M5Stack-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect.
    String str = mqttServerArr[j];
    mqttServer = str;
    DEBUG_PRINT("mqttServer: %s, mqttUser: %s, mqttPass: %s\n", mqttServer.c_str(), mqttUser.c_str(), mqttPass.c_str());
    mqttClient->setServer(mqttServer.c_str(), mqttPort);
    bool isConnected = mqttClient->connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str());
    // if (client->connect(clientId.c_str()))
    if (isConnected)
    {
      DEBUG_PRINT("Connected.\n");
      // Once connected, publish an announcement to the topic.
      String topicStr = "stat/" + myM5->topic + "STATE";
      publish(topicStr.c_str(), "MQTT reconnected\n");
      // ... and resubscribe.
      topicStr = "cmnd/" + myM5->topic + "#";
      mqttClient->subscribe(topicStr.c_str());
      DEBUG_PRINT("topic(%s) subscribed\n", topicStr.c_str());
      for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
      {
        JsonDocument deviceObj = deviceList[deviceIndex];
        String deviceTopic = deviceObj["mqtt"]["topic"];
        topicStr = "cmnd/" + deviceTopic + "#";
        mqttClient->subscribe(topicStr.c_str());
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
      logStr += mqttClient->state();
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
        mqttClient->setServer(mqttServer.c_str(), mqttPort);
      }
      i++;
      DEBUG_PRINT("try to reconnect again in 1 seconds\n");
      delay(1000);
    }
  }
  return;
}

void MyMqtt::publish(String topic, String message)
{
  if (mqttDisabled)
    return;
  DEBUG_PRINT("publishing >>>>> topic: %s, message: %s\n", topic.c_str(), message.c_str());
  if (!mqttClient->connected())
  {
    reConnectMqttServer();
  }
  for (int i = 0; i < message.length(); i = i + mqttMessageSizeLimit)
  {
    mqttClient->publish(topic.c_str(), message.substring(i, i + mqttMessageSizeLimit).c_str());
  }
}

void MyMqtt::publishJson(String topic, JsonDocument doc, bool retained)
{
  if (mqttDisabled)
    return;
  String jsonStr;
  serializeJson(doc, jsonStr);
  String str = "publish(" + topic + ", " + jsonStr;
  if (str.length() > 106)
    str = str.substring(0, 106) + "...})";
  else
    str += ")";
  INFO_PRINT("[%lu]%s\n", millis(), str.c_str());
  // INFO_PRINT("publish %s, %s...\n", topic.c_str(), jsonStr.substring(0, 80).c_str());
  if (!mqttClient->connected())
  {
    reConnectMqttServer();
  }
  mqttClient->beginPublish(topic.c_str(), measureJson(doc), retained);
  BufferingPrint bufferedClient(*mqttClient, 32);
  serializeJson(doc, bufferedClient);
  bufferedClient.flush();
  mqttClient->endPublish();
}

void MyMqtt::publishHaDiscovery()
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
    DEBUG_PRINT("%d: publishing for HA discovery\n", deviceIndex);
    // MyLog::DEBUG2 = false;
    publishJson(discoveryTopic, discoveryPayload, true);
    // MyLog::DEBUG2 = true;
  }
}

void MyMqtt::mqttCallback(char *topic_, byte *payload, unsigned int length)
{
  DEBUG_PRINT("mqttCallback invoked.\n");

  String msgStr = "";
  for (int i = 0; i < length; i++)
  {
    msgStr = msgStr + (char)payload[i];
  }
  String logStr = "Message arrived, topic: ";
  logStr += topic_;
  logStr += ", message: ";
  logStr += msgStr;
  logStr += "\n";
  DEBUG_PRINT(logStr.c_str());
  // DEBUG_PRINT("voltMater->topic: %s\n", voltMater->topic.c_str());
  // DEBUG_PRINT("myM5->topic: %s\n", myM5->topic.c_str());

  if (String(topic_).equals("cmnd/" + voltMater->topic + "getState"))
  {
    DEBUG_PRINT("responding to geState of %s\n", voltMater->topic.c_str());
    publishJson(("stat/" + voltMater->topic + "RESULT").c_str(), voltMater->getState(), false);
    return;
  }

  if (String(topic_).equals("cmnd/" + myM5->topic + "getState"))
  {
    DEBUG_PRINT("responding to geState of %s\n", myM5->topic.c_str());
    publishJson(("stat/" + myM5->topic + "RESULT").c_str(), myM5->getState(), false);
    return;
  }

  if (String(topic_).equals("cmnd/" + myM5->topic + "lcd"))
  {
    if (msgStr.equals("0"))
    {
      myM5->lcdSwitch(0);
      msgStr = "OFF";
    }
    else if (msgStr.equals("1"))
    {
      myM5->lcdSwitch(1);
      msgStr = "ON";
    }
    else
    {
      msgStr = "INVALID";
    }
    DEBUG_PRINT("responding to led!\n");
    publishJson(("stat/" + myM5->topic + "STATE").c_str(), myM5->getState(), true);
    publishJson(("stat/" + myM5->topic + "RESULT").c_str(), myM5->getState(), true);
    return;
  }

  if (String(topic_).equals("cmnd/" + myM5->topic + "led"))
  {
    if (msgStr.equals("0"))
    {
      myM5->ledSwitch(0);
      msgStr = "OFF";
    }
    else if (msgStr.equals("1"))
    {
      myM5->ledSwitch(1);
      msgStr = "ON";
    }
    else
    {
      msgStr = "INVALID";
    }
    DEBUG_PRINT("responding to led!\n");
    publishJson(("stat/" + myM5->topic + "STATE").c_str(), myM5->getState(), true);
    publishJson(("stat/" + myM5->topic + "RESULT").c_str(), myM5->getState(), true);
    return;
  }

  if (String(topic_).equals("cmnd/" + myM5->topic + "numberOfBleDevices"))
  {
    DEBUG_PRINT("responding to numberOfBleDevices!\n");
    publish(("stat/" + myM5->topic + "RESULT").c_str(), String(myScanCallbacks->advDevices.size()));
    return;
  }

  if (String(topic_).equals("cmnd/" + myM5->topic + "numberOfThermoDevices"))
  {
    DEBUG_PRINT("responding to numberOfThermoDevices!\n");
    publish(("stat/" + myM5->topic + "RESULT").c_str(), String(myScanCallbacks->advThermoDevices.size()));
    return;
  }

  if (String(topic_).equals("cmnd/" + myM5->topic + "reset"))
  {
    INFO_PRINT("goint to reset\n");
    myM5->println("goint to reset");
    delay(2000);
    myM5->reset();
    return;
  }

  int chargeStatus;
  int dischargeStatus;
  bool connectionStatus;
  for (int bleIndex = 0; bleIndex < myScanCallbacks->advDevices.size(); bleIndex++)
  {
    DEBUG_PRINT("myScanCallbacks->bleDevices[%d].topic = %s\n", bleIndex, myScanCallbacks->bleDevices[bleIndex].topic.c_str());
    // DEBUG_PRINT("myBleArr[%d].available = %d", bleIndex, myBleArr[bleIndex].available);
    /*
    if (!myBleArr[bleIndex].available)
    {
      DEBUG_PRINT("myBleArr[%d] is not available so continue.\n", bleIndex);
      continue;
    }
    */
    String deviceTopic = myScanCallbacks->bleDevices[bleIndex].topic;
    chargeStatus = myScanCallbacks->bleDevices[bleIndex].packBasicInfo.MosfetStatus & 1;
    dischargeStatus = (myScanCallbacks->bleDevices[bleIndex].packBasicInfo.MosfetStatus & 2) >> 1;
    // connectionStatus = myBleArr[bleIndex].isConnected();

    if (String(topic_).equals("cmnd/" + deviceTopic + "getBmsState"))
    {
      DEBUG_PRINT("responding to getBmsState of %s\n", deviceTopic.c_str());
      publishJson(("stat/" + deviceTopic + "RESULT").c_str(), myScanCallbacks->bleDevices[bleIndex].getState(), false);
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
          myScanCallbacks->bleDevices[bleIndex].mosfetCtrl(0, dischargeStatus);
          chargeStatus = 0;
          msgStr = "OFF";
        }
        else if (msgStr.equals("1"))
        {
          myScanCallbacks->bleDevices[bleIndex].mosfetCtrl(1, dischargeStatus);
          chargeStatus = 1;
          msgStr = "ON";
        }
        else if (msgStr.equals("toggle"))
        {
          myScanCallbacks->bleDevices[bleIndex].mosfetCtrl((chargeStatus ^ 1), dischargeStatus);
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
          myScanCallbacks->bleDevices[bleIndex].mosfetCtrl(chargeStatus, 0);
          dischargeStatus = 0;
          msgStr = "OFF";
        }
        else if (msgStr.equals("1"))
        {
          myScanCallbacks->bleDevices[bleIndex].mosfetCtrl(chargeStatus, 1);
          dischargeStatus = 1;
          msgStr = "ON";
        }
        else if (msgStr.equals("toggle"))
        {
          myScanCallbacks->bleDevices[bleIndex].mosfetCtrl(chargeStatus, (dischargeStatus ^ 1));
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
      // msgStr = "{\"chargeStatus\": " + String(chargeStatus) + ", \"dischargeStatus\": " + String(dischargeStatus) + "}";
      // publish(("stat/" + deviceTopic + "RESULT").c_str(), msgStr.c_str());
      // publish(("stat/" + deviceTopic + "STATE").c_str(), msgStr.c_str());
      JsonDocument mosfetState;
      mosfetState["chargeStatus"] = chargeStatus;
      mosfetState["dischargeStatus"] = dischargeStatus;
      publishJson(("stat/" + deviceTopic + "STATE").c_str(), mosfetState, true);
      publishJson(("stat/" + deviceTopic + "RESULT").c_str(), mosfetState, true);
      return;
    }
  }
}
