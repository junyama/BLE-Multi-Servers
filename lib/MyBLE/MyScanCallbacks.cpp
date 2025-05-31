#ifndef MY_SCAN_CPP
#define MY_SCAN_CPP

#include <NimBLEDevice.h>

/** Define a class to handle the callbacks when scan events are received */
class MyScanCallbacks : public NimBLEScanCallbacks
{
public:
  const NimBLEUUID serviceUUID = BLEUUID("0000ff00-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
  
  uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
  std::vector<const NimBLEAdvertisedDevice *> advDevices;
  int numberOfAdvDevices = 0;
  bool doConnect = false;

  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override
  {
    Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
    if (advertisedDevice->isAdvertisingService(serviceUUID))
    {
      Serial.printf("Found Our Service\n");
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

  /** Callback to process the results of the completed scan or restart it */
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
};

#endif /* MY_SCAN_CPP_ */