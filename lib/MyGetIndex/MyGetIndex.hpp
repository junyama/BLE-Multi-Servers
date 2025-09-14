#ifndef MY_GET_INDEX_HPP
#define MY_GET_INDEX_HPP

#include <NimBLEDevice.h>
#include "MyBLE2.hpp"
#include "MyThermo.hpp"

// #include "MyScanCallbacks.hpp"
// #include "MyClientCallbacks.hpp"
#include "MyLog.hpp"
// #include "MyTimer.hpp"

#define BLE_ARR_SIZE 3
//#define THERMO_ARR_SIZE 5

class MyGetIndex
{
private:
    // MyBLE2 *myBleArr;
    // MyThermo *myThermoArr;

    // MyTimer *myTimerArr;
    // MyClientCallbacks *myClientCallbacks;
    // MyScanCallbacks *myScanCallbacks;

public:
    static const char *TAG;

    // int numberOfConnectedBMS = 0;
    // int numberOfConnectedThermo = 0;

    // MyGetIndex(MyBLE2 *myBleArr_, MyThermo *myThermoArr_, MyScanCallbacks *myScanCallbacks_);
    static int myBleArr(MyBLE2 *myBleArr, NimBLEClient *client);
    //static int myThermoArr(MyThermo *myThermoArr, NimBLEClient *client);

    static String bleInfo(MyBLE2 *myBleArr, int index);
    //static String thermoInfo(MyThermo *myThermoArr, int index);

    static int bleDevices(std::vector<MyBLE2> *bleDevices, NimBLEClient *client);
    static int thermoDevices(std::vector<MyThermo> *thermoDevices, NimBLEClient *client);

    static String bleInfo(std::vector<MyBLE2> *bleDevices, int index);
    static String thermoInfo(std::vector<MyThermo> *thermoDevices, int index);
};

#endif /* MY_GET_INDEX_HPP */