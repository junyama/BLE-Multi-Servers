#include <NimBLEDevice.h>
#include "MyLog.cpp"
#include "MyBLE.cpp"
#include "MyBLE.cpp"


class MyClientCallbacks : public NimBLEClientCallbacks
{
private:
    const char *TAG = "MyClientCallbacks";

public:
    MyBLE *myBleArr;
    std::vector<MyBLE> *bleDevices;
    int numberOfAdvDevices;

    MyClientCallbacks(MyBLE *myBleArr_, std::vector<MyBLE> *bleDevices_) : myBleArr(myBleArr_), bleDevices(bleDevices_)
    {
    }

    int getIndexOfMyBleArr(NimBLEClient *pClient)
    {
        String peerAddress = String(pClient->getPeerAddress().toString().c_str());
        DEBUG_PRINT("numberOfAdvDevices = %d\n", numberOfAdvDevices);
        for (int i = 0; i < numberOfAdvDevices; i++)
        {
            String address = myBleArr[i].mac;
            DEBUG_PRINT("myBleArr[%d].mac = %s\n", i, address.c_str());
            if (peerAddress.equals(address))
            {
                return i;
            }
        }
        return 0;
    }

    void onConnect(NimBLEClient *pClient) override
    {
        DEBUG_PRINT("Connected to %s\n", pClient->getPeerAddress().toString().c_str());
        int index = getIndexOfMyBleArr(pClient);
        myBleArr[index].connected = true;
    }

    void onDisconnect(NimBLEClient *pClient, int reason) override
    {
        //myBleArr[getIndexOfMyBleArr(pClient)].connected = false;
        DEBUG_PRINT("Disconnected from %s, reason = %d\n", pClient->getPeerAddress().toString().c_str(), reason);
        int index = getIndexOfMyBleArr(pClient);
        myBleArr[index].connected = false;
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