#ifndef MY_THERMO_HPP
#define MY_THERMO_HPP

#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "MyLog.hpp"
// #include "MyMqtt.hpp"
// #include "MyClientCallbacks.cpp"

class MyThermo
{
private:
    const char *TAG = "MyThermo";
    NimBLEAddress peerAddress;
    byte commandParam = 0;
    bool toggle = false;
    byte ctrlCommand = 0;

    float temp = 0.0;
    float humi = 0.0;
    float voltage = 0.0;
    //bool freeL = true;

public:
    bool newPacketReceived = false;
    NimBLERemoteCharacteristic *pChr_rx_temp = nullptr;
    NimBLERemoteCharacteristic *pChr_rx_humid = nullptr;

    //bool available = false;
    String deviceName = "UNKNOWN";
    String mac = "UNKNOWN";
    String topic = "NOT_DEFINED";
    bool connected = false;

    int measurmentIntervalMs = 10000;
    unsigned long lastMeasurment = 0;

    // NimBLEClientCallbacks clientCallbacks;

    MyThermo();
    void setup(JsonDocument deviceObj);
    bool timeout(int currentTime);
    void processTempPacket(char *data, uint32_t dataSize);
    void processHumidPacket(char *data, uint32_t dataSize);
    JsonDocument getState();
};

#endif /* MY_THERMO_HPP */