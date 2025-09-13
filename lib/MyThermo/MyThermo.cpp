#include "MyThermo.hpp"

MyThermo::MyThermo()
{
}

MyThermo::MyThermo(NimBLEAddress peerAddress_, String deviceName_) :
peerAddress(peerAddress_), mac(String(peerAddress_.toString().c_str())), deviceName(deviceName_)
{

}

void MyThermo::setup(JsonDocument deviceObj)
{
    if (deviceObj["measurmentIntervalMs"])
        measurmentIntervalMs = deviceObj["measurmentIntervalMs"];
}

bool MyThermo::timeout(unsigned long currentTime)
{
    unsigned long tempTime = lastMeasurment + measurmentIntervalMs;
    if (currentTime >= tempTime)
    {
        DEBUG_PRINT("currentTime(%lu) >= lastMeasurment + measurmentIntervalMs(%lu)\n", currentTime, tempTime);
        lastMeasurment = currentTime;
        return true;
    }
    else
        return false;
}

float MyThermo::processTempPacket(char *data, uint32_t dataSize)
{
    temp = (data[0] | (data[1] << 8)) * 0.1; // little endian
    DEBUG_PRINT("temperature: %.1f C\n", temp);
    if (temp < 1.0)
        WARN_PRINT("temperature: %.1f C\n", temp);
    return temp;
}

float MyThermo::processHumidPacket(char *data, uint32_t dataSize)
{
    humi = (data[0] | (data[1] << 8)) * 0.01; // little endian
    DEBUG_PRINT("humidity: %.1f %%\n", humi);
    if (humi < 1.0)
        WARN_PRINT("humidity: %.1f C\n", humi);
    return humi;
}

JsonDocument MyThermo::getState()
{
    JsonDocument doc;
    doc["deviceName"] = deviceName;
    doc["connected"] = (int)connected;
    doc["temperature"] = temp;
    doc["humidity"] = humi;
    return doc;
}