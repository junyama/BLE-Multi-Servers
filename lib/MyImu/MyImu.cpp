#include "MyImu.hpp"

MyImu::MyImu()
{
    M5.IMU.Init();
}

bool MyImu::update()
{
    // Stores the triaxial gyroscope data of the inertial sensor to the relevant variable
    M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
    M5.IMU.getAccelData(&accX, &accY, &accZ); // Stores the triaxial accelerometer.
    M5.IMU.getAhrsData(&pitch, &roll, &yaw);  // Stores the inertial sensor attitude.
    M5.IMU.getTempData(&temp);                // Stores the inertial sensor temperature to temp.

    return true; //tentativ
    
    if (gyroX_ != gyroX)
    {
        gyroX_ = gyroX;
        return true;
    }
    else
        return false;
}

JsonDocument MyImu::getState()
{
    JsonDocument doc;
    doc["update"] = (int)update();
    doc["accX"] = accX;
    doc["temperature"] = temp;
    return doc;
}

bool MyImu::timeout(unsigned long currentTime)
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