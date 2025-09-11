#include "MyScanCallbacks.hpp"

MyScanCallbacks::MyScanCallbacks()
{
}

MyScanCallbacks::MyScanCallbacks(MyBLE2 *myBleArr_, MyM5 *myM5_, MyThermo *myThermoArr_)
    : myBleArr(myBleArr_), myM5(myM5_), myThermoArr(myThermoArr_)
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
      INFO_PRINT("onResult> push_back add> advDevices.size(): %d\n", advDevices.size());
    }
    return;
  }
  // else if (advertisedDevice->isAdvertisingService(serviceUUID_thermo))
  // else if (advertisedDevice->getAddress().equals(targeThermoAddress)) // tentative code2 to mach mac address
  if (advertisedDevice->getServiceDataUUID(0).equals(serviceDataUUID_thermo))
  {
    DEBUG4_PRINT("[%d]: Found Thermomater Service, Name: %s, Address: %s\n",
                 advThermoDevices.size(),
                 advertisedDevice->getName().c_str(),
                 advertisedDevice->getAddress().toString().c_str());
    DEBUG5_PRINT("DEBUG6\n");
    if (advThermoDevices.size() == 0)
    {
      DEBUG5_PRINT("DEBUG7\n");
      advThermoDevices.push_back(advertisedDevice);
      DEBUG5_PRINT("DEBUG8\n");
      DEBUG5_PRINT("onResult> first push_back> advThermoDevices.size(): %d\n", advThermoDevices.size());
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
    DEBUG5_PRINT("DEBUG9\n");
    return;
  }
  DEBUG_PRINT("Advertized device is not BSM or Thermomater: %s\n", advertisedDevice->toString().c_str());
}

/** Callback to process the results of the completed scan or restart it */
void MyScanCallbacks::onScanEnd(const NimBLEScanResults &results, int reason)
{
  numberOfBMS = advDevices.size();
  INFO_PRINT("BMS scan done, reason: %d, device count: %d, numberOfBMS: %d\n",
             reason, results.getCount(), numberOfBMS);
  // numberOfAdvDevices = advDevices.size();
  // DEBUG_PRINT("advDevices.size(): %d\n", advDevices.size());
  //*numberOfDevicesFound *= numberOfAdvDevices;
  // DEBUG_PRINT("numberOfDevicesFound: %d\n", *numberOfDevicesFound);
  char buff[256];
  sprintf(buff, "BMS scan done, found %d", numberOfBMS);
  myM5->println(String(buff));

  if (numberOfBMS == 0)
  {
    WARN_PRINT("no BMS found\n");
    // return;
    // WARN_PRINT("no BMS found and goint to rescan BLE\n");
    //   doRescan = true;
    //   delay(2000);
    //   myM5->reset();
  }

  for (int bleIndex = 0; bleIndex < numberOfBMS; bleIndex++)
  {
    MyBLE2 myBle(advDevices[bleIndex]->getAddress(), String(advDevices[bleIndex]->getName().c_str()));
    bleDevices.push_back(myBle);
    DEBUG_PRINT("[%d]: bleDevices.push_back: %s\n", bleIndex, bleDevices[bleIndex].deviceName);
  }

  if (numberOfBMS != 0 && !doConnect)
  {
    doConnect = true;
  }
  DEBUG5_PRINT("D4\n");
  // added for Thermomater
  // numberOfThermo = advThermoDevices.size(); ////////////////
  DEBUG5_PRINT("D5\n");
  INFO_PRINT("Thermomater scan done, reason: %d, device count: %d, advThermoDevices.size(): %d\n",
             reason, results.getCount(), advThermoDevices.size());
  // numberOfAdvThermoDevices = advThermoDevices.size();
  // DEBUG_PRINT("advThermoDevices.size(): %d\n", advThermoDevices.size());
  // numberOfThermoDevicesFound = &numberOfAdvThermoDevices;
  // DEBUG_PRINT("numberOfThermoDevicesFound: %d\n", *numberOfThermoDevicesFound);
  sprintf(buff, "Thermo scan done, found %d", advThermoDevices.size());
  myM5->println(String(buff));

  if (advThermoDevices.size() == 0)
  {
    WARN_PRINT("no thermomater found\n");
    return;
    // WARN_PRINT("no thermomater found and going to rescan BLE\n");
    // doRescan = true;
  }

  for (int index = 0; index < advThermoDevices.size(); index++)
  {
    /*
    std::string address = advThermoDevices[index]->getAddress().toString();
    thermoDevices[index].mac = String(address.c_str());
    // myM5->bmsInfoArr[bleIndex].mac = bleDevices[index].mac;
    DEBUG2_PRINT("thermoDevices[%d].mac set by %s\n", index, address.c_str());
    std::string deviceName = advThermoDevices[index]->getName();
    thermoDevices[index].deviceName = String(deviceName.c_str());
    // myM5->bmsInfoArr[index].deviceName = bleDevices[index].deviceName;
    DEBUG2_PRINT("thermoDevices[%d].deviceName set by %s\n", index, thermoDevices[index].deviceName.c_str());
    */
    MyThermo myThermo(advThermoDevices[index]->getAddress(), String(advThermoDevices[index]->getName().c_str()));
    thermoDevices.push_back(myThermo);
    WARN_PRINT("[%d/%d]: thermoDevices.push_back: %s\n", index, advThermoDevices.size(), thermoDevices[index].deviceName);
  }
  if (advThermoDevices.size() != 0 && !doConnectThermo)
  {
    doConnectThermo = true;
  }
}
