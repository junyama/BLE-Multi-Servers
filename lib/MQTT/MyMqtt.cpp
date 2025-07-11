#include "MyMqtt.hpp"

MyMqtt::MyMqtt(PubSubClient *mqttClient_, MyBLE2 *myBleArr_, int *numberOfBleDevices_, VoltMater *voltMater_, MyM5 *myM5_)
    : mqttClient(mqttClient_), myBleArr(myBleArr_), numberOfBleDevices(numberOfBleDevices_), voltMater(voltMater_), myM5(myM5_)
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
  String topic = configJson["mqtt"]["topic"];
  systemTopic = topic;

  mqttClient->setServer(mqttServer.c_str(), mqttPort);
  mqttClient->setCallback([this](char *topic_, byte *payload, unsigned int length)
                          { mqttCallback(topic_, payload, length); });
}

void MyMqtt::mqttDeviceSetup()
{
  DEBUG_PRINT("mqttDeviceSetup() called\n");
  // deviceList = configJson["devices"].as<JsonArray>();
  DEBUG_PRINT("mqttDeviceSetup: deviceList.size() = %d\n", deviceList.size());
  // myBleArr = myBleArr_;
  // int numberOfBleDevices = myScanCallbacks.numberOfBleDevices;
  DEBUG_PRINT("mqttDeviceSetup: *numberOfBleDevices = %d\n", *numberOfBleDevices);
  for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
  {
    JsonDocument deviceObj = deviceList[deviceIndex];
    String type = deviceObj["type"];
    if (type.equals("BMS"))
    {
      for (int bleIndex = 0; bleIndex < *numberOfBleDevices; bleIndex++)
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
      voltMater->setup(deviceObj);
    }
    else if (type.equals("Lipo"))
    {
      DEBUG_PRINT("setting up lipo mater\n");
      myM5->setup(deviceObj);
    }
    else if (type.equals("Controller"))
    {
      DEBUG_PRINT("setting up Controller\n");
      myM5->setup(deviceObj);
    }
  }
}

void MyMqtt::reConnectMqttServer()
{
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
      String topicStr = "stat/" + systemTopic + "STATE";
      publish(topicStr.c_str(), "MQTT reconnected\n");
      // ... and resubscribe.
      topicStr = "cmnd/" + systemTopic + "#";
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
  DEBUG_PRINT("publishing Json >>>>> topic: %s, payload: %s\n", topic.c_str(), jsonStr.c_str());
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
    DEBUG_PRINT("%d: publishing for HA discovery.................\n", deviceIndex);
    publishJson(discoveryTopic, discoveryPayload, true);
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
  String logStr = "Message arrived[";
  logStr += topic_;
  logStr += "] ";
  logStr += msgStr;
  logStr += "\n";
  DEBUG_PRINT(logStr.c_str());
  // LOGLCD(TAG, logStr);

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

  if (String(topic_).equals("cmnd/" + systemTopic + "lcd"))
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
    publishJson(("stat/" + systemTopic + "STATE").c_str(), myM5->getState(), true);
    publishJson(("stat/" + systemTopic + "RESULT").c_str(), myM5->getState(), true);
    return;
  }

  int chargeStatus;
  int dischargeStatus;
  bool connectionStatus;
  for (int bleIndex = 0; bleIndex < *numberOfBleDevices; bleIndex++)
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
