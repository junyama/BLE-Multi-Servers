#include "MyClientCallbacks.hpp"

MyClientCallbacks::MyClientCallbacks(MyBLE2 *myBleArr_, MyThermo *myThermoArr_, MyScanCallbacks *myScanCallbacks_)
    : myBleArr(myBleArr_), myThermoArr(myThermoArr_), myScanCallbacks(myScanCallbacks_)
{
}

void MyClientCallbacks::onConnect(NimBLEClient *pClient)
{
    //auto bleDevices = myScanCallbacks->bleDevices;
    //auto thermoDevices = myScanCallbacks->thermoDevices;
    DEBUG4_PRINT("Connected to %s\n", pClient->getPeerAddress().toString().c_str());
    int index = MyGetIndex::bleDevices(myScanCallbacks, pClient);
    if (index > -1)
    {
        myScanCallbacks->bleDevices[index].connected = true; ///////////
        DEBUG4_PRINT("bleDevices[%d] Connected\n", index);
    }
    else
    {
        index = MyGetIndex::thermoDevices(myScanCallbacks, pClient);
        if (index > -1)
        {
            myScanCallbacks->thermoDevices[index].connected = true; //////////
            DEBUG4_PRINT("thermoDevices[%d] Connected\n", index);
        }
        else
        {
            ERROR_PRINT("Connected event from unkown Address: %s\n", pClient->getPeerAddress().toString().c_str());
        }
    }
}

void MyClientCallbacks::onDisconnect(NimBLEClient *pClient, int reason)
{
    //auto bleDevices = myScanCallbacks->bleDevices;
    //auto thermoDevices = myScanCallbacks->thermoDevices;
    DEBUG_PRINT("Disconnected from %s, reason = %d\n", pClient->getPeerAddress().toString().c_str(), reason);
    int index = MyGetIndex::bleDevices(myScanCallbacks, pClient);
    if (index > -1)
    {
        myScanCallbacks->bleDevices[index].connected = false; //////////
        WARN_PRINT("Disconnected from %s\n", MyGetIndex::bleInfo(myScanCallbacks, index).c_str());
    }
    else
    {
        index = MyGetIndex::thermoDevices(myScanCallbacks, pClient);
        if (index > -1)
        {
            myScanCallbacks->thermoDevices[index].connected = false; ///////////
            WARN_PRINT("Disconnected from %s\n", MyGetIndex::thermoInfo(myScanCallbacks, index).c_str());
        }
        else
        {
            ERROR_PRINT("Disconnected event from unkown Address: %s\n", pClient->getPeerAddress().toString().c_str());
        }
    }
}

/********************* Security handled here *********************/
void MyClientCallbacks::onPassKeyEntry(NimBLEConnInfo &connInfo)
{
    DEBUG_PRINT("Server Passkey Entry\n");
    /**
     * This should prompt the user to enter the passkey displayed
     * on the peer device.
     */
    NimBLEDevice::injectPassKey(connInfo, 123456);
}

void MyClientCallbacks::onConfirmPasskey(NimBLEConnInfo &connInfo, uint32_t pass_key)
{
    DEBUG_PRINT("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
    /** Inject false if passkeys don't match. */
    NimBLEDevice::injectConfirmPasskey(connInfo, true);
}

/** Pairing process complete, we can check the results in connInfo */
void MyClientCallbacks::onAuthenticationComplete(NimBLEConnInfo &connInfo)
{
    if (!connInfo.isEncrypted())
    {
        DEBUG_PRINT("Encrypt connection failed - disconnecting\n");
        /** Find the client with the connection handle provided in connInfo */
        NimBLEDevice::getClientByHandle(connInfo.getConnHandle())->disconnect();
        return;
    }
}
