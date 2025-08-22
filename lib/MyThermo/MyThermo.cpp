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

void MyThermo::processPacket(char *data, uint32_t dataSize)
{
    freeL = 0;
    temp = (data[0] | (data[1] << 8)) * 0.01; // little endian
    humi = data[2];
    voltage = (data[3] | (data[4] << 8)) * 0.001; // little endian
    DEBUG_PRINT("temp = %.1f C ; humidity = %.1f %% ; voltage = %.3f V\n", temp, humi, voltage);
    freeL = 1;
}

JsonDocument MyThermo::getState()
{
}