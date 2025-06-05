#ifndef MY_MQTT_HPP
#define MY_MQTT_HPP

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <StreamUtils.h>
#include <WiFi.h>

#include "MyLog.cpp"
#include "MyBLE.cpp"
#include "VoltMater.hpp"
#include "LipoMater.hpp"

class MyMqtt
{
private:
    const char *TAG = "MyMqtt";

public:
    bool mqttDisabled;
    PubSubClient mqttClient;
    const int mqttMessageSizeLimit = 256; // const
    String mqttServer;
    String mqttServerArr[3];
    int mqttPort;            // const
    String mqttUser;         // const
    String mqttPass;         // const
    String systemTopic;      // const
    JsonDocument configJson; // const
    MyBLE *myBleArr;
    int numberOfBleDevices; // const
    JsonArray deviceList;   // const
    VoltMater *voltMater;
    LipoMater *lipoMater;

    MyMqtt()
    {
    }

    void setup(JsonDocument configJson_, WiFiClient wifiClient, MyBLE *myBleArr_, int numberOfBleDevices_, VoltMater *voltMater, LipoMater *lipoMater)
    {
        configJson = configJson_;
        String server = configJson["mqtt"]["server"];
        mqttServer = server;
        mqttPort = (int)configJson["mqtt"]["port"];
        String user = configJson["mqtt"]["user"];
        mqttUser = user;
        String pass = configJson["mqtt"]["password"];
        mqttPass = pass;
        deviceList = configJson["devices"].as<JsonArray>();
        DEBUG_PRINT("deviceList.size() = %d\n", deviceList.size());
        myBleArr = myBleArr_;
        numberOfBleDevices = numberOfBleDevices_;
        DEBUG_PRINT("numberOfBleDevices = %d\n", numberOfBleDevices);
        for (int bleIndex = 0; bleIndex < numberOfBleDevices; bleIndex++)
        {
            for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
            {
                //DEBUG_PRINT("bleIndex = %d deviceIndex = %d\n", bleIndex, deviceIndex);
                String mac = deviceList[deviceIndex]["mac"];
                DEBUG_PRINT("myBleArr[%d].mac = %s, deviceList[%d][\"mac\"] = %s\n", bleIndex, myBleArr[bleIndex].mac.c_str(), deviceIndex, mac.c_str());
                if (myBleArr[bleIndex].mac.equals(mac))
                {
                    String topic = deviceList[deviceIndex]["topic"];
                    myBleArr[bleIndex].topic = topic;
                    myBleArr[bleIndex].numberOfTemperature = (int)deviceList[deviceIndex]["numberOfTemperature"];
                    DEBUG_PRINT("myBleArr[%d].topic = %s, numberOfTemperature = %d\n", bleIndex,
                        myBleArr[bleIndex].topic.c_str(), myBleArr[bleIndex].numberOfTemperature);
                    break;
                }
            }
        }

        PubSubClient mqttClient(wifiClient);
    }

