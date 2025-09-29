#ifndef MY_MQTT_HPP
#define MY_MQTT_HPP

#include <PubSubClient.h>
#include <StreamUtils.h>

// #include "MyScanCallbacks.hpp"
#include "MyNotification.hpp"

#include "MyWiFi.hpp"
#include "MyBLE2.hpp"
#include "VoltMater.hpp"
// #include "LipoMater.hpp"
// #include "PowerSaving2.hpp"
#include "MyLog.hpp"
#include "MyM5.hpp"
#include "MyThermo.hpp"

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
    // String systemTopic;
    const int mqttMessageSizeLimit = 256;

    PubSubClient *mqttClient;
    JsonDocument configJson;
    // MyScanCallbacks myScanCallbacks;
    // PowerSaving2 *powerSaving;
    MyWiFi *myWiFi;
    //MyBLE2 *myBleArr;
    MyScanCallbacks *myScanCallbacks;
    MyNotification *myNotification;
    //int numberOfBleDevices;
    //MyThermo *myThermoArr;
    //int numberOfThermoDevices;

    VoltMater *voltMater;
    // LipoMater *lipoMater;
    MyM5 *myM5;

    bool mqttDisabled;

public:
    MyMqtt(PubSubClient *mqttClient_, VoltMater *voltMater_,
           MyM5 *myM5_, MyWiFi *myWiFi_, MyNotification *myNotification_, MyScanCallbacks *myScanCallbacks_);
    void mqttServerSetup(JsonDocument configJson);
    //void mqttDeviceSetup(int numberOfBleDevices_);
    // void mqttThermoSetup(int numberOfThermoDevices_);
    void deviceSetup();
    void bmsSetup();
    void thermoSetup();
    void bm6Setup();
    void reConnectMqttServer();
    void publish(String topic, String message);
    void publishJson(String topic, JsonDocument doc, bool retained);
    void publishHaDiscovery();
    void mqttCallback(char *topic_, byte *payload, unsigned int length);
};

#endif /* MY_MQTT_HPP */