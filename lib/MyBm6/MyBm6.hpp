#ifndef MY_BM6_HPP
#define MY_BM6_HPP

#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "MyLog.hpp"

#include "MyAes.hpp"

class MyBm6
{
private:
    const char *TAG = "MyBm6";
    //const u_int8_t key[16] = {108, 101, 97, 103, 101, 110, 100, 255, 254, 48, 49, 48, 48, 48, 48, 57};
    MyAes myAes;

public:
    bool newPacketReceived = false;
    NimBLERemoteCharacteristic *pChr_rx = nullptr;
    NimBLERemoteCharacteristic *pChr_tx = nullptr;
    String deviceName = "UNKNOWN";
    NimBLEAddress peerAddress;
    String mac = "00:00:00:00:00:00";
    String topic = "NOT_DEFINED/";
    bool connected = false;

    float voltage = 0.0;
    int temperature = 0;
    int soc = 0;

    int measurmentIntervalMs = 10000;
    unsigned long lastMeasurment = 0;

    MyBm6();
    MyBm6(NimBLEAddress peerAddress_, String deviceName_);
    void sendInfoCommand();
    void sendCommand(NimBLERemoteCharacteristic *pChr, uint8_t *data, uint32_t dataLen);
    bool bleCollectPacket(char *data, uint32_t dataSize);
    //bool processInfo(byte *data, unsigned int dataLen);
    JsonDocument getState();

    bool timeout(unsigned long currentTime);
};

#endif /* MY_BM6_HPP */