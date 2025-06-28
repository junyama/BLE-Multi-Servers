#ifndef POWER_SAVING2_HPP
#define POWER_SAVING2_HPP

#include <M5Core2.h>
#include <Arduino.h>
#include "MyLog.cpp"
#include "MyLcd2.hpp"

#define MSG_BUFFER_SIZE (50)

class PowerSaving2
{
private:
    const String TAG = "PowerSaving";
    MyLcd2* myLcd;

public:
    int lcdState = 1;
    int ledState = 1;

    PowerSaving2(MyLcd2* myLcd_);
    //void loop();
    void enable();
    void disable();
    void detectButton(int numberOfPages);
};

#endif /* POWER_SAVING2_HPP */