    void reConnectMqttServer()
    {
        DEBUG_PRINT("reConnectMqttServer() called");
        int i = 1;
        while (!mqttClient.connected())
        {
            DEBUG_PRINT("Attempting MQTT connection...");
            // Create a random client ID.
            String clientId = "M5Stack-";
            clientId += String(random(0xffff), HEX);
            // Attempt to connect.
            bool isConnected = mqttClient.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str());
            DEBUG_PRINT("mqttUser: %s, mqttPass: %s", mqttUser.c_str(), mqttPass.c_str());
            // if (client->connect(clientId.c_str()))
            if (isConnected)
            {
                DEBUG_PRINT("Connected.");
                // Once connected, publish an announcement to the topic.
                String topicStr = "stat/" + systemTopic + "STATE";
                publish(topicStr.c_str(), "MQTT reconnected");
                // ... and resubscribe.
                topicStr = "cmnd/" + systemTopic + "#";
                mqttClient.subscribe(topicStr.c_str());
                DEBUG_PRINT("topic(%s) subscribed", topicStr.c_str());
                for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
                {
                    JsonDocument deviceObj = deviceList[deviceIndex];
                    String deviceTopic = deviceObj["mqtt"]["topic"];
                    topicStr = "cmnd/" + deviceTopic + "#";
                    mqttClient.subscribe(topicStr.c_str());
                    DEBUG_PRINT("topic(%s) subscribed", topicStr.c_str());
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
                DEBUG_PRINT(logStr.c_str());
                if (i == 5)
                {
                    DEBUG_PRINT("failed to reConnectMqttServer many times.");
                    mqttDisabled = true;
                    M5.shutdown(1);
                }
                if (i == 3)
                {
                    DEBUG_PRINT("failed to reConnectMqttServer a few times. Use the second mqttServer");
                    String mqttServerConf2 = configJson["mqtt"]["server2"];
                    if (mqttServerConf2 != "null")
                    {
                        mqttServer = mqttServerConf2;
                        DEBUG_PRINT("MQTT changed mqttServer: %s", mqttServer.c_str());
                    }
                    mqttClient.setServer(mqttServer.c_str(), mqttPort);
                }
                i++;
                DEBUG_PRINT("try to reconnect again in 1 seconds");
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
        DEBUG_PRINT("publishing Json >>>>> topic: %s, payload: %s", topic.c_str(), jsonStr.c_str());
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
        DEBUG_PRINT("loading discovery payload from config.json");
        for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
        {
            JsonDocument deviceObj = deviceList[deviceIndex];
            String deviceTopic = deviceObj["mqtt"]["topic"];
            discoveryTopic = "homeassistant/device/" + deviceTopic + "config";
            discoveryPayload = deviceObj["mqtt"]["discoveryPayload"];
            DEBUG_PRINT("%d: publishing for HA discovery.................", deviceIndex);
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
        DEBUG_PRINT("mqttCallback invoked.");

        String msgStr = "";
        for (int i = 0; i < length; i++)
        {
            msgStr = msgStr + (char)payload[i];
        }
        String logStr = "Message arrived[";
        logStr += topic_;
        logStr += "] ";
        logStr += msgStr;
        DEBUG_PRINT(logStr.c_str());
        // LOGLCD(TAG, logStr);

        if (String(topic_).equals("cmnd/" + voltMater->topic + "getState"))
        {
            DEBUG_PRINT("responding to geState of %s", voltMater->topic.c_str());
            publishJson(("stat/" + voltMater->topic + "RESULT").c_str(), voltMater->getState(), false);
            return;
        }

        if (String(topic_).equals("cmnd/" + lipoMater->topic + "getState"))
        {
            DEBUG_PRINT("responding to geState of %s", lipoMater->topic.c_str());
            publishJson(("stat/" + lipoMater->topic + "RESULT").c_str(), lipoMater->getState(), false);
            return;
        }

        int chargeStatus;
        int dischargeStatus;
        bool connectionStatus;
        for (int bleIndex = 0; bleIndex < numberOfBleDevices; bleIndex++)
        {
            DEBUG_PRINT("myBleArr[%d].topic = %s", bleIndex, myBleArr[bleIndex].topic.c_str());
            // DEBUG_PRINT("myBleArr[%d].available = %d", bleIndex, myBleArr[bleIndex].available);
            // if (!myBleArr[bleIndex].available)
            {
                DEBUG_PRINT("myBleArr[%d] is not available so continue.", bleIndex);
                continue;
            }
            String deviceTopic = myBleArr[bleIndex].topic;
            chargeStatus = myBleArr[bleIndex].packBasicInfo.MosfetStatus & 1;
            dischargeStatus = (myBleArr[bleIndex].packBasicInfo.MosfetStatus & 2) >> 1;
            // connectionStatus = myBleArr[bleIndex].isConnected();

            if (String(topic_).equals("cmnd/" + deviceTopic + "getBmsState"))
            {
                DEBUG_PRINT("responding to getBmsState of %s!", deviceTopic.c_str());
                publishJson(("stat/" + deviceTopic + "RESULT").c_str(), myBleArr[bleIndex].getState(), false);
                return;
            }
            /*
            if (String(topic_).equals("cmnd/" + deviceTopic + "connection"))
            {
                DEBUG_PRINT("connection status: %d", connectionStatus);
                if (msgStr.equals(""))
                {
                    if (connectionStatus)
                        msgStr = "ON";
                    else
                        msgStr = "OFF";
                }
                else if (msgStr.equals("0"))
                {
                    myBleArr[bleIndex].disconnectFromServer();
                    msgStr = "OFF";
                    connectionStatus = false;
                }
                else if (msgStr.equals("1"))
                {
                    myBleArr[bleIndex].connectToServer();
                    msgStr = "ON";
                    connectionStatus = true;
                }
                else
                {
                    msgStr = "INVALID";
                }
                DEBUG_PRINT("responding to connection");
                publish(("stat/" + deviceTopic + "CONNECTION").c_str(), msgStr.c_str());
                msgStr = "{\"connectionStatus\": " + String(connectionStatus) + "}";
                publish(("stat/" + deviceTopic + "RESULT").c_str(), msgStr.c_str());
                publish(("stat/" + deviceTopic + "STATE").c_str(), msgStr.c_str());
                return;
            }*/

            if ((String(topic_).equals("cmnd/" + deviceTopic + "charge")) || ((String(topic_).equals("cmnd/" + deviceTopic + "discharge"))))
            {
                if (String(topic_).equals("cmnd/" + deviceTopic + "charge"))
                {
                    DEBUG_PRINT("charge status: %d, discharge status: %d", chargeStatus, dischargeStatus);
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
                    DEBUG_PRINT("responding to charge!");
                    publish(("stat/" + deviceTopic + "CHARGE").c_str(), msgStr.c_str());
                }
                else if (String(topic_).equals("cmnd/" + deviceTopic + "discharge"))
                {
                    DEBUG_PRINT("charge status: %d, discharge status: %d", chargeStatus, dischargeStatus);
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
                    DEBUG_PRINT("responding to discharge!");
                    publish(("stat/" + deviceTopic + "DISCARGE").c_str(), msgStr.c_str());
                }
                msgStr = "{\"chargeStatus\": " + String(chargeStatus) + ", \"dischargeStatus\": " + String(dischargeStatus) + "}";
                publish(("stat/" + deviceTopic + "RESULT").c_str(), msgStr.c_str());
                publish(("stat/" + deviceTopic + "STATE").c_str(), msgStr.c_str());
                return;
            }
        }
    }
};

#endif /* MY_MQTT_HPP */