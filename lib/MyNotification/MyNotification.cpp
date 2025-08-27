#include "MyNotification.hpp"

MyNotification::MyNotification(MyBLE2 *myBleArr_, MyTimer *myTimerArr_, MyScanCallbacks *myScanCallbacks_, MyClientCallbacks *myClientCallbacks_, MyThermo *myThermoArr_)
    : myBleArr(myBleArr_), myTimerArr(myTimerArr_), myScanCallbacks(myScanCallbacks_), myClientCallbacks(myClientCallbacks_), myThermoArr(myThermoArr_)
{
  DEBUG_PRINT("an instance created\n");
}
// int getIndexOfMyBleArr(NimBLERemoteCharacteristic *pRemoteCharacteristic)
int MyNotification::getIndexOfMyBleArr(NimBLEClient *client)
{
  auto peerAddress = client->getPeerAddress();
  for (int i = 0; i < myScanCallbacks->numberOfAdvDevices; i++)
  {
    if (peerAddress == myBleArr[i].pChr_rx->getClient()->getPeerAddress())
    {
      return i;
    }
  }
  return -1;
}

int MyNotification::getIndexOfMyThermoArr(NimBLEClient *client)
{
  auto peerAddress = client->getPeerAddress();
  for (int i = 0; i < myScanCallbacks->numberOfAdvThermoDevices; i++)
  {
    // if (peerAddress == myThermoArr[i].pChr_rx_temp->getClient()->getPeerAddress())
    std::string macStr = myThermoArr[i].mac.c_str();
    if (peerAddress == NimBLEAddress(macStr, 0))
    {
      return i;
    }
  }
  return -1;
}

/** Notification / Indication receiving handler callback */
void MyNotification::notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  std::string str = (isNotify == true) ? "Notification" : "Indication";
  str += " from ";
  str += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
  str += ", Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
  str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
  str += ", Value = " + std::string((char *)pData, length);
  DEBUG_PRINT("%s\n", str.c_str());
  int bleIndex = getIndexOfMyBleArr(pRemoteCharacteristic->getClient());
  if (bleIndex > -1)
  {
    DEBUG_PRINT("notification from myBleArr[%d]\n", bleIndex);
    myBleArr[bleIndex].bleCollectPacket((char *)pData, length);
  }
  else
  {
    DEBUG_PRINT("bleIndex: %d, notification from a device other than BMS\n", bleIndex);
    int thermoIndex = getIndexOfMyThermoArr(pRemoteCharacteristic->getClient());
    if (thermoIndex > -1)
    {
      auto remoteCharacteristicUUID = pRemoteCharacteristic->getUUID();
      DEBUG_PRINT("notification from myThermoArr[%d], UUID: %s\n",
                  thermoIndex, remoteCharacteristicUUID.toString().c_str());
      if (remoteCharacteristicUUID.equals(myScanCallbacks->charUUID_thermo_temp))
        myThermoArr[thermoIndex].processTempPacket((char *)pData, length);
      else if (remoteCharacteristicUUID.equals(myScanCallbacks->charUUID_thermo_humid))
        myThermoArr[thermoIndex].processHumidPacket((char *)pData, length);
      else
      {
        DEBUG_PRINT("no matching UUID: %s", remoteCharacteristicUUID.toString().c_str());
        DEBUG_PRINT(" of myThermoArr[%d]\n", thermoIndex);
      }
    }
  }
}

