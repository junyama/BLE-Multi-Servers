#ifndef VOLT_MATER_CPP
#define VOLT_MATER_CPP

#include "LipoMater.hpp"

void LipoMater::setup(JsonDocument deviceObj)
{
    available = true;
    if (deviceObj["measurmentIntervalMs"])
        measurmentIntervalMs = deviceObj["measurmentIntervalMs"];
    String topic_ = deviceObj["mqtt"]["topic"];
    DEBUG_PRINT("deviceObj[\"topic\"]: %s\n", topic_.c_str());
    if (topic_ != "null")
        topic = topic_;
}

bool LipoMater::timeout(int currentTime)
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

JsonDocument LipoMater::getState()
{
    JsonDocument doc;
    if (available)
    {
        voltage = M5.Axp.GetBatVoltage();
        doc["voltage"] = voltage;
        current = M5.Axp.GetBatCurrent();
        doc["current"] = current;
    }
    else
    {
        doc["voltage"] = 0;
        doc["current"] = 0;
    }
    return doc;
}

#endif /* LIPO_MATER_CPP */