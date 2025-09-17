#ifndef MY_SCAN_CB_HPP
#define MY_SCAN_CB_HPP

#include <NimBLEDevice.h>
#include "MyLog.hpp"
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
  const NimBLEUUID serviceUUID2 = BLEUUID("FF00"); // xiaoxiang bms original module
  const NimBLEUUID charUUID_tx = BLEUUID("0000ff02-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
  const NimBLEUUID charUUID_rx = BLEUUID("0000ff01-0000-1000-8000-00805f9b34fb"); // xiaoxiang bms original module
  
  //const NimBLEUUID serviceUUID_thermo = BLEUUID("00010203-0405-0607-0809-0a0b0c0d1912"); // Xiaomi thermomater
  const NimBLEUUID serviceDataUUID_thermo = BLEUUID("181A"); // Xiaomi thermomater
  //const NimBLEUUID serviceUUID_thermo = BLEUUID("EBE0CCB0-7A0A-4B0C-8A1A-6FF2997DA3A6"); // Xiaomi thermomater
  //const NimBLEUUID charUUID_thermo_rx = BLEUUID("EBE0CCB0-7A0A-4B0C-8A1A-6FF2997DA3A6"); // Xiaomi thermomater
  const NimBLEUUID charUUID_thermo_temp = BLEUUID("2A1F"); // Xiaomi thermomater
  const NimBLEUUID charUUID_thermo_temp2 = BLEUUID("2A1E"); // Xiaomi thermomater
  const NimBLEUUID charUUID_thermo_humid = BLEUUID("2A6F"); // Xiaomi thermomater
  //const NimBLEUUID serviceUUID_thermo = BLEUUID("31300c0e-ac7a-4de5-8ebe-779decafc200"); // FlicHub
  //const NimBLEUUID charUUID_thermo_tx = BLEUUID("31300C0E-AC7A-4DE5-8EBE-779DECAFC201"); // FlicHub
  //const NimBLEUUID charUUID_thermo_rx = BLEUUID("31300C0E-AC7A-4DE5-8EBE-779DECAFC202"); // FlicHub

  uint32_t scanTimeMs = 10000; /** scan time in milliseconds, 0 = scan forever */
  std::vector<const NimBLEAdvertisedDevice *> advDevices;
  std::vector<const NimBLEAdvertisedDevice *> advThermoDevices;

  //int numberOfBMS = 0;
  //int numberOfThermo = 0;

  //int *numberOfDevicesFound;
  //int *numberOfThermoDevicesFound;

  bool doConnect = false;
  bool doConnectThermo = false;

  //bool doRescan = false;

  //MyBLE2 *myBleArr;
  //MyThermo *myThermoArr;

  std::vector<MyBLE2> bleDevices;
  std::vector<MyThermo> thermoDevices;

  MyM5 *myM5;

  MyScanCallbacks();

  MyScanCallbacks(MyM5 *myM5_);

  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;

  /** Callback to process the results of the completed scan or restart it */
  void onScanEnd(const NimBLEScanResults &results, int reason) override;
};

#endif /* MY_SACN_CB_HPP_ */
