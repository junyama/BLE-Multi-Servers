#ifndef MY_NOTIFICATION_HPP
#define MY_NOTIFICATION_HPP

#include <NimBLEDevice.h>
#include "MyBLE2.hpp"
#include "MyScanCallbacks.hpp"
#include "MyClientCallbacks.hpp"
#include "MyLog.cpp"
#include "MyTimer.hpp"

class MyNotification
{
private:
    const char *TAG = "MyNotification";
    MyBLE2 *myBleArr;
    MyTimer *myTimerArr;
    MyClientCallbacks *myClientCallbacks;
    MyScanCallbacks *myScanCallbacks;

public:
    MyNotification(MyBLE2 *myBleArr_, MyTimer *myTimerArr_, MyScanCallbacks *myScanCallbacks_, MyClientCallbacks *myClientCallbacks);
    /** Notification / Indication receiving handler callback */
    void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
    int getIndexOfMyBleArr(NimBLEClient *client);
    /** Handles the provisioning of clients and connects / interfaces with the server */
    bool connectToServer();
    void printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE2 myBle);
};

#endif /* MY_NOTIFICATION_HPP */