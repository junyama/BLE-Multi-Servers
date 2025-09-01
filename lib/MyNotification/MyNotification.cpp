#include "MyNotification.hpp"

MyNotification::MyNotification(MyBLE2 *myBleArr_, MyScanCallbacks *myScanCallbacks_, MyClientCallbacks *myClientCallbacks_, MyThermo *myThermoArr_)
    : myBleArr(myBleArr_), myScanCallbacks(myScanCallbacks_), myClientCallbacks(myClientCallbacks_), myThermoArr(myThermoArr_)
{
  DEBUG_PRINT("an instance created\n");
}

// int getIndexOfMyBleArr(NimBLERemoteCharacteristic *pRemoteCharacteristic)
int MyNotification::getIndexOfMyBleArr(NimBLEClient *client)
{
  DEBUG_PRINT("getIndexOfMyBleArr: advDevices.size(): %d\n", myScanCallbacks->advDevices.size());
  auto peerAddress = client->getPeerAddress();
  DEBUG_PRINT("getIndexOfMyBleArr: peerAddress: %s\n", peerAddress.toString().c_str());
  for (int index = 0; index < myScanCallbacks->advDevices.size(); index++)
  {
    DEBUG_PRINT("getIndexOfMyBleArr: myBleArr[index].mac: %s\n",
                myBleArr[index].pChr_rx->getClient()->getPeerAddress().toString().c_str());
    if (peerAddress == myBleArr[index].pChr_rx->getClient()->getPeerAddress())
    {
      return index;
    }
  }
  return -1;
}

int MyNotification::getIndexOfMyThermoArr(NimBLEClient *client)
{
  auto peerAddress = client->getPeerAddress();
  for (int index = 0; index < myScanCallbacks->advThermoDevices.size(); index++)
  {
    // if (peerAddress == myThermoArr[i].pChr_rx_temp->getClient()->getPeerAddress())
    std::string macStr = myThermoArr[index].mac.c_str();
    if (peerAddress == NimBLEAddress(macStr, 0))
    {
      return index;
    }
  }
  return -1;
}

/** Notification / Indication receiving handler callback */
void MyNotification::notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  /*
  std::string str = (isNotify == true) ? "Notification" : "Indication";
  str += " from ";
  str += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
  str += ", Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
  str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
  str += ", Value = " + std::string((char *)pData, length);
  DEBUG_PRINT("%s\n", str.c_str());
  */
  DEBUG3_PRINT("Notification from mac: %s, Service: %s, Characteristic: %s, Value: 0x%x\n",
              pRemoteCharacteristic->getClient()->getPeerAddress().toString().c_str(),
              pRemoteCharacteristic->getRemoteService()->getUUID().toString().c_str(),
              pRemoteCharacteristic->getUUID().toString().c_str(),
              (int)pData);

  try
  {
    if (pRemoteCharacteristic->getRemoteService()->getUUID() == myScanCallbacks->serviceUUID2)
    {
      DEBUG_PRINT("notification from BMS\n");
      int bleIndex = getIndexOfMyBleArr(pRemoteCharacteristic->getClient());
      if (bleIndex > -1)
      {
        DEBUG3_PRINT("notification from myBleArr[%d]\n", bleIndex);
        myBleArr[bleIndex].bleCollectPacket((char *)pData, length);
        return;
      }
      else
      {
        throw std::runtime_error("notification from a BMS with a wrong mac\n");
      }
      return;
    }

    if (pRemoteCharacteristic->getRemoteService()->getUUID() == myScanCallbacks->serviceDataUUID_thermo)
    {
      DEBUG_PRINT("notification from Thermomater\n");
      int thermoIndex = getIndexOfMyThermoArr(pRemoteCharacteristic->getClient());
      if (thermoIndex > -1)
      {
        //DEBUG3_PRINT("notification from myThermoArr[%d]\n", thermoIndex);
        auto remoteCharacteristicUUID = pRemoteCharacteristic->getUUID();
        DEBUG3_PRINT("notification from myThermoArr[%d], UUID: %s\n", thermoIndex,
                     remoteCharacteristicUUID.toString().c_str());
        if (remoteCharacteristicUUID.equals(myScanCallbacks->charUUID_thermo_temp))
        {
          myThermoArr[thermoIndex].processTempPacket((char *)pData, length);
          return;
        }
        if (remoteCharacteristicUUID.equals(myScanCallbacks->charUUID_thermo_humid))
        {
          myThermoArr[thermoIndex].processHumidPacket((char *)pData, length);
          return;
        }
        char buff[256];
        sprintf(buff, "no matching UUID: %s of myThermoArr[%d]", remoteCharacteristicUUID.toString().c_str(), thermoIndex);
        throw std::runtime_error(buff);
      }
      else
      {
        throw std::runtime_error("notification from a Thermomater with a wrong mac\n");
      }
      return;
    }
    throw std::runtime_error("notification from a device other than BMS or Thermomater\n");
  }
  catch (const std::runtime_error &e)
  {
    ERROR_PRINT("%s\n", e.what());
  }
}

