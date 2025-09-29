#include "MyScanCallbacks.hpp"

MyScanCallbacks::MyScanCallbacks()
{
}

MyScanCallbacks::MyScanCallbacks(MyM5 *myM5_)
    : myM5(myM5_)
{
}

void MyScanCallbacks::onResult(const NimBLEAdvertisedDevice *advertisedDevice)
{
  // NimBLEAddress targeThermoAddress("a4:c1:38:f8:21:63", BLE_ADDR_PUBLIC); // tentative code1 to match mac address
  DEBUG_PRINT("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
  DEBUG_PRINT("Advertised Device ServiceData UUID: %s\n", advertisedDevice->getServiceDataUUID(0).toString().c_str());
  if (advertisedDevice->isAdvertisingService(serviceUUID))
  {
    // DEBUG4_PRINT("Found BMS Service: %s\n", advertisedDevice->toString().c_str());
    DEBUG4_PRINT("Found BMS Service, Name: %s, Address: %s\n",
                 advertisedDevice->getName().c_str(),
                 advertisedDevice->getAddress().toString().c_str());
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
      DEBUG_PRINT("onResult> first push_back> advDevices.size(): %d\n", advDevices.size());
    }
    else
    {
      for (int i = 0; i < advDevices.size(); i++)
      {
        if (advDevices.at(i)->getAddress().equals(advertisedDevice->getAddress()))
        {
          DEBUG_PRINT("onResult> BMS device already added\n");
          return;
        }
      }
      // advDevice = advertisedDevice;
      advDevices.push_back(advertisedDevice);
      DEBUG_PRINT("onResult> push_back add> advDevices.size(): %d\n", advDevices.size());
    }
    return;
  }
  // else if (advertisedDevice->isAdvertisingService(serviceUUID_thermo))
  // else if (advertisedDevice->getAddress().equals(targeThermoAddress)) // tentative code2 to mach mac address
  if (advertisedDevice->getServiceDataUUID(0).equals(serviceDataUUID_thermo))
  {
    DEBUG4_PRINT("Found Thermomater Service, Name: %s, Address: %s\n",
                 advertisedDevice->getName().c_str(),
                 advertisedDevice->getAddress().toString().c_str());
    if (advThermoDevices.size() == 0)
    {
      advThermoDevices.push_back(advertisedDevice);
      DEBUG_PRINT("onResult> first push_back> advThermoDevices.size(): %d\n", advThermoDevices.size());
    }
    else
    {
      for (int index = 0; index < advThermoDevices.size(); index++)
      {
        if (advThermoDevices.at(index)->getAddress().equals(advertisedDevice->getAddress()))
        {
          DEBUG_PRINT("onResult>Thermo device already added\n");
          return;
        }
      }
      advThermoDevices.push_back(advertisedDevice);
      DEBUG_PRINT("onResult> push_back add> advThermoDevices.size(): %d\n", advThermoDevices.size());
    }
    return;
  }

  if (advertisedDevice->getName() == deviceName_bm6)
  {
    DEBUG4_PRINT("Found BM6 device, Name: %s, Address: %s\n",
                 advertisedDevice->getName().c_str(),
                 advertisedDevice->getAddress().toString().c_str());
    if (advBm6Devices.size() == 0)
    {
      advBm6Devices.push_back(advertisedDevice);
      DEBUG_PRINT("onResult> first push_back> advBm6Devices.size(): %d\n", advBm6Devices.size());
    }
    else
    {
      for (int index = 0; index < advBm6Devices.size(); index++)
      {
        if (advBm6Devices.at(index)->getAddress().equals(advertisedDevice->getAddress()))
        {
          DEBUG_PRINT("onResult>BM6 device already added\n");
          return;
        }
      }
      advBm6Devices.push_back(advertisedDevice);
      DEBUG_PRINT("onResult> push_back add> advBm6Devices.size(): %d\n", advBm6Devices.size());
    }
    return;
  }

  DEBUG_PRINT("Advertized device is not BSM or Thermomater or BM6: %s\n", advertisedDevice->toString().c_str());
}

