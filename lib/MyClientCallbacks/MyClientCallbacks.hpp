#ifndef MY_CLIENT_HPP
#define MY_CLIENT_HPP

#include <NimBLEDevice.h>
#include "MyLog.cpp"
#include "MyBLE2.hpp"

class MyClientCallbacks : public NimBLEClientCallbacks
{
private:
    const char *TAG = "MyClientCallbacks";
    MyBLE2 *myBleArr;

public:
    int *numberOfBleDevices;

    MyClientCallbacks(MyBLE2 *myBleArr_, int *numberOfBleDevice_);

    int getIndexOfMyBleArr(NimBLEClient *pClient);

    void onConnect(NimBLEClient *pClient) override;

    void onDisconnect(NimBLEClient *pClient, int reason) override;

    /********************* Security handled here *********************/
    void onPassKeyEntry(NimBLEConnInfo &connInfo) override;

    void onConfirmPasskey(NimBLEConnInfo &connInfo, uint32_t pass_key) override;

    /** Pairing process complete, we can check the results in connInfo */
    void onAuthenticationComplete(NimBLEConnInfo &connInfo) override;
};

#endif /* MY_CLIENT_HPP_ */