bool MyNotification::connectToServer()
{
  DEBUG_PRINT("connectToServer() called\n");
  NimBLEClient *pClient = nullptr;

  // Serial.printf("advDevices.size() = %d\n", advDevices.size());
  // numberOfAdvDevices = advDevices.size();
  // Serial.printf("numberOfAdvDevices = %d\n", numberOfAdvDevices);

  unsigned long initalMesurementTime = 0;
  // unsigned long initalMesurementTime2 = 0;
  bool atLeastOneConnected = false;
  for (int index = 0; index < myScanCallbacks->numberOfAdvDevices; index++)
  {
    try
    {
      /** Check if we have a client we should reuse first **/
      if (NimBLEDevice::getCreatedClientCount())
      {
        /*
         *  Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(myScanCallbacks->advDevices.at(index)->getAddress());
        if (pClient)
        {
          if (!pClient->connect(myScanCallbacks->advDevices.at(index), false))
          {
            throw std::runtime_error("Reconnect failed\n");
          }
          DEBUG_PRINT("Reconnected client\n");
        }
        else
        {
          /**
           *  We don't already have a client that knows this device,
           *  check for a client that is disconnected that we can use.
           */
          pClient = NimBLEDevice::getDisconnectedClient();
        }
      }

      /** No client to reuse? Create a new one. */
      if (!pClient)
      {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS)
        {
          throw std::runtime_error("Max clients reached - no more connections available\n");
        }

        pClient = NimBLEDevice::createClient();
        DEBUG_PRINT("New client created for BMS\n");

        pClient->setClientCallbacks(myClientCallbacks, false);
        /**
         *  Set initial connection parameters:
         *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
         *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
         *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
         */
        pClient->setConnectionParams(12, 12, 0, 150);

        /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
        pClient->setConnectTimeout(10 * 1000);

        if (!pClient->connect(myScanCallbacks->advDevices.at(index)))
        {
          /** Created a client but failed to connect, don't need to keep it as it has no data */
          NimBLEDevice::deleteClient(pClient);
          myScanCallbacks->advDevices.clear();
          throw std::runtime_error("Failed to connect, deleted client\n");
        }
      }

      if (!pClient->isConnected())
      {
        if (!pClient->connect(myScanCallbacks->advDevices.at(index)))
        {
          throw std::runtime_error("Failed to connect BMS\n");
        }
      }

      DEBUG_PRINT("Connected to BMS[%d/%d] at: %s RSSI: %d\n", index, myScanCallbacks->numberOfAdvDevices,
                  pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

      /** Now we can read/write/subscribe the characteristics of the services we are interested in */
      NimBLERemoteService *pSvc = nullptr;
      // NimBLERemoteCharacteristic *pChr = nullptr;
      // const std::vector<NimBLERemoteCharacteristic *> *pChrs = nullptr;
      // NimBLERemoteDescriptor *pDsc = nullptr;

      pSvc = pClient->getService(myScanCallbacks->serviceUUID);
      if (pSvc)
      {
        //
        // pChr_rx = nullptr;
        // pChrSt.pChr_rx = nullptr;

        // pChr_rx = pSvc->getCharacteristic(charUUID_rx);
        // pChrSt.pChr_rx = pSvc->getCharacteristic(charUUID_rx);
        myBleArr[index].pChr_rx = pSvc->getCharacteristic(myScanCallbacks->charUUID_rx);

        if (!myBleArr[index].pChr_rx)
        {
          throw std::runtime_error("charUUID_rx not found.\n");
        }
        if (myBleArr[index].pChr_rx->canNotify())
        {
          if (!myBleArr[index].pChr_rx->subscribe(true, [this](NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                  { notifyCB(pRemoteCharacteristic, pData, length, isNotify); }))
          {
            pClient->disconnect();
            throw std::runtime_error("subscription failed");
          }
        }
        // pChr_tx = nullptr;
        // pChrSt.pChr_tx = nullptr;
        // pChr_tx = pSvc->getCharacteristic(myScanCallbacks.charUUID_tx);
        // pChrSt.pChr_tx = pSvc->getCharacteristic(myScanCallbacks.charUUID_tx);
        myBleArr[index].pChr_tx = pSvc->getCharacteristic(myScanCallbacks->charUUID_tx);

        if (!myBleArr[index].pChr_tx)
        {
          throw std::runtime_error("charUUID_tx not found.\n");
        }
      }
      else
      {
        throw std::runtime_error("serviceUUID of BMS not found.\n");
      }
      DEBUG_PRINT("Done with an advertized BMS[%d/%d]!\n", index, myScanCallbacks->numberOfAdvDevices);
      atLeastOneConnected = true;
      /*
       myBleArr[i].deviceName = String(myScanCallbacks.advDevices[i]->getName().c_str());
       DEBUG_PRINT("myBleArr[%d].deviceName = %s\n", i, myBleArr[i].deviceName.c_str());
       myBleArr[i].mac = String(myScanCallbacks.advDevices.at(i)->getAddress().toString().c_str());
       DEBUG_PRINT("myBleArr[%d].mac = %s\n", i, myBleArr[i].mac.c_str());
       */
      new (myTimerArr + index) MyTimer(initalMesurementTime, 5000);
      DEBUG_PRINT("myTimerArr[%d].lastMeasurment = %d, measurmentIntervalMs = %d\n", index, myTimerArr[index].lastMeasurment,
                  myTimerArr[index].measurmentIntervalMs);
      initalMesurementTime += 200;
      /*
      new (myTimerArr2 + i) MyTimer(initalMesurementTime2, 10000);
      DEBUG_PRINT("myTimerArr2[%d].lastMeasurment = %d, measurmentIntervalMs = %d\n", i, myTimerArr2[i].lastMeasurment,
                  myTimerArr[i].measurmentIntervalMs);
      initalMesurementTime2 += 1000;
      */
    }
    catch (const std::runtime_error &e)
    {
      DEBUG_PRINT("%s\n", e.what());
      int numberOfDevicesFound = *myScanCallbacks->numberOfDevicesFound;
      *myScanCallbacks->numberOfDevicesFound *= numberOfDevicesFound - 1;
      continue;
    }
  }
  myScanCallbacks->advDevices.clear();
  DEBUG_PRINT("Done with all the advertized BMS[%d]!\n", myScanCallbacks->numberOfAdvDevices);
  return atLeastOneConnected;
}

