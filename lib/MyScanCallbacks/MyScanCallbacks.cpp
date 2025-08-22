#include "MyScanCallbacks.hpp"

MyScanCallbacks::MyScanCallbacks()
{
}

MyScanCallbacks::MyScanCallbacks(MyBLE2 *myBleArr_, MyM5 *myM5_, int *numberOfDevicesFound_, MyThermo *myThermoArr_, int *numberOfThermoDevicesFound_)
    : myBleArr(myBleArr_), myM5(myM5_), numberOfDevicesFound(numberOfDevicesFound_), myThermoArr(myThermoArr_), numberOfThermoDevicesFound(numberOfThermoDevicesFound_)
{
}

void MyScanCallbacks::onResult(const NimBLEAdvertisedDevice *advertisedDevice)
{
  //NimBLEAddress targeThermoAddress("a4:c1:38:f8:21:63", BLE_ADDR_PUBLIC); // tentative code1 to match mac address
  DEBUG_PRINT("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
  if (advertisedDevice->isAdvertisingService(serviceUUID))
  {
    DEBUG_PRINT("Found BMS Service\n");
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
  else if (advertisedDevice->isAdvertisingService(serviceUUID_thermo))
  //else if (advertisedDevice->getAddress().equals(targeThermoAddress)) // tentative code2 to mach mac address
  {
    DEBUG_PRINT("Found Thermomater Service\n");
    advThermoDevices.push_back(advertisedDevice);
    DEBUG_PRINT("onResult:advThermoDevices.size(): %d\n", advThermoDevices.size());
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
  myM5->println(String(buff));

  if (*numberOfDevicesFound == 0)
  {
    DEBUG_PRINT("no device found, going to reset\n");
    delay(2000);
    myM5->reset();
  }

  for (int bleIndex = 0; bleIndex < numberOfAdvDevices; bleIndex++)
  {
    std::string address = advDevices[bleIndex]->getAddress().toString();
    myBleArr[bleIndex].mac = String(address.c_str());
    myM5->bmsInfoArr[bleIndex].mac = myBleArr[bleIndex].mac;
    DEBUG_PRINT("myBleArr[%d].mac set by %s\n", bleIndex, address.c_str());
    std::string deviceName = advDevices[bleIndex]->getName();
    myBleArr[bleIndex].deviceName = String(deviceName.c_str());
    myM5->bmsInfoArr[bleIndex].deviceName = myBleArr[bleIndex].deviceName;
    DEBUG_PRINT("myBleArr[%d].deviceName set by %s\n", bleIndex, myBleArr[bleIndex].deviceName.c_str());
    // bleDevices->push_back(MyBLE(advDevices[bleIndex]->getAddress())); //crash
  }
  // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
  if (advDevices.size() != 0 && !doConnect)
  {
    // NimBLEDevice::getScan()->stop();
    doConnect = true;
  }

  // added for Thermomater
  DEBUG_PRINT("Scan Ended, reason: %d, device count: %d, numberOfAdvThermoDevices: %d\n", reason, results.getCount(), advThermoDevices.size());
  numberOfAdvThermoDevices = advThermoDevices.size();
  DEBUG_PRINT("numberOfAdvThermoDevices: %d\n", numberOfAdvThermoDevices);
  *numberOfThermoDevicesFound *= numberOfAdvThermoDevices;
  DEBUG_PRINT("numberOfThermoDevicesFound: %d\n", *numberOfThermoDevicesFound);
  sprintf(buff, "Thermomater scan done, rc: %d, thermomater found: %d/%d", reason, numberOfAdvThermoDevices, results.getCount());
  myM5->println(String(buff));
  
  for (int thermoIndex = 0; thermoIndex < numberOfAdvThermoDevices; thermoIndex++)
  {
    std::string address = advThermoDevices[thermoIndex]->getAddress().toString();
    myThermoArr[thermoIndex].mac = String(address.c_str());
    //myM5->bmsInfoArr[bleIndex].mac = myBleArr[thermoIndex].mac;
    DEBUG_PRINT("myThermoArr[%d].mac set by %s\n", thermoIndex, address.c_str());
    std::string deviceName = advThermoDevices[thermoIndex]->getName();
    myThermoArr[thermoIndex].deviceName = String(deviceName.c_str());
    //myM5->bmsInfoArr[thermoIndex].deviceName = myBleArr[thermoIndex].deviceName;
    DEBUG_PRINT("myThermoArr[%d].deviceName set by %s\n", thermoIndex, myThermoArr[thermoIndex].deviceName.c_str());
  }
  if (advThermoDevices.size() != 0 && !doConnectThermo)
  {
    doConnectThermo = true;
  }
}
