#include "MyGetIndex.hpp"

/*
MyGetIndex::MyGetIndex(MyBLE2 *myBleArr_, MyThermo *myThermoArr_, MyScanCallbacks *myScanCallbacks_)
    : myBleArr(myBleArr_),  myThermoArr(myThermoArr_), myScanCallbacks(myScanCallbacks_)
{
  DEBUG_PRINT("an instance created\n");
}
*/

int MyGetIndex::myBleArr(MyBLE2 *myBleArr, NimBLEClient *client)
{
  DEBUG_PRINT("myBleArr() called, BLE_ARR_SIZE: %d\n", BLE_ARR_SIZE);
  auto peerAddress = client->getPeerAddress();
  DEBUG_PRINT("myBleArr(), peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < BLE_ARR_SIZE; index++)
  {
    DEBUG_PRINT("myBleArr(), myBleArr[%d].mac: %s\n", index, myBleArr[index].mac.c_str());
    std::string macStr = myBleArr[index].mac.c_str();
    if (peerAddress == NimBLEAddress(macStr, 0))
    {
      return index;
    }
  }
  return -1;
}

int MyGetIndex::myThermoArr(MyThermo *myThermoArr, NimBLEClient *client)
{
  DEBUG_PRINT("myThermoArr() called, THERMO_ARR_SIZE: %d\n", THERMO_ARR_SIZE);
  auto peerAddress = client->getPeerAddress();
  DEBUG_PRINT("myThermoArr(), peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < THERMO_ARR_SIZE; index++)
  {
    DEBUG_PRINT("myThermoArr(), myThermoArr[%d].mac: %s\n", index, myThermoArr[index].mac.c_str());
    std::string macStr = myThermoArr[index].mac.c_str();
    if (peerAddress == NimBLEAddress(macStr, 0))
    {
      return index;
    }
  }
  return -1;
}