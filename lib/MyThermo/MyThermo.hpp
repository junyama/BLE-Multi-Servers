#ifndef MY_THERMO_HPP
#define MY_THERMO_HPP

#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "MyLog.cpp"
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

public:
    bool newPacketReceived = false;
    NimBLERemoteCharacteristic *pChr_rx = nullptr;
    NimBLERemoteCharacteristic *pChr_tx = nullptr;
    
    String deviceName = "UNKNOWN";
    String mac = "UNKNOWN";
    String topic = "NOT_DEFINED";
    bool connected = false;

    // NimBLEClientCallbacks clientCallbacks;

    MyThermo();

};

#endif /* MY_THERMO_HPP */