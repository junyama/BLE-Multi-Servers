#ifndef MY_GET_INDEX_HPP
#define MY_GET_INDEX_HPP

#include <NimBLEDevice.h>
#include "MyScanCallbacks.hpp"
#include "MyBLE2.hpp"
#include "MyThermo.hpp"

//#include "MyScanCallbacks.hpp"
//#include "MyClientCallbacks.hpp"
#include "MyLog.hpp"
//#include "MyTimer.hpp"

#define BLE_ARR_SIZE 3
#define THERMO_ARR_SIZE 5

class MyGetIndex
{
private:
    //MyBLE2 *myBleArr;
    //MyThermo *myThermoArr;

    //MyTimer *myTimerArr;
   // MyClientCallbacks *myClientCallbacks;
    //MyScanCallbacks *myScanCallbacks;

public:
    static const char *TAG;

    //int numberOfConnectedBMS = 0;
    //int numberOfConnectedThermo = 0;

    //MyGetIndex(MyBLE2 *myBleArr_, MyThermo *myThermoArr_, MyScanCallbacks *myScanCallbacks_);
    static int myBleArr(MyBLE2 *myBleArr, NimBLEClient *client);
    static int myThermoArr(MyThermo *myThermoArr, NimBLEClient *client);

    static int bleDevices(MyScanCallbacks *myScanCallbacks, NimBLEClient *client);
    static int thermoDevices(MyScanCallbacks *myScanCallbacks, NimBLEClient *client);

    static String bleInfo(MyBLE2 *myBleArr, int index);
    static String thermoInfo(MyThermo *myThermoArr, int index);

    static String bleInfo(MyScanCallbacks *myScanCallbacks, int index);
    static String thermoInfo(MyScanCallbacks *myScanCallbacks, int index);
};

#endif /* MY_GET_INDEX_HPP */