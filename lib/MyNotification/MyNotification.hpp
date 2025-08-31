#ifndef MY_NOTIFICATION_HPP
#define MY_NOTIFICATION_HPP

#include <NimBLEDevice.h>
#include "MyBLE2.hpp"
#include "MyThermo.hpp"

#include "MyScanCallbacks.hpp"
#include "MyClientCallbacks.hpp"
#include "MyLog.hpp"
//#include "MyTimer.hpp"

class MyNotification
{
private:
    const char *TAG = "MyNotification";
    MyBLE2 *myBleArr;
    MyThermo *myThermoArr;

    //MyTimer *myTimerArr;
    MyClientCallbacks *myClientCallbacks;
    MyScanCallbacks *myScanCallbacks;

public:
    int numberOfConnectedBMS = 0;
    int numberOfConnectedThermo = 0;

    MyNotification(MyBLE2 *myBleArr_, MyScanCallbacks *myScanCallbacks_, MyClientCallbacks *myClientCallbacks, MyThermo *myThermoArr_);
    /** Notification / Indication receiving handler callback */
    void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    int getIndexOfMyBleArr(NimBLEClient *client);
    int getIndexOfMyThermoArr(NimBLEClient *client);
    /** Handles the provisioning of clients and connects / interfaces with the server */
    bool connectToServer();
    bool connectToThermo();
    //void printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE2 myBle);
    void clear();
};

#endif /* MY_NOTIFICATION_HPP */