bool MyNotification::connectToThermo()
{
  DEBUG_PRINT("connectToThermo() called\n");
  NimBLEClient *pClient = nullptr;
  bool atLeastOneConnected = false;
  for (int index = 0; index < myScanCallbacks->numberOfAdvThermoDevices; index++)
  {
    try
    {
      DEBUG_PRINT("Start connecting a thermometer[%d/%d]\n", index, myScanCallbacks->numberOfAdvThermoDevices);
      if (NimBLEDevice::getCreatedClientCount())
      {
        DEBUG_PRINT("NimBLEDevice::getCreatedClientCount(): %d\n", NimBLEDevice::getCreatedClientCount());
        /*
         *  Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(myScanCallbacks->advThermoDevices.at(index)->getAddress());
        if (pClient)
        {
          if (!pClient->connect(myScanCallbacks->advThermoDevices.at(index), false))
          {
            throw std::runtime_error("Reconnect failed\n");
          }
          DEBUG_PRINT("Reconnected client\n");
        }
        else
        {
          /**
           *  We don't already have a client that knows this device,
           *  check for a client that is disconnected that we can use.
           */
          pClient = NimBLEDevice::getDisconnectedClient();
        }
      }

      if (!pClient)
      {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS)
        {
          throw std::runtime_error("Max clients reached - no more connections available\n");
        }

        pClient = NimBLEDevice::createClient();
        DEBUG_PRINT("New client created for a thermomater\n");

        pClient->setClientCallbacks(myClientCallbacks, false);

        pClient->setConnectionParams(12, 12, 0, 150);

        pClient->setConnectTimeout(20 * 1000); // set longer than 5 as an original

        bool result = pClient->connect(myScanCallbacks->advThermoDevices.at(index));
        //
        if (!result)
        {
          NimBLEDevice::deleteClient(pClient);
          myScanCallbacks->advThermoDevices.clear();
          throw std::runtime_error("Failed to connect a thermomater, deleted client\n");
        }
        //
        DEBUG_PRINT("Connected to a thermomater[%d/%d] at: %s RSSI: %d\n", index, myScanCallbacks->numberOfAdvThermoDevices,
                    pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
        //
        NimBLERemoteService *pSvc = nullptr;
        pSvc = pClient->getService(myScanCallbacks->serviceDataUUID_thermo);
        if (pSvc)
        {
          DEBUG_PRINT("serviceDataUUID of Thermomater found: %s\n", pSvc->toString().c_str());

          // setup temperature notification
          myThermoArr[index].pChr_rx_temp = pSvc->getCharacteristic(myScanCallbacks->charUUID_thermo_temp);
          if (!myThermoArr[index].pChr_rx_temp)
          {
            throw std::runtime_error("charUUID_thermo_temp not found.\n");
          }
          DEBUG_PRINT("charUUID_thermo_temp found.\n");

          if (myThermoArr[index].pChr_rx_temp->canNotify())
          {
            if (!myThermoArr[index].pChr_rx_temp->subscribe(true, [this](NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                            { notifyCB(pRemoteCharacteristic, pData, length, isNotify); }))
            {
              pClient->disconnect();
              throw std::runtime_error("Failed subscribeing charUUID_thermo_temp.\n");
            }
          }
          // setup humidity notification
          myThermoArr[index].pChr_rx_humid = pSvc->getCharacteristic(myScanCallbacks->charUUID_thermo_humid);
          if (!myThermoArr[index].pChr_rx_humid)
          {
            throw std::runtime_error("charUUID_thermo_humid not found.\n");
          }
          DEBUG_PRINT("charUUID_thermo_humid found.\n");

          if (myThermoArr[index].pChr_rx_humid->canNotify())
          {
            if (!myThermoArr[index].pChr_rx_humid->subscribe(true, [this](NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                             { notifyCB(pRemoteCharacteristic, pData, length, isNotify); }))
            {
              pClient->disconnect();
              throw std::runtime_error("Failed subscribeing charUUID_thermo_humid.\n");
            }
          }
        }
        else
        {
          throw std::runtime_error("serviceUUID of Thermomater not found.\n");
        }
      }
      DEBUG_PRINT("Done with an advertized thermomater[%d/%d]!\n", index, myScanCallbacks->numberOfAdvThermoDevices);
      atLeastOneConnected = true;
    }
    catch (const std::runtime_error &e)
    {
      DEBUG_PRINT("%s\n", e.what());
      int numberOfThermoDevicesFound = *myScanCallbacks->numberOfThermoDevicesFound;
      *myScanCallbacks->numberOfThermoDevicesFound *= numberOfThermoDevicesFound - 1;
      continue;
    }
  }
  myScanCallbacks->advThermoDevices.clear();
  DEBUG_PRINT("Done with all the advertized thermomaters[%d]!\n", myScanCallbacks->numberOfAdvThermoDevices);
  return atLeastOneConnected;
}

void MyNotification::printBatteryInfo(int bleIndex, int numberOfAdvDevices, MyBLE2 myBle)
{
  DEBUG_PRINT("== %d/%d =============================\n", bleIndex, numberOfAdvDevices);
  DEBUG_PRINT("deviceName = %s\n", myBle.deviceName.c_str());
  DEBUG_PRINT("packBasicInfo.Volts = %d\n", myBle.packBasicInfo.Volts);
  DEBUG_PRINT("packBasicInfo.Amps = %d\n", myBle.packBasicInfo.Amps);
  DEBUG_PRINT("packCellInfo.CellDiff = %d\n", myBle.packCellInfo.CellDiff);
  DEBUG_PRINT("packBasicInfo.CapacityRemainPercent = %d\n", myBle.packBasicInfo.CapacityRemainPercent);
  DEBUG_PRINT("packBasicInfo.Temp1 = %d\n", myBle.packBasicInfo.Temp1);
  DEBUG_PRINT("packBasicInfo.Temp2 = %d\n", myBle.packBasicInfo.Temp2);
  DEBUG_PRINT("===================================\n");
}
