#include "MyLog.cpp"

class MyTimer
{
private:
    const String TAG = "MyTimer";
public:
    unsigned long lastMeasurment = 0;
    int measurmentIntervalMs = 5000;
    MyTimer()
    {
    }
    MyTimer(unsigned long lastMeasurment_, int measurmentIntervalMs_)
    {
        lastMeasurment = lastMeasurment_;
        measurmentIntervalMs = measurmentIntervalMs_;
    }
    bool timeout(int currentTime)
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
};