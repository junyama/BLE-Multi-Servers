
/** NimBLE_Client Demo:
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

#include <M5Core2.h>

#include "MyBLE.cpp"
#include "MyScanCallBacks.cpp"

/*typedef struct
{
  NimBLERemoteCharacteristic *pChr_rx;
  NimBLERemoteCharacteristic *pChr_tx;
} pChrStruct;*/

// static pChrStruct pChrSt;
//  static std::vector<pChrStruct> pChrStV;

// static const NimBLEAdvertisedDevice *advDevice;
// static std::vector<const NimBLEAdvertisedDevice *> advDevices;
// static int numberOfAdvDevices = 0;
// static std::vector<const std::vector<NimBLERemoteCharacteristic *> *> pChrsV;
//  static NimBLERemoteCharacteristic *pChr_rx;
//  static NimBLERemoteCharacteristic *pChr_tx;

// static bool doConnect = false;
// static uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */

// static NimBLEUUID serviceUUID = BLEUUID("0000ff00-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
static NimBLEUUID charUUID_tx = BLEUUID("0000ff02-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
static NimBLEUUID charUUID_rx = BLEUUID("0000ff01-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module

// MyBLE myBLE;
MyBLE myBleArr[3];

NimBLEClientCallbacks clientCallbacks;

/** Define a class to handle the callbacks when scan events are received */
/*class ScanCallbacks : public NimBLEScanCallbacks
{
  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override
  {
    Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
    if (advertisedDevice->isAdvertisingService(serviceUUID))
    {
      Serial.printf("Found Our Service\n");
      // stop scan before connecting //
      // NimBLEDevice::getScan()->stop(); //Jun: comment out
      // Save the device reference in a global for the client to use //
      // advDevice = advertisedDevice;
      // Ready to connect now //
      // doConnect = true;

      //  Jun: added BEGIN
      if (advDevices.size() == 0)
      {
        // advDevice = advertisedDevice;
        advDevices.push_back(advertisedDevice);
        Serial.printf("onResult:advDevices.size()=%d\n", advDevices.size());
      }
      else
      {
        for (int i = 0; i < advDevices.size(); i++)
        {
          if (advDevices.at(i)->getAddress().equals(advertisedDevice->getAddress()))
          {
            Serial.printf("onResult:device already added\n");
            return;
          }
        }
        // advDevice = advertisedDevice;
        advDevices.push_back(advertisedDevice);
        Serial.printf("onResult:advDevices.size()=%d\n", advDevices.size());
      }
      // END
    }
  }

  // Callback to process the results of the completed scan or restart it //
  void onScanEnd(const NimBLEScanResults &results, int reason) override
  {
    Serial.printf("Scan Ended, reason: %d, device count: %d, numberOfAdvDevices: %d;\n", reason, results.getCount(), advDevices.size());
    numberOfAdvDevices = advDevices.size();
    // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    if (advDevices.size() != 0 && !doConnect)
    {
      // NimBLEDevice::getScan()->stop();
      doConnect = true;
    }
  }
} scanCallbacks;*/

MyScanCallbacks myScanCallbacks;

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
  Serial.printf("%s\n", str.c_str());
  myBleArr[getIndexOfMyBleArr(pRemoteCharacteristic)].bleCollectPacket((char *)pData, length);
}

/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer()
{
  NimBLEClient *pClient = nullptr;

  // Serial.printf("advDevices.size() = %d\n", advDevices.size());
  // numberOfAdvDevices = advDevices.size();
  // Serial.printf("numberOfAdvDevices = %d\n", numberOfAdvDevices);

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
          Serial.printf("Reconnect failed\n");
          return false;
        }
        Serial.printf("Reconnected client\n");
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
        Serial.printf("Max clients reached - no more connections available\n");
        return false;
      }

      pClient = NimBLEDevice::createClient();

      Serial.printf("New client created\n");

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
        Serial.printf("Failed to connect, deleted client\n");
        myScanCallbacks.advDevices.clear();
        return false;
      }
    }

    if (!pClient->isConnected())
    {
      if (!pClient->connect(myScanCallbacks.advDevices.at(i)))
      {
        Serial.printf("Failed to connect\n");
        return false;
      }
    }

    Serial.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

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
      myBleArr[i].pChr_rx = pSvc->getCharacteristic(charUUID_rx);

      if (!myBleArr[i].pChr_rx)
      {
        Serial.printf("charUUID_rx not found.\n");
      }
      // pChr_tx = nullptr;
      // pChrSt.pChr_tx = nullptr;
      // pChr_tx = pSvc->getCharacteristic(charUUID_tx);
      // pChrSt.pChr_tx = pSvc->getCharacteristic(charUUID_tx);
      myBleArr[i].pChr_tx = pSvc->getCharacteristic(charUUID_tx);

      if (!myBleArr[i].pChr_tx)
      {
        Serial.printf("charUUID_tx not found.\n");
      }
      //
      // pChrs = &pSvc->getCharacteristics();
      //}
      // Serial.printf("pChrs->size() = %d\n", (int)pChrs->size());
      // if (pChrs)
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
          */

          if (myBleArr[i].pChr_rx->canNotify())
          {
            if (!myBleArr[i].pChr_rx->subscribe(true, notifyCB))
            {
              pClient->disconnect();
              return false;
            }
          }

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
          */
        }
        // pChrsV.push_back(pChrs);
        // pChrStV.push_back(pChrSt);
      }
    }
    else
    {
      Serial.printf("serviceUUID not found.\n");
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
  }
  myScanCallbacks.advDevices.clear();
  Serial.println("Done with this device!");
  return true;
}

