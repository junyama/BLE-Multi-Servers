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

String MyGetIndex::bleInfo(MyBLE2 *myBleArr, int index)
{
  char buff[256];
  sprintf(buff, "myBleArr[%d], Name: %s, topic: %s",
          index, myBleArr[index].deviceName.c_str(), myBleArr[index].topic.c_str());
  return String(buff);
}

String MyGetIndex::thermoInfo(MyThermo *myThermoArr, int index)
{
  char buff[256];
  sprintf(buff, "myThermoArr[%d], Name: %s, topic: %s",
          index, myThermoArr[index].deviceName.c_str(), myThermoArr[index].topic.c_str());
  return String(buff);
}

int MyGetIndex::bleDevices(MyScanCallbacks *myScanCallbacks, NimBLEClient *client)
{
  // DEBUG_PRINT("myBleArr() called, BLE_ARR_SIZE: %d\n", BLE_ARR_SIZE);
  auto peerAddress = client->getPeerAddress();
  auto bleDevices = myScanCallbacks->bleDevices;
  DEBUG_PRINT("client peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < bleDevices.size(); index++)
  {
    DEBUG_PRINT("bleDevices[%d].mac: %s\n", index, bleDevices[index].mac.c_str());
    if (peerAddress == bleDevices[index].peerAddress)
    {
      return index;
    }
  }
  return -1;
}

int MyGetIndex::thermoDevices(MyScanCallbacks *myScanCallbacks, NimBLEClient *client)
{
  auto peerAddress = client->getPeerAddress();
  auto thermoDevices = myScanCallbacks->thermoDevices;
  DEBUG_PRINT("client peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < thermoDevices.size(); index++)
  {
    DEBUG_PRINT("thermoDevices[%d].mac: %s\n", index, thermoDevices[index].mac.c_str());
    if (peerAddress == thermoDevices[index].peerAddress)
    {
      return index;
    }
  }
  return -1;
}

String MyGetIndex::bleInfo(MyScanCallbacks *myScanCallbacks, int index)
{
  auto bleDevices = myScanCallbacks->bleDevices;
  char buff[256];
  sprintf(buff, "bleDevices[%d], Name: %s, topic: %s",
          index, bleDevices[index].deviceName.c_str(), bleDevices[index].topic.c_str());
  return String(buff);
}

String MyGetIndex::thermoInfo(MyScanCallbacks *myScanCallbacks, int index)
{
  auto thermoDevices = myScanCallbacks->thermoDevices;
  char buff[256];
  sprintf(buff, "thermoDevices[%d], Name: %s, topic: %s",
          index, thermoDevices[index].deviceName.c_str(), thermoDevices[index].topic.c_str());
  return String(buff);
}