bool MyNotification::connectToServer()
{
  DEBUG_PRINT("connectToServer() called\n");
  NimBLEClient *pClient = nullptr;
  unsigned long initalMesurementTime = 0;
  bool atLeastOneConnected = false;
  numberOfConnectedBMS = 0;
  for (int index = 0; index < myScanCallbacks->advDevices.size(); index++)
  {
    try
    {
      std::string deviceName = myScanCallbacks->advDevices.at(index)->getName();
      INFO_PRINT("Start connecting a BMS[%d/%d]: %s\n",
                   (index + 1), myScanCallbacks->advDevices.size(), deviceName.c_str());
      // Check if we have a client we should reuse first
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
          char buff[256];
          printf(buff, "Max clients(%d) reached - no more connections available", NIMBLE_MAX_CONNECTIONS);
          throw std::runtime_error(buff);
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
        pClient->setConnectTimeout(30 * 1000);
        pClient->setSelfDelete(true, true);
        if (!pClient->connect(myScanCallbacks->advDevices.at(index)))
        {
          /** Created a client but failed to connect, don't need to keep it as it has no data */
          //NimBLEDevice::deleteClient(pClient);
          char buff[256];
          sprintf(buff, "Failed to connect BMS[%d/%d]: %s, deleted client",
                  (index + 1), myScanCallbacks->advDevices.size(), deviceName.c_str());
          throw std::runtime_error(buff);
        }
      }

      if (!pClient->isConnected())
      {
        if (!pClient->connect(myScanCallbacks->advDevices.at(index)))
        {
          throw std::runtime_error("Failed to connect BMS\n");
        }
      }

      DEBUG_PRINT("Connected to BMS[%d/%d] at: %s RSSI: %d\n", (index + 1), myScanCallbacks->advDevices.size(),
                  pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

      /** Now we can read/write/subscribe the characteristics of the services we are interested in */
      NimBLERemoteService *pSvc = nullptr;
      pSvc = pClient->getService(myScanCallbacks->serviceUUID);
      if (pSvc)
      {
        DEBUG2_PRINT("serviceDataUUID of BMS found: %s\n", pSvc->toString().c_str());
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
      INFO_PRINT("Connected with an advertized BMS[%d/%d]!\n", (index + 1), myScanCallbacks->advDevices.size());
      atLeastOneConnected = true;
      numberOfConnectedBMS++;

      /*
      new (myTimerArr + index) MyTimer(initalMesurementTime, 5000);
      DEBUG_PRINT("myTimerArr[%d].lastMeasurment = %d, measurmentIntervalMs = %d\n", index, myTimerArr[index].lastMeasurment,
                  myTimerArr[index].measurmentIntervalMs);
      initalMesurementTime += 200;
      */
    }
    catch (const std::runtime_error &e)
    {
      ERROR_PRINT("%s\n", e.what());
      continue;
    }
  }
  INFO_PRINT("Done with all the advertized BMS, connected(%d/%d)!\n",
              numberOfConnectedBMS, myScanCallbacks->advDevices.size());
  return atLeastOneConnected;
}

bool MyNotification::connectToThermo()
{
  DEBUG_PRINT("connectToThermo() called\n");
  NimBLEClient *pClient = nullptr;
  bool atLeastOneConnected = false;
  numberOfConnectedThermo = 0;
  for (int index = 0; index < myScanCallbacks->advThermoDevices.size(); index++)
  {
    try
    {
      std::string deviceName = myScanCallbacks->advThermoDevices.at(index)->getName();
      INFO_PRINT("Start connecting a thermometer[%d/%d]: %s\n",
                   (index + 1), myScanCallbacks->advThermoDevices.size(), deviceName.c_str());
      // Check if we have a client we should reuse first
      if (NimBLEDevice::getCreatedClientCount())
      {
        // DEBUG_PRINT("NimBLEDevice::getCreatedClientCount(): %d\n", NimBLEDevice::getCreatedClientCount());
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
      /** No client to reuse? Create a new one. */
      if (!pClient)
      {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS)
        {
          char buff[256];
          printf(buff, "Max clients(%d) reached - no more connections available", NIMBLE_MAX_CONNECTIONS);
          throw std::runtime_error("Max clients reached - no more connections available\n");
        }
        pClient = NimBLEDevice::createClient();
        DEBUG_PRINT("New client created for a thermomater\n");
        pClient->setClientCallbacks(myClientCallbacks, false);
        pClient->setConnectionParams(12, 12, 0, 150);
        pClient->setConnectTimeout(40 * 1000); // set longer than 5 as an original
        pClient->setSelfDelete(true, true);
        if (!pClient->connect(myScanCallbacks->advThermoDevices.at(index)))
        {
          //NimBLEDevice::deleteClient(pClient);
          char buff[256];
          sprintf(buff, "Failed to connect thermomater[%d/%d]: %s, deleted client",
                  (index + 1), myScanCallbacks->advThermoDevices.size(), deviceName.c_str());
          throw std::runtime_error(buff);
        }
        DEBUG2_PRINT("Connected to a thermomater[%d/%d]: %s at: %s RSSI: %d\n",
                     (index + 1), myScanCallbacks->advThermoDevices.size(), deviceName.c_str(),
                     pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
        NimBLERemoteService *pSvc = nullptr;
        pSvc = pClient->getService(myScanCallbacks->serviceDataUUID_thermo);
        if (pSvc)
        {
          DEBUG2_PRINT("serviceDataUUID of Thermomater found: %s\n", pSvc->toString().c_str());
          // setup temperature notification
          myThermoArr[index].pChr_rx_temp = pSvc->getCharacteristic(myScanCallbacks->charUUID_thermo_temp);
          if (!myThermoArr[index].pChr_rx_temp)
          {
            throw std::runtime_error("charUUID_thermo_temp not found.\n");
          }
          DEBUG2_PRINT("charUUID_thermo_temp found.\n");
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
          DEBUG2_PRINT("charUUID_thermo_humid found.\n");
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
      INFO_PRINT("Connected with an advertized thermomater[%d/%d]!\n", (index + 1), myScanCallbacks->advThermoDevices.size());
      // thermomater[index].available = true;
      atLeastOneConnected = true;
      numberOfConnectedThermo++;
    }
    catch (const std::runtime_error &e)
    {
      ERROR_PRINT("%s\n", e.what());
      //return false; //not workring
      continue;
    }
  }
  INFO_PRINT("Done with all the advertized thermomaters, connected (%d/%d)\n",
               numberOfConnectedThermo, myScanCallbacks->advThermoDevices.size());
  return atLeastOneConnected;
}

void MyNotification::clear()
{
  numberOfConnectedBMS = 0;
  numberOfConnectedThermo = 0;
}