void setup()
{
  Serial.begin(9600);
  Serial.printf("Starting NimBLE Client\n");

  /** Initialize NimBLE and set the device name */
  NimBLEDevice::init("NimBLE-Client");

  /**
   * Set the IO capabilities of the device, each option will trigger a different pairing method.
   *  BLE_HS_IO_KEYBOARD_ONLY   - Passkey pairing
   *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
   *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
   */
  // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY); // use passkey
  // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

  /**
   * 2 different ways to set security - both calls achieve the same result.
   *  no bonding, no man in the middle protection, BLE secure connections.
   *  These are the default values, only shown here for demonstration.
   */
  // NimBLEDevice::setSecurityAuth(false, false, true);

  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

  /** Optional: set the transmit power */
  NimBLEDevice::setPower(3); /** 3dbm */
  NimBLEScan *pScan = NimBLEDevice::getScan();

  /** Set the callbacks to call when scan events occur, no duplicates */
  pScan->setScanCallbacks(&myScanCallbacks, false);
  pScan->setScanCallbacks(&myScanCallbacks, false);

  /** Set scan interval (how often) and window (how long) in milliseconds */
  pScan->setInterval(100);
  pScan->setWindow(100);

  /**
   * Active scan will gather scan response data from advertisers
   *  but will use more energy from both devices
   */
  pScan->setActiveScan(true);

  /** Start scanning for advertisers */
  pScan->start(myScanCallbacks.scanTimeMs);
  Serial.printf("Scanning for peripherals\n");
}

