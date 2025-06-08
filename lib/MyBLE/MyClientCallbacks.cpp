#include <NimBLEDevice.h>
#include "MyLog.cpp"
#include "MyBLE.cpp"

class MyClientCallbacks : public NimBLEClientCallbacks
{
private:
    const char *TAG = "MyClientCallbacks";

public:
    MyBLE *myBleArr;
    int numberOfAdvDevices;
    bool *BLE_client_connected[3];

    MyClientCallbacks(MyBLE *myBleArr_) : myBleArr(myBleArr_)
    {
    }

    int getIndexOfMyBleArr(NimBLEClient *client)
    {
        auto peerAddress = client->getPeerAddress();
        for (int i = 0; i < numberOfAdvDevices; i++)
        {
            if (peerAddress == myBleArr[i].pChr_rx->getClient()->getPeerAddress())
            {
                return i;
            }
        }
        return 0;
    }

    void onConnect(NimBLEClient *pClient) override
    {
        // BLE_client_connected = true;
        DEBUG_PRINT("Connected\n");
    }

    void onDisconnect(NimBLEClient *pClient, int reason) override
    {
        // BLE_client_connected = false;
        DEBUG_PRINT("%s Disconnected, reason = %d - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
        // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }

    /********************* Security handled here *********************/
    void onPassKeyEntry(NimBLEConnInfo &connInfo) override
    {
        DEBUG_PRINT("Server Passkey Entry\n");
        /**
         * This should prompt the user to enter the passkey displayed
         * on the peer device.
         */
        NimBLEDevice::injectPassKey(connInfo, 123456);
    }

    void onConfirmPasskey(NimBLEConnInfo &connInfo, uint32_t pass_key) override
    {
        DEBUG_PRINT("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }

    /** Pairing process complete, we can check the results in connInfo */
    void onAuthenticationComplete(NimBLEConnInfo &connInfo) override
    {
        if (!connInfo.isEncrypted())
        {
            DEBUG_PRINT("Encrypt connection failed - disconnecting\n");
            /** Find the client with the connection handle provided in connInfo */
            NimBLEDevice::getClientByHandle(connInfo.getConnHandle())->disconnect();
            return;
        }
    }
};