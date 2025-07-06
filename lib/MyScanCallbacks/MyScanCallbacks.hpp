#ifndef MY_SACN_CB_HPP
#define MY_SCAN_CB_HPP

#include <NimBLEDevice.h>
#include "MyLog.cpp"
#include "MyBLE2.hpp"
#include "MyM5.hpp"

/** Define a class to handle the callbacks when scan events are received */
class MyScanCallbacks : public NimBLEScanCallbacks
{
private:
  const char *TAG = "MyScanCallbacks";
  // MyBLE myBle;

public:
  const NimBLEUUID serviceUUID = BLEUUID("0000ff00-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
  const NimBLEUUID charUUID_tx = BLEUUID("0000ff02-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
  const NimBLEUUID charUUID_rx = BLEUUID("0000ff01-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module

  uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
  std::vector<const NimBLEAdvertisedDevice *> advDevices;
  int numberOfAdvDevices = 0;
  int *numberOfDevicesFound;
  bool doConnect = false;
  MyBLE2 *myBleArr;
  MyM5 *myM5;

  MyScanCallbacks(); 

  MyScanCallbacks(MyBLE2 *myBleArr_, MyM5 *myM5_, int *numberOfDevicesFound_);

  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;

  /** Callback to process the results of the completed scan or restart it */
  void onScanEnd(const NimBLEScanResults &results, int reason) override;
};

#endif /* MY_SACN_CB_HPP_ */
