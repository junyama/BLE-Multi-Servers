#ifndef MY_SACN_CB_HPP
#define MY_SCAN_CB_HPP

#include <NimBLEDevice.h>
#include "MyLog.cpp"
#include "MyBLE2.hpp"
#include "MyThermo.hpp"

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

  // const NimBLEUUID serviceUUID_thermo = BLEUUID("ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6"); // Xiaomi thermomater
  const NimBLEUUID serviceUUID_thermo = BLEUUID("31300c0e-ac7a-4de5-8ebe-779decafc200"); // Xiaomi thermomater
  const NimBLEUUID charUUID_thermo = BLEUUID("ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6");    // Xiaomi thermomater

  uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
  std::vector<const NimBLEAdvertisedDevice *> advDevices;
  std::vector<const NimBLEAdvertisedDevice *> advThermoDevices;

  int numberOfAdvDevices = 0;
  int numberOfAdvThermoDevices = 0;

  int *numberOfDevicesFound;
  int *numberOfThermoDevicesFound;

  bool doConnect = false;
  MyBLE2 *myBleArr;
  MyThermo *myThermoArr;

  MyM5 *myM5;

  MyScanCallbacks();

  MyScanCallbacks(MyBLE2 *myBleArr_, MyM5 *myM5_, int *numberOfDevicesFound_, MyThermo *myThermoArr, int *numberOfThermoDevicesFound_);

  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;

  /** Callback to process the results of the completed scan or restart it */
  void onScanEnd(const NimBLEScanResults &results, int reason) override;
};

#endif /* MY_SACN_CB_HPP_ */
