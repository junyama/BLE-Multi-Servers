#include <NimBLEDevice.h>
#include "MyBLE.cpp"
#include "MyScanCallbacks.cpp"
#include "MyLog.cpp"
#include "MyTimer.cpp"

extern char *TAG;
extern MyScanCallbacks myScanCallbacks;
extern MyBLE myBleArr[3];
extern NimBLEClientCallbacks clientCallbacks;
extern MyTimer myTimerArr[3];

int getIndexOfMyBleArr(NimBLERemoteCharacteristic *pRemoteCharacteristic)
{
  auto peerAddress = pRemoteCharacteristic->getClient()->getPeerAddress();
  for (int i = 0; i < myScanCallbacks.numberOfAdvDevices; i++)
  {
    if (peerAddress == myBleArr[i].pChr_rx->getClient()->getPeerAddress())
    {
      return i;
    }
  }
  return 0;
}

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  std::string str = (isNotify == true) ? "Notification" : "Indication";
  str += " from ";
  str += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
  str += ", Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
  str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
  str += ", Value = " + std::string((char *)pData, length);
  DEBUG_PRINT("%s\n", str.c_str());
  myBleArr[getIndexOfMyBleArr(pRemoteCharacteristic)].bleCollectPacket((char *)pData, length);
}

bool connectToServer()
{
  NimBLEClient *pClient = nullptr;

  // Serial.printf("advDevices.size() = %d\n", advDevices.size());
  // numberOfAdvDevices = advDevices.size();
  // Serial.printf("numberOfAdvDevices = %d\n", numberOfAdvDevices);

  unsigned long initalMesurementTime = 0;
  for (int i = 0; i < myScanCallbacks.advDevices.size(); i++)
  {
    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getCreatedClientCount())
    {
      /**
       *  Special case when we already know this device, we send false as the
       *  second argument in connect() to prevent refreshing the service database.
       *  This saves considerable time and power.
       */
      pClient = NimBLEDevice::getClientByPeerAddress(myScanCallbacks.advDevices.at(i)->getAddress());
      if (pClient)
      {
        if (!pClient->connect(myScanCallbacks.advDevices.at(i), false))
        {
          DEBUG_PRINT("Reconnect failed\n");
          return false;
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
        DEBUG_PRINT("Max clients reached - no more connections available\n");
        return false;
      }

      pClient = NimBLEDevice::createClient();

      DEBUG_PRINT("New client created\n");

      pClient->setClientCallbacks(&clientCallbacks, false);
      /**
       *  Set initial connection parameters:
       *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
       *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
       *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
       */
      pClient->setConnectionParams(12, 12, 0, 150);

      /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
      pClient->setConnectTimeout(5 * 1000);

      if (!pClient->connect(myScanCallbacks.advDevices.at(i)))
      {
        /** Created a client but failed to connect, don't need to keep it as it has no data */
        NimBLEDevice::deleteClient(pClient);
        DEBUG_PRINT("Failed to connect, deleted client\n");
        myScanCallbacks.advDevices.clear();
        return false;
      }
    }

    if (!pClient->isConnected())
    {
      if (!pClient->connect(myScanCallbacks.advDevices.at(i)))
      {
        DEBUG_PRINT("Failed to connect\n");
        return false;
      }
    }

    DEBUG_PRINT("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    /** Now we can read/write/subscribe the characteristics of the services we are interested in */
    NimBLERemoteService *pSvc = nullptr;
    // NimBLERemoteCharacteristic *pChr = nullptr;
    // const std::vector<NimBLERemoteCharacteristic *> *pChrs = nullptr;
    // NimBLERemoteDescriptor *pDsc = nullptr;

    pSvc = pClient->getService(myScanCallbacks.serviceUUID);
    if (pSvc)
    {
      //
      // pChr_rx = nullptr;
      // pChrSt.pChr_rx = nullptr;

      // pChr_rx = pSvc->getCharacteristic(charUUID_rx);
      // pChrSt.pChr_rx = pSvc->getCharacteristic(charUUID_rx);
      myBleArr[i].pChr_rx = pSvc->getCharacteristic(myScanCallbacks.charUUID_rx);

      if (!myBleArr[i].pChr_rx)
      {
        DEBUG_PRINT("charUUID_rx not found.\n");
        return false;
      }
      if (myBleArr[i].pChr_rx->canNotify())
      {
        if (!myBleArr[i].pChr_rx->subscribe(true, notifyCB))
        {
          pClient->disconnect();
          return false;
        }
      }
      // pChr_tx = nullptr;
      // pChrSt.pChr_tx = nullptr;
      // pChr_tx = pSvc->getCharacteristic(myScanCallbacks.charUUID_tx);
      // pChrSt.pChr_tx = pSvc->getCharacteristic(myScanCallbacks.charUUID_tx);
      myBleArr[i].pChr_tx = pSvc->getCharacteristic(myScanCallbacks.charUUID_tx);

      if (!myBleArr[i].pChr_tx)
      {
        DEBUG_PRINT("charUUID_tx not found.\n");
        return false;
      }
      //
      // pChrs = &pSvc->getCharacteristics();
      //}
      // Serial.printf("pChrs->size() = %d\n", (int)pChrs->size());
      // if (pChrs)

      /*
      if (myBleArr[i].pChr_rx && myBleArr[i].pChr_tx)
      {
        // for (int i = 0; i < pChrs->size(); i++)
        {
          /*
          if (pChrs->at(i)->canRead())
          {
            Serial.printf("%s Value: %s\n", pChrs->at(i)->getUUID().toString().c_str(), pChrs->at(i)->readValue().c_str());
          }

          //
          if (pChrs->at(i)->canWrite())
          {
            if (pChrs->at(i)->writeValue("Tasty"))
            {
              Serial.printf("Wrote new value to: %s\n", pChrs->at(i)->getUUID().toString().c_str());
            }
            else
            {
              pClient->disconnect();
              return false;
            }

            if (pChrs->at(i)->canRead())
            {
              Serial.printf("The value of: %s is now: %s\n", pChrs->at(i)->getUUID().toString().c_str(), pChrs->at(i)->readValue().c_str());
            }
          }
          //

          if (pChrs->at(i)->canNotify())
          {
            if (!pChrs->at(i)->subscribe(true, notifyCB))
            {
              pClient->disconnect();
              return false;
            }
          }
          //

          if (myBleArr[i].pChr_rx->canNotify())
          {
            if (!myBleArr[i].pChr_rx->subscribe(true, notifyCB))
            {
              pClient->disconnect();
              return false;
            }
          }
          */

      /*
      else if (pChrs->at(i)->canIndicate())
      {
        // Send false as first argument to subscribe to indications instead of notifications
        if (!pChrs->at(i)->subscribe(false, notifyCB))
        {
          pClient->disconnect();
          return false;
        }
      }
      //
    }
    // pChrsV.push_back(pChrs);
    // pChrStV.push_back(pChrSt);
  }*/
    }
    else
    {
      DEBUG_PRINT("serviceUUID not found.\n");
      return false;
    }

    /*
    pSvc = pClient->getService(serviceUUID);
    if (pSvc)
    {
      // pChrs->at(i) = pSvc->getCharacteristic(charUUID_rx);
      if (pChrs->at(i))
      {
        if (pChrs->at(i)->canRead())
        {
          Serial.printf("%s Value: %s\n", pChrs->at(i)->getUUID().toString().c_str(), pChrs->at(i)->readValue().c_str());
        }

        pDsc = pChrs->at(i)->getDescriptor(NimBLEUUID("C01D"));
        if (pDsc)
        {
          Serial.printf("Descriptor: %s  Value: %s\n", pDsc->getUUID().toString().c_str(), pDsc->readValue().c_str());
        }

        if (pChrs->at(i)->canWrite())
        {
          if (pChrs->at(i)->writeValue("No tip!"))
          {
            Serial.printf("Wrote new value to: %s\n", pChrs->at(i)->getUUID().toString().c_str());
          }
          else
          {
            pClient->disconnect();
            return false;
          }

          if (pChrs->at(i)->canRead())
          {
            Serial.printf("The value of: %s is now: %s\n",
                          pChrs->at(i)->getUUID().toString().c_str(),
                          pChrs->at(i)->readValue().c_str());
          }
        }

        if (pChrs->at(i)->canNotify())
        {
          if (!pChrs->at(i)->subscribe(true, notifyCB))
          {
            pClient->disconnect();
            return false;
          }
        }
        else if (pChrs->at(i)->canIndicate())
        {
          // Send false as first argument to subscribe to indications instead of notifications //
          if (!pChrs->at(i)->subscribe(false, notifyCB))
          {
            pClient->disconnect();
            return false;
          }
        }
      }
    }
    else
    {
      Serial.printf("BAAD service not found.\n");
    }

    Serial.printf("Done with this device!\n");
    return true;
    */
    myBleArr[i].deviceName = String(myScanCallbacks.advDevices[i]->getName().c_str());
    DEBUG_PRINT("myBleArr[%d].deviceName = %s\n", i, myBleArr[i].deviceName.c_str());
    new (myTimerArr + i) MyTimer(initalMesurementTime, 5000);
    DEBUG_PRINT("myTimerArr[%d].lastMeasurment = %d, measurmentIntervalMs = %d\n", i, myTimerArr[i].lastMeasurment,
                myTimerArr[i].measurmentIntervalMs);
    initalMesurementTime += 1000;
  }
  myScanCallbacks.advDevices.clear();
  DEBUG_PRINT("Done with this device!\n");
  return true;
}

void printBatteryInfo(MyBLE myBle)
{
  DEBUG_PRINT("===================================\n");
  DEBUG_PRINT("deviceName = %s\n", myBle.deviceName.c_str());
  DEBUG_PRINT("packBasicInfo.Volts = %d\n", myBle.packBasicInfo.Volts);
  DEBUG_PRINT("packBasicInfo.Amps = %d\n", myBle.packBasicInfo.Amps);
  DEBUG_PRINT("packCellInfo.CellDiff = %d\n", myBle.packCellInfo.CellDiff);
  DEBUG_PRINT("packBasicInfo.CapacityRemainPercent = %d\n", myBle.packBasicInfo.CapacityRemainPercent);
  DEBUG_PRINT("packBasicInfo.Temp1 = %d\n", myBle.packBasicInfo.Temp1);
  DEBUG_PRINT("packBasicInfo.Temp2 = %d\n", myBle.packBasicInfo.Temp2);
  DEBUG_PRINT("===================================\n");
}
