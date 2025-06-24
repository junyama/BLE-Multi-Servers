#include "MyTimer.hpp"

MyTimer::MyTimer()
{
}

MyTimer::MyTimer(unsigned long lastMeasurment_, int measurmentIntervalMs_)
{
    lastMeasurment = lastMeasurment_;
    measurmentIntervalMs = measurmentIntervalMs_;
}

bool MyTimer::timeout(int currentTime)
{
    if ((currentTime - lastMeasurment) >= measurmentIntervalMs)
    {
        DEBUG_PRINT("currentTime(%d) - lastMeasument(%d) >= measurmentIntervalMs(%d)\n", currentTime, lastMeasurment, measurmentIntervalMs);
        lastMeasurment = currentTime;
        return true;
    }
    else
        return false;
}