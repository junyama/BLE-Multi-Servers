#ifndef MY_CLIENT_HPP
#define MY_CLIENT_HPP

#include <NimBLEDevice.h>
#include "MyLog.hpp"
#include "MyBLE2.hpp"
#include "MyThermo.hpp"
#include "MyGetIndex.hpp"
#include "MyScanCallbacks.hpp"
#include "MyM5.hpp"

class MyClientCallbacks : public NimBLEClientCallbacks
{
private:
    const char *TAG = "MyClientCallbacks";
    //MyBLE2 *myBleArr;
    //MyThermo *myThermoArr;

    MyScanCallbacks *myScanCallbacks;
    MyM5 *myM5;

public:
    int numberOfConnectedBMS = 0;
    int numberOfConnectedThermo = 0;

    MyClientCallbacks(MyScanCallbacks *myScanCallbacks_, MyM5 *myM5_);

    void onConnect(NimBLEClient *pClient) override;

    void onDisconnect(NimBLEClient *pClient, int reason) override;

    /********************* Security handled here *********************/
    void onPassKeyEntry(NimBLEConnInfo &connInfo) override;

    void onConfirmPasskey(NimBLEConnInfo &connInfo, uint32_t pass_key) override;

    /** Pairing process complete, we can check the results in connInfo */
    void onAuthenticationComplete(NimBLEConnInfo &connInfo) override;
};

#endif /* MY_CLIENT_HPP_ */