/** Callback to process the results of the completed scan or restart it */
void MyScanCallbacks::onScanEnd(const NimBLEScanResults &results, int reason)
{
  INFO_PRINT("increament scan count: %d\n", ++myM5->numberOfScan);
  // numberOfBMS = advDevices.size();
  INFO_PRINT("BMS scan done, reason: %d, device count: %d, advDevices.size(): %d\n",
             reason, results.getCount(), advDevices.size());
  // numberOfAdvDevices = advDevices.size();
  // DEBUG_PRINT("advDevices.size(): %d\n", advDevices.size());
  //*numberOfDevicesFound *= numberOfAdvDevices;
  // DEBUG_PRINT("numberOfDevicesFound: %d\n", *numberOfDevicesFound);
  char buff[256];
  sprintf(buff, "BMS scan done, found %d", advDevices.size());
  myM5->println(String(buff));

  if (advDevices.size() == 0)
  {
    WARN_PRINT("no BMS found\n");
    return;
    // WARN_PRINT("no BMS found and goint to rescan BLE\n");
    //  doRescan = true;
    //  delay(2000);
    //  myM5->reset();
  }

  bleDevices.clear();
  myM5->bmsInfoVec.clear();
  for (int bleIndex = 0; bleIndex < advDevices.size(); bleIndex++)
  {
    bleDevices.emplace_back(advDevices[bleIndex]->getAddress(), String(advDevices[bleIndex]->getName().c_str()));
    DEBUG4_PRINT("bleDevices[%d] created with Name: %s, Address: %s\n",
                 index, bleDevices[bleIndex].deviceName.c_str(), bleDevices[bleIndex].mac.c_str());

    /*
    myM5->bmsInfoArr[bleIndex].mac = bleDevices[bleIndex].mac;
    myM5->bmsInfoArr[bleIndex].deviceName = bleDevices[bleIndex].deviceName;
    */

    myM5->createBmsInfoVec(bleDevices[bleIndex].deviceName, bleDevices[bleIndex].mac);
  }
  // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
  if (advDevices.size() != 0 && !doConnect)
  {
    doConnect = true;
  }

  // added for Thermomater
  // numberOfThermo = advThermoDevices.size(); ////////////////

  INFO_PRINT("Thermomater scan done, reason: %d, device count: %d, advThermoDevices.size(): %d\n",
             reason, results.getCount(), advThermoDevices.size());
  // numberOfAdvThermoDevices = advThermoDevices.size();
  // DEBUG_PRINT("advThermoDevices.size(): %d\n", advThermoDevices.size());
  // advThermoDevices.size()DevicesFound = &numberOfAdvThermoDevices;
  // DEBUG_PRINT("advThermoDevices.size()DevicesFound: %d\n", *advThermoDevices.size()DevicesFound);
  sprintf(buff, "Thermo scan done, found %d", advThermoDevices.size());
  myM5->println(String(buff));

  if (advThermoDevices.size() == 0)
  {
    WARN_PRINT("no thermomater found\n");
    return;
    // WARN_PRINT("no thermomater found and going to rescan BLE\n");
    // doRescan = true;
  }

  thermoDevices.clear();
  for (int index = 0; index < advThermoDevices.size(); index++)
  {
    thermoDevices.emplace_back(advThermoDevices[index]->getAddress(), String(advThermoDevices[index]->getName().c_str()));
    DEBUG4_PRINT("thermoDevices[%d] created with Name: %s, Address: %s\n",
                 index, thermoDevices[index].deviceName.c_str(), thermoDevices[index].mac.c_str());
    myM5->createThermoInfoVec(thermoDevices[index].deviceName, thermoDevices[index].mac);
  }
  if (advThermoDevices.size() != 0 && !doConnectThermo)
  {
    doConnectThermo = true;
  }

  INFO_PRINT("BM6 scan done, reason: %d, device count: %d, advBm6Devices.size(): %d\n",
             reason, results.getCount(), advBm6Devices.size());
  sprintf(buff, "BM6 scan done, found %d", advBm6Devices.size());
  myM5->println(String(buff));

  if (advBm6Devices.size() == 0)
  {
    WARN_PRINT("no BM6 found\n");
    return;
  }

  bm6Devices.clear();
  for (int index = 0; index < advBm6Devices.size(); index++)
  {
    bm6Devices.emplace_back(advBm6Devices[index]->getAddress(), String(advBm6Devices[index]->getName().c_str()));
    DEBUG4_PRINT("bm6Devices[%d] created with Name: %s, Address: %s\n",
                 index, bm6Devices[index].deviceName.c_str(), bm6Devices[index].mac.c_str());
    //myM5->createBm6InfoVec(bm6Devices[index].deviceName, bm6Devices[index].mac);
  }
  if (advBm6Devices.size() != 0 && !doConnectBm6)
  {
    doConnectBm6 = true;
  }
}
