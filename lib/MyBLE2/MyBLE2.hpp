#ifndef MY_BLE2_HPP
#define MY_BLE2_HPP

#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "MyLog.cpp"
// #include "MyMqtt.hpp"
// #include "MyClientCallbacks.cpp"

typedef struct
{
    byte start;
    byte type;
    byte status;
    byte dataLen;
} bmsPacketHeaderStruct;

typedef struct
{
    uint16_t Volts; // unit 1mV
    int32_t Amps;   // unit 1mA
    int32_t Watts;  // unit 1W
    uint16_t CapacityRemainAh;
    uint8_t CapacityRemainPercent; // unit 1%
    uint32_t CapacityRemainWh;     // unit Wh
    uint16_t Temp1;                // unit 0.1C
    uint16_t Temp2;                // unit 0.1C
    uint16_t BalanceCodeLow;
    uint16_t BalanceCodeHigh;
    uint8_t MosfetStatus;
} packBasicInfoStruct;

typedef struct
{
    uint8_t NumOfCells;
    uint16_t CellVolt[15]; // cell 1 has index 0 :-/
    uint16_t CellMax;
    uint16_t CellMin;
    uint16_t CellDiff; // difference between highest and lowest
    uint16_t CellAvg;
    uint16_t CellMedian;
    uint32_t CellColor[15];
    uint32_t CellColorDisbalance[15]; // green cell == median, red/violet cell => median + c_cellMaxDisbalanceAdd commentMore actions
} packCellInfoStruct;

// void publish(String topic, String message);
// void publishJson(String topic, JsonDocument doc, bool retained);

class MyBLE2
{
private:
    const char *TAG = "MyBLE";
    NimBLEAddress peerAddress;
    byte commandParam = 0;
    bool toggle = false;
    byte ctrlCommand = 0;

public:
    bool newPacketReceived = false;
    NimBLERemoteCharacteristic *pChr_rx = nullptr;
    NimBLERemoteCharacteristic *pChr_tx = nullptr;
    packBasicInfoStruct packBasicInfo = {}; // here shall be the latest data got from BMS
    packCellInfoStruct packCellInfo = {};   // here shall be the latest data got from BMS
    String deviceName = "UNKNOWN";
    String mac = "UNKNOWN";
    int numberOfTemperature = 2;
    String topic = "NOT_DEFINED";
    bool connected = false;

    // NimBLEClientCallbacks clientCallbacks;

    MyBLE2();

    //MyBLE2(NimBLEAddress peerAddress_);

    //MyBLE2(const MyBLE2 &obj);

    void sendInfoCommand();

    JsonDocument getMosfetState();

    JsonDocument getState();

    byte calcChecksum(byte *packet);

    void bmsMosfetCtrl();

    void mosfetCtrl(int chargeStatus, int dischargeStatus);

    void bmsGetInfo3();

    void bmsGetInfo4();

    void bmsGetInfo5();

    void sendCommand(NimBLERemoteCharacteristic *pChr, uint8_t *data, uint32_t dataLen);

    bool processDeviceInfo(byte *data, unsigned int dataLen);

    bool bleCollectPacket(char *data, uint32_t dataSize); // reconstruct packet from BLE incomming data, called by notifyCallback function

    int16_t two_ints_into16(int highbyte, int lowbyte); // turns two bytes into a single long integerAdd commentMore actions

    bool processBasicInfo(packBasicInfoStruct *output, byte *data, unsigned int dataLen);

    bool processCellInfo(packCellInfoStruct *output, byte *data, unsigned int dataLen);

    bool bmsProcessPacket(byte *packet);
};

#endif /* MY_BLE2_HPP */