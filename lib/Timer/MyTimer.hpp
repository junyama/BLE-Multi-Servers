#ifndef MY_TIMER_HPP
#define MY_TIMER_HPP

#include "MyLog.cpp"

class MyTimer
{
private:
    const String TAG = "MyTimer";

public:
    unsigned long lastMeasurment = 0;
    int measurmentIntervalMs = 5000;

    MyTimer();
    MyTimer(unsigned long lastMeasurment_, int measurmentIntervalMs_);
    bool timeout(int currentTime);
};

#endif /* MY_TIMER_HPP  */