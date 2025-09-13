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

int MyGetIndex::bleDevices(std::vector<MyBLE2> *bleDevices, NimBLEClient *client)
{
  auto peerAddress = client->getPeerAddress();
  DEBUG_PRINT("client peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < bleDevices->size(); index++)
  {
    DEBUG_PRINT("bleDevices[%d].mac: %s\n", index, bleDevices->at(index).mac.c_str());
    if (peerAddress == bleDevices->at(index).peerAddress)
    {
      return index;
    }
  }
  return -1;
}

int MyGetIndex::thermoDevices(std::vector<MyThermo> *thermoDevices, NimBLEClient *client)
{
  auto peerAddress = client->getPeerAddress();
  DEBUG_PRINT("client peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < thermoDevices->size(); index++)
  {
    DEBUG_PRINT("thermoDevices[%d].mac: %s\n", index, thermoDevices->at(index).mac.c_str());
    if (peerAddress == thermoDevices->at(index).peerAddress)
    {
      return index;
    }
  }
  return -1;
}

String MyGetIndex::bleInfo(std::vector<MyBLE2> *bleDevices, int index)
{
  char buff[256];
  sprintf(buff, "bleDevices[%d], Name: %s, topic: %s",
          index, bleDevices->at(index).deviceName.c_str(), bleDevices->at(index).topic.c_str());
  return String(buff);
}

String MyGetIndex::thermoInfo(std::vector<MyThermo> *thermoDevices, int index)
{
  char buff[256];
  sprintf(buff, "thermoDevices[%d], Name: %s, topic: %s",
          index, thermoDevices->at(index).deviceName.c_str(), thermoDevices->at(index).topic.c_str());
  return String(buff);
}