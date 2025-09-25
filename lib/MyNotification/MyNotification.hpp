#ifndef MY_NOTIFICATION_HPP
#define MY_NOTIFICATION_HPP

#include <NimBLEDevice.h>
#include "MyBLE2.hpp"
#include "MyThermo.hpp"

#include "MyScanCallbacks.hpp"
#include "MyClientCallbacks.hpp"
#include "MyLog.hpp"
#include "MyGetIndex.hpp"
#include "MyM5.hpp"
// #include "MyTimer.hpp"

#define FAIL_LIMIT 3

class MyNotification
{
private:
    const char *TAG = "MyNotification";
    //MyBLE2 *myBleArr;
    //MyThermo *myThermoArr;

    // MyTimer *myTimerArr;
    MyClientCallbacks *myClientCallbacks;
    MyScanCallbacks *myScanCallbacks;
    MyM5 *myM5;

    int failCount = 0;

public:
    //int numberOfBMS = 0;
    //int numberOfThermo = 0;
    
    //int numberOfConnectedBMS = 0;
    //int numberOfConnectedThermo = 0;

    MyNotification(MyScanCallbacks *myScanCallbacks_, MyClientCallbacks *myClientCallbacks_, MyM5 *myM5_);
    /** Notification / Indication receiving handler callback */
    void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    //int getIndexOfMyBleArr(NimBLEClient *client);
    //int getIndexOfMyThermoArr(NimBLEClient *client);
    /** Handles the provisioning of clients and connects / interfaces with the server */
    bool connectToServer();
    bool connectToThermo();
    // void printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE2 myBle);
    void clearResources();
};

#endif /* MY_NOTIFICATION_HPP */