#include <NimBLEDevice.h>
#include "MyBLE.cpp"

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
int getIndexOfMyBleArr(NimBLEClient *client);
/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer();
void printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE myBle);