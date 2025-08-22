#include "MyClientCallbacks.hpp"

MyClientCallbacks::MyClientCallbacks(MyBLE2 *myBleArr_, int *numberOfBleDevices_, MyThermo *myThermoArr_, int *numberOfThermoDevices_) 
: myBleArr(myBleArr_), numberOfBleDevices(numberOfBleDevices_), myThermoArr(myThermoArr_), numberOfThermoDevices(numberOfThermoDevices_)
{
}

int MyClientCallbacks::getIndexOfMyBleArr(NimBLEClient *pClient)
{
    String peerAddress = String(pClient->getPeerAddress().toString().c_str());
    DEBUG_PRINT("*numberOfBleDevices:%d\n", *numberOfBleDevices);
    for (int i = 0; i < *numberOfBleDevices; i++)
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

void MyClientCallbacks::onConnect(NimBLEClient *pClient)
{
    DEBUG_PRINT("Connected to %s\n", pClient->getPeerAddress().toString().c_str());
    int index = getIndexOfMyBleArr(pClient);
    myBleArr[index].connected = true;
}

void MyClientCallbacks::onDisconnect(NimBLEClient *pClient, int reason)
{
    // myBleArr[getIndexOfMyBleArr(pClient)].connected = false;
    DEBUG_PRINT("Disconnected from %s, reason = %d\n", pClient->getPeerAddress().toString().c_str(), reason);
    int index = getIndexOfMyBleArr(pClient);
    myBleArr[index].connected = false;
    // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
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
