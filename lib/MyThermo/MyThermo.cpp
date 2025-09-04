#include "MyThermo.hpp"

MyThermo::MyThermo()
{
}

void MyThermo::setup(JsonDocument deviceObj)
{
    if (deviceObj["measurmentIntervalMs"])
        measurmentIntervalMs = deviceObj["measurmentIntervalMs"];
}

bool MyThermo::timeout(int currentTime)
{
    if ((currentTime - lastMeasurment) >= measurmentIntervalMs)
    {
        DEBUG_PRINT("millis() - lastMeasument: %d - %d >= measurmentIntervalMs: %d\n", currentTime, lastMeasurment, measurmentIntervalMs);
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
    doc["temperature"] = temp;
    doc["humidity"] = humi;
    return doc;
}