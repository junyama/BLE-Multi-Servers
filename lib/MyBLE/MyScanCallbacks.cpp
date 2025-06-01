#ifndef MY_SCAN_CPP
#define MY_SCAN_CPP

#include <NimBLEDevice.h>
#include "MyLog.cpp"

/** Define a class to handle the callbacks when scan events are received */
class MyScanCallbacks : public NimBLEScanCallbacks
{
private:
  const char *TAG = "MyScanCallbacks";

public:
  const NimBLEUUID serviceUUID = BLEUUID("0000ff00-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
  const NimBLEUUID charUUID_tx = BLEUUID("0000ff02-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
  const NimBLEUUID charUUID_rx = BLEUUID("0000ff01-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module

  uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
  std::vector<const NimBLEAdvertisedDevice *> advDevices;
  int numberOfAdvDevices = 0;
  bool doConnect = false;

  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override
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
        DEBUG_PRINT("onResult:advDevices.size()=%d\n", advDevices.size());
      }
      // END
    }
  }

  /** Callback to process the results of the completed scan or restart it */
  void onScanEnd(const NimBLEScanResults &results, int reason) override
  {
    DEBUG_PRINT("Scan Ended, reason: %d, device count: %d, numberOfAdvDevices: %d;\n", reason, results.getCount(), advDevices.size());
    numberOfAdvDevices = advDevices.size();
    // NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    if (advDevices.size() != 0 && !doConnect)
    {
      // NimBLEDevice::getScan()->stop();
      doConnect = true;
    }
  }
};

#endif /* MY_SCAN_CPP_ */