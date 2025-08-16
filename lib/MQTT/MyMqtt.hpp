#ifndef MY_MQTT_HPP
#define MY_MQTT_HPP

#include <PubSubClient.h>
#include <StreamUtils.h>

// #include "MyScanCallbacks.hpp"
#include "MyBLE2.hpp"
#include "VoltMater.hpp"
//#include "LipoMater.hpp"
//#include "PowerSaving2.hpp"
#include "MyLog.cpp"
#include "MyM5.hpp"

class MyMqtt
{
private:
    const char *TAG = "MyMqtt";
    JsonArray deviceList;
    String mqttServer;
    JsonArray mqttServerArr;
    int mqttPort;
    String mqttUser;
    String mqttPass;
    //String systemTopic;
    const int mqttMessageSizeLimit = 256;

    PubSubClient *mqttClient;
    JsonDocument configJson;
    // MyScanCallbacks myScanCallbacks;
    //PowerSaving2 *powerSaving;
    MyBLE2 *myBleArr;
    int *numberOfBleDevices;
    VoltMater *voltMater;
    //LipoMater *lipoMater;
    MyM5 *myM5;

    bool mqttDisabled;

public:
    MyMqtt(PubSubClient *mqttClient_, MyBLE2 *myBleArr_, int *numberOfBleDevices_, VoltMater *voltMater_, MyM5 *myM5_);
    void mqttServerSetup(JsonDocument configJson);
    void mqttDeviceSetup();
    void reConnectMqttServer();
    void publish(String topic, String message);
    void publishJson(String topic, JsonDocument doc, bool retained);
    void publishHaDiscovery();
    void mqttCallback(char *topic_, byte *payload, unsigned int length);
};

#endif /* MY_MQTT_HPP */