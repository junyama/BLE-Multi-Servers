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
    PubSubClient *mqttClient;
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

    void setup(JsonDocument configJson_, PubSubClient *mqttClient, MyBLE *myBleArr_, int numberOfBleDevices_, VoltMater *voltMater, LipoMater *lipoMater)
    {
        configJson = configJson_;
        String server = configJson["mqtt"]["server"];
        mqttServer = server;
        // myLcd.println("Going to setup BLE\n");
        mqttPort = (int)configJson["mqtt"]["port"];
        String user = configJson["mqtt"]["user"];
        mqttUser = user;
        String pass = configJson["mqtt"]["password"];
        mqttPass = pass;
        mqttClient->setServer(mqttServer.c_str(), mqttPort);
        // mqttClient->setCallback(mqttCallback);
        deviceList = configJson["devices"].as<JsonArray>();
        DEBUG_PRINT("deviceList.size() = %d\n", deviceList.size());
        myBleArr = myBleArr_;
        numberOfBleDevices = numberOfBleDevices_;
        DEBUG_PRINT("numberOfBleDevices = %d\n", numberOfBleDevices);
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
                    DEBUG_PRINT("myBleArr[%d].mac = %s, deviceList[%d][\"mac\"] = %s\n", bleIndex, myBleArr[bleIndex].mac.c_str(), deviceIndex, mac.c_str());
                    if (myBleArr[bleIndex].mac.equals(mac))
                    {
                        String topic = deviceObj["mqtt"]["topic"];
                        myBleArr[bleIndex].topic = topic;
                        myBleArr[bleIndex].numberOfTemperature = (int)deviceObj["numberOfTemperature"];
                        DEBUG_PRINT("myBleArr[%d].topic = %s, numberOfTemperature = %d\n", bleIndex,
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
                lipoMater->setup(deviceObj);
            }
        }
        //PubSubClient mqttClient(wifiClient);
    }

    void reConnectMqttServer()
    {
        DEBUG_PRINT("reConnectMqttServer() called\n");
        int i = 1;
        while (!mqttClient->connected())
        {
            DEBUG_PRINT("Attempting MQTT connection...\n");
            // Create a random client ID.
            String clientId = "M5Stack-";
            clientId += String(random(0xffff), HEX);
            // Attempt to connect.
            DEBUG_PRINT("mqttUser: %s, mqttPass: %s", mqttUser.c_str(), mqttPass.c_str());
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
                DEBUG_PRINT("topic(%s) subscribed", topicStr.c_str());
                for (int deviceIndex = 0; deviceIndex < deviceList.size(); deviceIndex++)
                {
                    JsonDocument deviceObj = deviceList[deviceIndex];
                    String deviceTopic = deviceObj["mqtt"]["topic"];
                    topicStr = "cmnd/" + deviceTopic + "#";
                    mqttClient->subscribe(topicStr.c_str());
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
                logStr += mqttClient->state();
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
                    String mqttServerConf2 = configJson["mqtt"]["server2"];
                    if (mqttServerConf2 != "null")
                    {
                        mqttServer = mqttServerConf2;
                        DEBUG_PRINT("MQTT changed mqttServer: %s", mqttServer.c_str());
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

    void publish(String topic, String message)
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

    void publishJson(String topic, JsonDocument doc, bool retained)
    {
        if (mqttDisabled)
            return;
        String jsonStr;
        serializeJson(doc, jsonStr);
        DEBUG_PRINT("publishing Json >>>>> topic: %s, payload: %s", topic.c_str(), jsonStr.c_str());
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
};

#endif /* MY_MQTT_HPP */