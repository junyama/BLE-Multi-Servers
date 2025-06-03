
/** NimBLE_Client Demo:
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>

#include <M5Core2.h>

#include "PowerSaving2.hpp"
#include "MyLcd2.hpp"
#include "MyBLE.cpp"
#include "MyScanCallBacks.cpp"
#include "MyLog.cpp"
#include "MyTimer.cpp"
#include "MySdCard.hpp"

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
// static NimBLEUUID myScanCallbacks.charUUID_tx = BLEUUID("0000ff02-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
// static NimBLEUUID charUUID_rx = BLEUUID("0000ff01-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module

const char *TAG = "main";
JsonDocument configJson;
PowerSaving2 powerSaving;
MyLcd2 myLcd;
MyTimer myTimer(0, 10000);
MyTimer myTimerArr[3];

// MyBLE myBLE;
MyBLE myBleArr[3];

NimBLEClientCallbacks clientCallbacks;

MyScanCallbacks myScanCallbacks;

void loadConfig();
void saveConfig();

int getIndexOfMyBleArr(NimBLERemoteCharacteristic *pRemoteCharacteristic);

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);

/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer();

void detectButton(int numberOfPages);

void printBatteryInfo(MyBLE myBle);

void setup()
{
  M5.begin(); // Init M5Core2.
  Serial.begin(9600);
  MySdCard::deleteFile(SD, "/log.txt");
  MySdCard::setup();

  powerSaving.disable();
  DEBUG_PRINT("Starting NimBLE Client\n");

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
  // pScan->setScanCallbacks(&myScanCallbacks, false);

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
  DEBUG_PRINT("Scanning for peripherals\n");
}

void loop()
{
  detectButton(myScanCallbacks.numberOfAdvDevices);
  /** Loop here until we find a device we want to connect to */
  if (myScanCallbacks.doConnect)
  {
    myScanCallbacks.doConnect = false;
    DEBUG_PRINT("Found a device we want to connect to, do it now.\n");
    if (connectToServer())
    {
      DEBUG_PRINT("Success! we should now be getting notifications.\n");
    }
    else
    {
      DEBUG_PRINT("Failed to connect, goint to reset\n");
      // NimBLEDevice::getScan()->start(myScanCallbacks.scanTimeMs, false, true);
      delay(3000);
      M5.shutdown(1);
    }
  }
  else
  {
    // DEBUG_PRINT("Not Found a device yet and chck again or already foundd and skip to send commands\n");
  }

  // delay(3000);

  // DEBUG_PRINT("\n");
  for (int j = 0; j < myScanCallbacks.numberOfAdvDevices; j++)
  {
    if (myTimer.timeout(millis()))
    {
      printBatteryInfo(myBleArr[j]);
      myLcd.bmsInfoArr[j].deviceName = myBleArr[j].deviceName;
      myLcd.updateBmsInfo(j, myBleArr[j].packBasicInfo.Volts, myBleArr[j].packBasicInfo.Amps,
                          myBleArr[j].packCellInfo.CellDiff,
                          myBleArr[j].packBasicInfo.Temp1, myBleArr[j].packBasicInfo.Temp2,
                          myBleArr[j].packBasicInfo.CapacityRemainPercent);
    }

    if (myBleArr[j].pChr_tx)
      if (myTimerArr[j].timeout(millis()))
      {
        {
          char buff[256];
          sprintf(buff, "command to %s: Service = %s, Charastaric = %s\n",
                  myBleArr[j].pChr_tx->getClient()->getPeerAddress().toString().c_str(),
                  myBleArr[j].pChr_tx->getRemoteService()->getUUID().toString().c_str(),
                  myBleArr[j].pChr_tx->getUUID().toString().c_str());
          // myBleArr[j].bmsGetInfo5();
          String string = "Send bmsGetInfo5 command to " + String(buff) + "\n";
          // DEBUG_PRINT(string.c_str());
          // delay(1000);
          // DEBUG_PRINT("\n");
          if (myBleArr[j].toggle)
          {
            myBleArr[j].bmsGetInfo3();
            string = "Send bmsGetInfo3 command to " + String(buff) + "\n";
            DEBUG_PRINT(string.c_str());
          }
          else
          {
            myBleArr[j].bmsGetInfo4();
            string = "Send bmsGetInfo4 command to " + String(buff);
            DEBUG_PRINT(string.c_str());
          }
        }
      }
      else
      {
        // DEBUG_PRINT("myBleArr[%d].pChr_tx == null\n", j);
      }
  }
}