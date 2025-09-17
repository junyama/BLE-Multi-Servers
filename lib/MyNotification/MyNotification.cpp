#include "MyNotification.hpp"

MyNotification::MyNotification(MyScanCallbacks *myScanCallbacks_, MyClientCallbacks *myClientCallbacks_)
    : myScanCallbacks(myScanCallbacks_), myClientCallbacks(myClientCallbacks_)
{
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
  DEBUG_PRINT("Notification from Address: %s, Service: %s, Characteristic: %s, Value: 0x%x\n",
              pRemoteCharacteristic->getClient()->getPeerAddress().toString().c_str(),
              pRemoteCharacteristic->getRemoteService()->getUUID().toString().c_str(),
              pRemoteCharacteristic->getUUID().toString().c_str(),
              (int)pData);

  try
  {
    if (pRemoteCharacteristic->getRemoteService()->getUUID() == myScanCallbacks->serviceUUID2)
    {
      DEBUG_PRINT("Notification from BMS\n");
      // int bleIndex = getIndexOfMyBleArr(pRemoteCharacteristic->getClient());
      int bleIndex = MyGetIndex::bleDevices(&myScanCallbacks->bleDevices, pRemoteCharacteristic->getClient());
      if (bleIndex > -1)
      {
        DEBUG3_PRINT("Notification from %s\n", MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, bleIndex).c_str());
        //myBleArr[bleIndex].bleCollectPacket((char *)pData, length);
        myScanCallbacks->bleDevices[bleIndex].bleCollectPacket((char *)pData, length);
        return;
      }
      else
      {
        throw std::runtime_error("Notification from a BMS with a wrong mac\n");
      }
      return;
    }

    if (pRemoteCharacteristic->getRemoteService()->getUUID() == myScanCallbacks->serviceDataUUID_thermo)
    {
      DEBUG_PRINT("Notification from Thermomater\n");
      // int thermoIndex = getIndexOfMyThermoArr(pRemoteCharacteristic->getClient());
      // int thermoIndex = MyGetIndex::myThermoArr(myThermoArr, pRemoteCharacteristic->getClient());
      int thermoIndex = MyGetIndex::thermoDevices(&myScanCallbacks->thermoDevices, pRemoteCharacteristic->getClient());
      if (thermoIndex > -1)
      {
        // DEBUG3_PRINT("Notification from myThermoArr[%d]\n", thermoIndex);
        auto remoteCharacteristicUUID = pRemoteCharacteristic->getUUID();
        DEBUG_PRINT("Notification from myScanCallbacks->thermoDevices[%d], UUID: %s\n", thermoIndex,
                    remoteCharacteristicUUID.toString().c_str());
        if (remoteCharacteristicUUID.equals(myScanCallbacks->charUUID_thermo_temp))
        {
          DEBUG3_PRINT("Notification from %s, tempatarure: %.1fC\n",
                       MyGetIndex::thermoInfo(&myScanCallbacks->thermoDevices, thermoIndex).c_str(),
                       myScanCallbacks->thermoDevices[thermoIndex].processTempPacket((char *)pData, length));
          //myThermoArr[thermoIndex].processTempPacket((char *)pData, length);

          // DEBUG3_PRINT("Notification from myThermoArr[%d], topic: %s, tempatarure: 0x%x\n",thermoIndex, myThermoArr[thermoIndex].topic.c_str(), (int)pData);
          return;
        }
        if (remoteCharacteristicUUID.equals(myScanCallbacks->charUUID_thermo_humid))
        {
          DEBUG3_PRINT("Notification from %s, humidity: %.1f%%\n",
                       MyGetIndex::thermoInfo(&myScanCallbacks->thermoDevices, thermoIndex).c_str(),
                       myScanCallbacks->thermoDevices[thermoIndex].processHumidPacket((char *)pData, length));
          // DEBUG3_PRINT("Notification from myThermoArr[%d], topic: %s, humidity: 0x%x\n",thermoIndex, myThermoArr[thermoIndex].topic.c_str(), (int)pData);
          return;
        }
        char buff[256];
        sprintf(buff, "no matching UUID: %s of myScanCallbacks->thermoDevices[%d]", remoteCharacteristicUUID.toString().c_str(), thermoIndex);
        throw std::runtime_error(buff);
      }
      else
      {
        throw std::runtime_error("Notification from a Thermomater with a wrong mac\n");
      }
      return;
    }
    throw std::runtime_error("Notification from a device other than BMS or Thermomater\n");
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
  //numberOfConnectedBMS = 0;
  for (int index = 0; index < myScanCallbacks->advDevices.size(); index++)
  {
    int count = index + 1;
    try
    {
      std::string deviceName = myScanCallbacks->advDevices.at(index)->getName();
      // INFO_PRINT("Start connecting a BMS[%d] in %d: %s\n", index, myScanCallbacks->advDevices.size(), deviceName.c_str());
      INFO_PRINT("[%d/%d]: Start connecting a BMS[%d]: %s >>>>>>>\n",
                 count, myScanCallbacks->advDevices.size(), index, deviceName.c_str());
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
            WARN_PRINT("[%d/%d] Failed to reconnect with %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, index).c_str());
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
          WARN_PRINT("Max clients(%d) reached - no more connections available\n", NIMBLE_MAX_CONNECTIONS);
          break;
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
        pClient->setConnectTimeout(5 * 1000);
        pClient->setSelfDelete(true, true);
        if (!pClient->connect(myScanCallbacks->advDevices.at(index)))
        {
          /** Created a client but failed to connect, don't need to keep it as it has no data */
          // NimBLEDevice::deleteClient(pClient);
          WARN_PRINT("[%d/%d] Failed to reconnect with %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, index).c_str());
          if (failCount <= FAIL_LIMIT)
          {
            WARN_PRINT("return with false\n");
            // clearResources();
            return false;
          }
          WARN_PRINT("Exceeded the fail limit (%d). Delete client and continue to next device\n", FAIL_LIMIT);
          NimBLEDevice::deleteClient(pClient);
          continue;
        }
      }

      if (!pClient->isConnected())
      {
        if (!pClient->connect(myScanCallbacks->advDevices.at(index)))
        {
          WARN_PRINT("[%d/%d] Failed to connect with %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, index).c_str());
          if (failCount <= FAIL_LIMIT)
          {
            WARN_PRINT("return with false\n");
            // clearResources();
            return false;
          }
          WARN_PRINT("Exceeded the fail limit (%d). Delete client and continue to next device\n", FAIL_LIMIT);
          NimBLEDevice::deleteClient(pClient);
          continue;
        }
      }

      DEBUG_PRINT("Connected to BMS[%d] in %d at: %s RSSI: %d\n", index, myScanCallbacks->advDevices.size(),
                  pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

      /** Now we can read/write/subscribe the characteristics of the services we are interested in */
      NimBLERemoteService *pSvc = nullptr;
      pSvc = pClient->getService(myScanCallbacks->serviceUUID);
      if (pSvc)
      {
        DEBUG2_PRINT("serviceDataUUID of BMS found: %s\n", pSvc->toString().c_str());
        //myBleArr[index].pChr_rx = pSvc->getCharacteristic(myScanCallbacks->charUUID_rx);
        myScanCallbacks->bleDevices[index].pChr_rx = pSvc->getCharacteristic(myScanCallbacks->charUUID_rx);
        if (/*!myBleArr[index].pChr_rx || */!myScanCallbacks->bleDevices[index].pChr_rx)
        {
          WARN_PRINT("[%d/%d] charUUID_rx not found %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, index).c_str());
          if (failCount <= FAIL_LIMIT)
          {
            WARN_PRINT("return with false\n");
            // clearResources();
            return false;
          }
          WARN_PRINT("Exceeded the fail limit (%d). Delete client and continue to next device\n", FAIL_LIMIT);
          NimBLEDevice::deleteClient(pClient);
          continue;
        }
        if (/*myBleArr[index].pChr_rx->canNotify() || */myScanCallbacks->bleDevices[index].pChr_rx->canNotify())
        {
          if (/*!myBleArr[index].pChr_rx->subscribe(true, [this](NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                  { notifyCB(pRemoteCharacteristic, pData, length, isNotify); }) || */
                                                   !myScanCallbacks->bleDevices[index].pChr_rx->subscribe(true, [this](NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                  { notifyCB(pRemoteCharacteristic, pData, length, isNotify); }))
          {
            WARN_PRINT("[%d/%d] subscription failed %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, index).c_str());
            if (failCount <= FAIL_LIMIT)
            {
              WARN_PRINT("return with false\n");
              return false;
            }
            WARN_PRINT("Exceeded the fail limit (%d). Deletea client and continue to next device\n", FAIL_LIMIT);
            NimBLEDevice::deleteClient(pClient);
            continue;
          }
        }
        //myBleArr[index].pChr_tx = pSvc->getCharacteristic(myScanCallbacks->charUUID_tx);
        myScanCallbacks->bleDevices[index].pChr_tx = pSvc->getCharacteristic(myScanCallbacks->charUUID_tx);
        if (/*!myBleArr[index].pChr_tx ||*/ !myScanCallbacks->bleDevices[index].pChr_tx)
        {
          WARN_PRINT("[%d/%d] charUUID_tx not found %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, index).c_str());
          if (failCount <= FAIL_LIMIT)
          {
            WARN_PRINT("return with false\n");
            // clearResources();
            return false;
          }
          WARN_PRINT("Exceeded the fail limit (%d). Deletea client and continue to next device\n", FAIL_LIMIT);
          NimBLEDevice::deleteClient(pClient);
          continue;
        }
      }
      else
      {
        WARN_PRINT("[%d/%d] serviceUUID not found %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::bleInfo(&myScanCallbacks->bleDevices, index).c_str());
        if (failCount <= FAIL_LIMIT)
        {
          WARN_PRINT("return with false\n");
          // clearResources();
          return false;
        }
        WARN_PRINT("Exceeded the fail limit (%d). Deletea client and continue to next device\n", FAIL_LIMIT);
        NimBLEDevice::deleteClient(pClient);
        continue;
      }
      // INFO_PRINT("Connected with an advertized BMS[%d] in %d!\n", index, myScanCallbacks->advDevices.size());
      INFO_PRINT("[%d/%d]: Connected with an myScanCallbacks->bleDevices[%d]: %s <<<<<<<<\n", count, myScanCallbacks->advDevices.size(), index, deviceName.c_str());
      atLeastOneConnected = true;
      //numberOfConnectedBMS++;

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
             myClientCallbacks->numberOfConnectedBMS, myScanCallbacks->advDevices.size());
  // return atLeastOneConnected;
  if (myClientCallbacks->numberOfConnectedBMS > 0)
    return true;
  return false;
}

bool MyNotification::connectToThermo()
{
  DEBUG_PRINT("connectToThermo() called\n");
  NimBLEClient *pClient = nullptr;
  bool atLeastOneConnected = false;
  //numberOfConnectedThermo = 0;
  int exceptionType = 0;
  for (int index = 0; index < myScanCallbacks->advThermoDevices.size(); index++)
  {
    int count = index + 1;
    std::string deviceName = myScanCallbacks->advThermoDevices.at(index)->getName();
    INFO_PRINT("[%d/%d]: Start connecting a thermometer[%d]: %s >>>>>>>\n",
               count, myScanCallbacks->thermoDevices.size(), index, deviceName.c_str());
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
          // throw std::runtime_error("Reconnect failed\n");
          WARN_PRINT("Reconnect failed\n");
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
        exceptionType = 0; // exit
        char buff[256];
        printf(buff, "Max clients(%d) reached - no more connections available", NIMBLE_MAX_CONNECTIONS);
        WARN_PRINT("%s\n", buff);
        break;
      }

      pClient = NimBLEDevice::createClient();

      DEBUG_PRINT("New client created for a thermomater\n");

      pClient->setClientCallbacks(myClientCallbacks, false);
      /* Set initial connection parameters:*/
      pClient->setConnectionParams(12, 12, 0, 150);
      pClient->setConnectTimeout(40 * 1000); // set longer than 5 as an original

      pClient->setSelfDelete(true, true);

      if (!pClient->connect(myScanCallbacks->advThermoDevices.at(index)))
      {
        //++failCount;
        char buff[256];
        sprintf(buff, "[%d/%d] Failed to connect with %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::thermoInfo(&myScanCallbacks->thermoDevices, index).c_str());
        WARN_PRINT("%s\n", buff);
        if (failCount <= FAIL_LIMIT)
        {
          WARN_PRINT("return with false\n");
          clearResources();
          /*
          auto clientList = NimBLEDevice::getConnectedClients();
          for (NimBLEClient *pClient : clientList)
          {
            NimBLEDevice::deleteClient(pClient);
          }
          myScanCallbacks->advDevices.clear();
          myScanCallbacks->numberOfBMS = 0;
          myScanCallbacks->advThermoDevices.clear();
          myScanCallbacks->numberOfThermo = 0;
          */
          return false;
        } //

        /* experimental code
        WARN_PRINT("[%d] Failed to connect with %s\n", ++failCount, MyGetIndex::thermoInfo(myThermoArr, index).c_str());
        if (failCount < 3)
        {
          // WARN_PRINT("failCount: %d\n", ++failCount);
          auto clientList = NimBLEDevice::getConnectedClients();
          for (NimBLEClient *pClient : clientList)
          {
            NimBLEDevice::deleteClient(pClient);
          }
          myScanCallbacks->advDevices.clear();
          myScanCallbacks->numberOfBMS = 0;
          myScanCallbacks->advThermoDevices.clear();
          myScanCallbacks->numberOfThermo = 0;
          return false;
        }
        exceptionType = 1;
        NimBLEDevice::deleteClient(pClient);
        char buff[256];
        sprintf(buff, "[%d/%d]: Failed to connect with myThermoArr[%d]: %s, deleted client",
                count, numberOfThermo, index, deviceName.c_str());
        throw std::runtime_error(buff);
        // experimental code */

        WARN_PRINT("Exceeded the fail limit (%d). Deletea client and continue to next device\n", FAIL_LIMIT);
        NimBLEDevice::deleteClient(pClient);
        continue;
      }
    }

    if (!pClient->isConnected())
    {
      if (!pClient->connect(myScanCallbacks->advThermoDevices.at(index)))
      {
        WARN_PRINT("[%d/%d] Failed to connect with %s\n", ++failCount, FAIL_LIMIT, MyGetIndex::thermoInfo(&myScanCallbacks->thermoDevices, index).c_str());
        if (failCount <= FAIL_LIMIT)
        {
          WARN_PRINT("return with false\n");
          // clearResources();
          return false;
        }
        WARN_PRINT("Exceeded the fail limit (%d). Deletea client and continue to next device\n", FAIL_LIMIT);
        NimBLEDevice::deleteClient(pClient);
        continue;
      }
    }

    DEBUG2_PRINT("Connected to a thermomater[%d] in %d: %s at: %s RSSI: %d\n",
                 index, myScanCallbacks->advThermoDevices.size(), deviceName.c_str(),
                 pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    {
      /** Now we can read/write/subscribe the characteristics of the services we are interested in */
      NimBLERemoteService *pSvc = nullptr;
      pSvc = pClient->getService(myScanCallbacks->serviceDataUUID_thermo);
      if (pSvc)
      {
        DEBUG2_PRINT("serviceDataUUID of Thermomater found: %s\n", pSvc->toString().c_str());
        // setup temperature Notification
        myScanCallbacks->thermoDevices[index].pChr_rx_temp = pSvc->getCharacteristic(myScanCallbacks->charUUID_thermo_temp);
        if (!myScanCallbacks->thermoDevices[index].pChr_rx_temp)
        {
          WARN_PRINT("[%d/%d] charUUID_thermo_temp not found\n", ++failCount, FAIL_LIMIT);
          if (failCount <= FAIL_LIMIT)
          {
            WARN_PRINT("return with false\n");
            // clearResources();
            return false;
          }
          WARN_PRINT("Exceeded the fail limit (%d). Deletea client and continue to next device\n", FAIL_LIMIT);
          NimBLEDevice::deleteClient(pClient);
          continue;
        }
        DEBUG2_PRINT("charUUID_thermo_temp found.\n");
        if (myScanCallbacks->thermoDevices[index].pChr_rx_temp->canNotify())
        {
          if (!myScanCallbacks->thermoDevices[index].pChr_rx_temp->subscribe(true, [this](NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                          { notifyCB(pRemoteCharacteristic, pData, length, isNotify); }))
          {
            WARN_PRINT("[%d/%d] Failed subscribeing charUUID_thermo_temp.\n", ++failCount, FAIL_LIMIT);
            if (failCount <= FAIL_LIMIT)
            {
              WARN_PRINT("return with false\n");
              // clearResources();
              return false;
            }
            WARN_PRINT("Exceeded the fail limit (%d). Deletea client and continue to next device\n", FAIL_LIMIT);
            pClient->disconnect();
            NimBLEDevice::deleteClient(pClient);
            continue;
          }
        }
        // setup humidity Notification
        myScanCallbacks->thermoDevices[index].pChr_rx_humid = pSvc->getCharacteristic(myScanCallbacks->charUUID_thermo_humid);
        if (!myScanCallbacks->thermoDevices[index].pChr_rx_humid)
        {
          WARN_PRINT("[%d/%d] charUUID_thermo_humid not found.\n", ++failCount, FAIL_LIMIT);
          if (failCount <= FAIL_LIMIT)
          {
            WARN_PRINT("return with false\n");
            // clearResources();
            return false;
          }
          WARN_PRINT("Exceeded the fail limit (%d). Delete client and continue to next device\n", FAIL_LIMIT);
          NimBLEDevice::deleteClient(pClient);
          continue;
        }
        DEBUG2_PRINT("charUUID_thermo_humid found.\n");
        if (myScanCallbacks->thermoDevices[index].pChr_rx_humid->canNotify())
        {
          if (!myScanCallbacks->thermoDevices[index].pChr_rx_humid->subscribe(true, [this](NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                                           { notifyCB(pRemoteCharacteristic, pData, length, isNotify); }))
          {
            WARN_PRINT("[%d/%d] Failed subscribeing charUUID_thermo_humid.\n", ++failCount, FAIL_LIMIT);
            if (failCount <= FAIL_LIMIT)
            {
              WARN_PRINT("return with false\n");
              // clearResources();
              return false;
            }
            WARN_PRINT("Exceeded the fail limit (%d). Delete client and continue to next device\n", FAIL_LIMIT);
            NimBLEDevice::deleteClient(pClient);
            continue;
          }
        }
      }
      else
      {
        WARN_PRINT("[%d/%d] serviceUUID of Thermomater not found.\n", ++failCount, FAIL_LIMIT);
        if (failCount <= FAIL_LIMIT)
        {
          WARN_PRINT("return with false\n");
          // clearResources();
          return false;
        }
        WARN_PRINT("Exceeded the fail limit (%d). Delete client and continue to next device\n", FAIL_LIMIT);
        NimBLEDevice::deleteClient(pClient);
        continue;
      }
    }

    INFO_PRINT("[%d/%d]: Connected with an myScanCallbacks->thermoDevices[%d]: %s <<<<<<<<\n", count, myScanCallbacks->thermoDevices.size(), index, deviceName.c_str());
    INFO_PRINT("Now getting notifications from myScanCallbacks->thermoDevices[%d]\n", myClientCallbacks->numberOfConnectedThermo);
    // myThermoArr[index].connected = true;
    //  thermomater[index].available = true;
    atLeastOneConnected = true;
    //numberOfConnectedThermo++;
  }
  // INFO_PRINT("Done with all the advertized thermomaters, connected (%d/%d)\n", numberOfConnectedThermo, myScanCallbacks->myScanCallbacks->thermoDevices.size());
  //  return atLeastOneConnected;
  if (myClientCallbacks->numberOfConnectedThermo > 0)
    return true;
  return false;
}

void MyNotification::clearResources()
{
  auto clientList = NimBLEDevice::getConnectedClients();
  for (NimBLEClient *pClient : clientList)
  {
    NimBLEDevice::deleteClient(pClient);
  }
  //numberOfConnectedBMS = 0;
  //numberOfConnectedThermo = 0;
  for (int i = 0; i < myScanCallbacks->advDevices.size(); i++)
  {
    /*
    myBleArr[i].connected = false;
    myBleArr[i].mac = "00:00:00:00:00:00";
    myBleArr[i].deviceName = "UNKNOWN";
    myBleArr[i].topic = "NOT_DEFINED/";
    */
  }
  myScanCallbacks->advDevices.clear();
  //myScanCallbacks->numberOfBMS = 0;
  /*
  for (int i = 0; i < myScanCallbacks->numberOfThermo; i++)
  {
    myThermoArr[i].connected = false;
    myThermoArr[i].mac = "00:00:00:00:00:00";
    myThermoArr[i].deviceName = "UNKNOWN";
    myThermoArr[i].topic = "NOT_DEFINED/";
  }
  */
  myScanCallbacks->advThermoDevices.clear();
  //myScanCallbacks->numberOfThermo = 0;
}