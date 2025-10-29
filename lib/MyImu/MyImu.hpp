#ifndef MY_IMU_HPP
#define MY_IMU_HPP

#include <ArduinoJson.h>

#include "MyLog.hpp"

class MyImu
{
private:
    const char *TAG = "MyImu";
    float accX = 0.0F; // Define variables for storing inertial sensor data
    float accY = 0.0F;
    float accZ = 0.0F;
    float gyroX = 0.0F;
    float gyroY = 0.0F;
    float gyroZ = 0.0F;
    float pitch = 0.0F;
    float roll = 0.0F;
    float yaw = 0.0F;
    float temp = 0.0F;

    float gyroX_ = 0.0F;

public:
    int measurmentIntervalMs = 2000;
    unsigned long lastMeasurment = 0;

    MyImu();
    bool update();
    JsonDocument getState();

    bool timeout(unsigned long currentTime);
};

#endif /* MY_IMU_HPP */