#include "MyGetIndex.hpp"

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

int MyGetIndex::bm6Devices(std::vector<MyBm6> *bm6Devices, NimBLEClient *client)
{
  auto peerAddress = client->getPeerAddress();
  DEBUG_PRINT("client peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < bm6Devices->size(); index++)
  {
    DEBUG_PRINT("bm6Devices[%d].mac: %s\n", index, bm6Devices->at(index).mac.c_str());
    if (peerAddress == bm6Devices->at(index).peerAddress)
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

String MyGetIndex::bm6Info(std::vector<MyBm6> *bm6Devices, int index)
{
  char buff[256];
  sprintf(buff, "bm6Devices[%d], Name: %s, topic: %s",
          index, bm6Devices->at(index).deviceName.c_str(), bm6Devices->at(index).topic.c_str());
  return String(buff);
}