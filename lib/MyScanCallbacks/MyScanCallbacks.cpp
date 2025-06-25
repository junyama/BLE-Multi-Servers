#include "MyScanCallbacks.hpp"

MyScanCallbacks::MyScanCallbacks()
{
}

MyScanCallbacks::MyScanCallbacks(MyBLE2 *myBleArr_, MyLcd2 *myLcd_, int *numberOfDevicesFound_)
    : myBleArr(myBleArr_), myLcd(myLcd_), numberOfDevicesFound(numberOfDevicesFound_)
{
}

void MyScanCallbacks::onResult(const NimBLEAdvertisedDevice *advertisedDevice)
{
  DEBUG_PRINT("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
  if (advertisedDevice->isAdvertisingService(serviceUUID))
  {
    DEBUG_PRINT("Found Our Service\n");
    /** stop scan before connecting */
    // NimBLEDevice::getScan()->stop(); //Jun: comment out
    /** Save the device reference in a global for the client to use*/
    // advDevice = advertisedDevice;
    /** Ready to connect now */
    // doConnect = true;

    //  Jun: added BEGIN
    if (advDevices.size() == 0)
    {
      // advDevice = advertisedDevice;
      advDevices.push_back(advertisedDevice);
      DEBUG_PRINT("onResult:advDevices.size()=%d\n", advDevices.size());
    }
    else
    {
      for (int i = 0; i < advDevices.size(); i++)
      {
        if (advDevices.at(i)->getAddress().equals(advertisedDevice->getAddress()))
        {
          DEBUG_PRINT("onResult:device already added\n");
          return;
        }
      }
      // advDevice = advertisedDevice;
      advDevices.push_back(advertisedDevice);
      DEBUG_PRINT("onResult:advDevices.size(): %d\n", advDevices.size());
    }
    // END
  }
}

/** Callback to process the results of the completed scan or restart it */
void MyScanCallbacks::onScanEnd(const NimBLEScanResults &results, int reason)
{
  DEBUG_PRINT("Scan Ended, reason: %d, device count: %d, numberOfAdvDevices: %d\n", reason, results.getCount(), advDevices.size());
  numberOfAdvDevices = advDevices.size();
  DEBUG_PRINT("numberOfAdvDevices: %d\n", numberOfAdvDevices);
  *numberOfDevicesFound *= numberOfAdvDevices;
  DEBUG_PRINT("numberOfDevicesFound: %d\n", *numberOfDevicesFound);
  char buff[256];
  sprintf(buff, "BLE scan done, rc: %d, device found: %d/%d", reason, numberOfAdvDevices, results.getCount());
  myLcd->println(String(buff));
  for (int bleIndex = 0; bleIndex < numberOfAdvDevices; bleIndex++)
  {
    std::string address = advDevices[bleIndex]->getAddress().toString();
    myBleArr[bleIndex].mac = String(address.c_str());
    myLcd->bmsInfoArr[bleIndex].mac = myBleArr[bleIndex].mac;
    DEBUG_PRINT("myBleArr[%d].mac set by %s\n", bleIndex, address.c_str());
    std::string deviceName = advDevices[bleIndex]->getName();
    myBleArr[bleIndex].deviceName = String(deviceName.c_str());
    myLcd->bmsInfoArr[bleIndex].deviceName = myBleArr[bleIndex].deviceName;
    DEBUG_PRINT("myBleArr[%d].deviceName set by %s\n", bleIndex, myBleArr[bleIndex].deviceName.c_str());
    // bleDevices->push_back(MyBLE(advDevices[bleIndex]->getAddress())); //crash
  }
  // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
  if (advDevices.size() != 0 && !doConnect)
  {
    // NimBLEDevice::getScan()->stop();
    doConnect = true;
  }
}
