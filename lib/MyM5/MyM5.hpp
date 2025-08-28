#ifndef MY_M5_HPP
#define MY_M5_HPP

#include <M5Core2.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "MyLog.hpp"
//#include "MyLcd2.hpp"

#define MSG_BUFFER_SIZE (50)

class MyM5
{
private:
    const String TAG = "PowerSaving";
    //MyLcd2 *myLcd;
    int height;
    int width;
    bool isBatteryInfoShown = false;

    typedef struct
    {
        String deviceName = "== UNKOWN ==";
        String mac = "== UNKNOWN ==";
        float volt = 0.0;    // unit 1mV
        float current = 0.0; // unit 1mA
        float cellDiff = 0.0;
        int capacityRemain = 0;   // unit 1%
        float temparature1 = 0.0; // unit 0.1C
        float temparature2 = 0.0;
    } BmsInfoStruct;

    typedef struct
    {
        float vMater;
        float lipoVolt;
        float lipoCurrent;
    } VMaterLipoInfoStruct;
    VMaterLipoInfoStruct vMaterLipoInfo;

public:
    int lcdState = 1;
    int ledState = 1;
    float lipoVoltage;
    float lipoCurrent;
    int measurmentIntervalMs = 60000;
    unsigned long lastMeasurment = 0;
    int resetIntervalSec = 3600;
    unsigned long lastReset;

    int numberOfConnectedBMS;
    int numberOfConnectedThermo;
    int bmsIndexShown = 0;
    BmsInfoStruct bmsInfoArr[3];

    String topic;

    MyM5();
    void setup(JsonDocument deviceObj);
    void powerSave(int status);
    void lcdSwitch(int state);
    void ledSwitch(int state);
    JsonDocument getState();

    bool timeout(int currentTime);
    bool resetTimeout(int currentTime);
    void detectButton();
    void shutdown(int sec);
    void reset();

    void println(String text);
    void updateBmsInfo(int bmsIndex, float packVoltage, float current, float cellDiff, float temparature, float mainVolt, int capacityRemain);
    void updateVoltMaterInfo(float volt);
    void updateLipoInfo();
    void showBatteryInfo();
};

#endif /* MY_M5_HPP */