void loop()
{
  /** Loop here until we find a device we want to connect to */
  delay(3000);
  Serial.printf("\n");
  if (myScanCallbacks.doConnect)
  {
    myScanCallbacks.doConnect = false;
    /** Found a device we want to connect to, do it now */
    if (connectToServer())
    {
      Serial.printf("Success! we should now be getting notifications.\n");
    }
    else
    {
      Serial.printf("Failed to connect, starting scan\n");
    }

    // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
  }
  //
  // uint8_t data[7] = {0xdd, 0xa5, 0x5, 0x0, 0xff, 0xfb, 0x77};
  // for (int j = 0; j < pChrsV.size(); j++)
  // for (int j = 0; j < pChrStV.size(); j++)
  char buff[256];
  String str;
  for (int j = 0; j < myScanCallbacks.numberOfAdvDevices; j++)
  {
    delay(2000);
    Serial.printf("\nmyBleArr[%d]========================\n", j);
    Serial.printf("main, deviceName = %s\n", myBleArr[j].deviceName.c_str());
    Serial.printf("main, packBasicInfo.Volts = %dmV\n", myBleArr[j].packBasicInfo.Volts);
    Serial.printf("main, packBasicInfo.Amps = %dmA\n", myBleArr[j].packBasicInfo.Amps);
    Serial.printf("main, packBasicInfo.CapacityRemainPercent = %d%%\n", myBleArr[j].packBasicInfo.CapacityRemainPercent);
    Serial.printf("main, packCellInfo.CellDiff = %dmV\n", myBleArr[j].packCellInfo.CellDiff);
    Serial.printf("===================================\n");
    // auto pChrs = pChrsV.at(j);
    // auto pChrS = pChrStV.at(j);
    /*
    for (int i = 0; i < pChrs->size(); i++)
    {
      if (pChrs->at(i)->canRead())
      {
        Serial.printf("canRead[%d, %d]: %s Value: %s\n", j, i, pChrs->at(i)->getUUID().toString().c_str(), pChrs->at(i)->readValue().c_str());
      }
      if (pChrs->at(i)->canWrite())
      {
        Serial.printf("canWrite[%d, %d]: %s Value: %s\n", j, i, pChrs->at(i)->getUUID().toString().c_str(), pChrs->at(i)->readValue().c_str());
      }
    }
    */
    // Serial.printf("pChrStV.at(j)tV[%d].pChr_rx: %s Value: %s\n", j, pChrStV.at(j).pChr_rx->getUUID().toString().c_str(), pChrStV.at(j).pChr_rx->readValue().c_str());
    //  pChrStV.at(j).pChr_tx->writeValue(data, sizeof(data), true);
    // myBLE.bmsGetInfo5(pChrStV.at(j).pChr_tx);
    if (myBleArr[j].pChr_tx)
    {
      sprintf(buff, "command to %s: Service = %s, Charastaric = %s\n",
              myBleArr[j].pChr_tx->getClient()->getPeerAddress().toString().c_str(),
              myBleArr[j].pChr_tx->getRemoteService()->getUUID().toString().c_str(),
              myBleArr[j].pChr_tx->getUUID().toString().c_str());
      myBleArr[j].bmsGetInfo5();
      str = "Send bmsGetInfo5 command to " + String(buff);
      Serial.print(str.c_str());
      delay(1000);
      Serial.printf("\n");
      if (myBleArr[j].toggle)
      {
        myBleArr[j].bmsGetInfo3();
        str = "Send bmsGetInfo3 command to " + String(buff);
        Serial.print(str.c_str());
      }
      else
      {
        myBleArr[j].bmsGetInfo4();
        str = "Send bmsGetInfo4 command to " + String(buff);
        Serial.print(str.c_str());
      }
    }
    else
      Serial.printf("myBleArr[%d].pChr_tx == null\n", j);
    /*Serial.printf("Send bmsGetInfo5 command to %s: Service = %s, Charastaric = %s\n",
                  pChrStV.at(j).pChr_tx->getClient()->getPeerAddress().toString().c_str(),
                  pChrStV.at(j).pChr_tx->getRemoteService()->getUUID().toString().c_str(),
                  pChrStV.at(j).pChr_tx->getUUID().toString().c_str());*/
    /*if (myBleArr[j].pChr_tx)
      Serial.printf("Send bmsGetInfo5 command to %s: Service = %s, Charastaric = %s\n",
                    myBleArr[j].pChr_tx->getClient()->getPeerAddress().toString().c_str(),
                    myBleArr[j].pChr_tx->getRemoteService()->getUUID().toString().c_str(),
                    myBleArr[j].pChr_tx->getUUID().toString().c_str());*/
  }
}