#ifndef POWER_SAVING2_HPP
#define POWER_SAVING2_HPP

//#include "M5Core2.h"
#include <M5Unified.h>
#include <Arduino.h>

//#include "MyDebug.hpp"

#define MSG_BUFFER_SIZE (50)

class PowerSaving2
{
private:
    const String TAG = "PowerSaving";

public:
    int lcdState = 1;
    int ledState = 1;

    PowerSaving2();
    //void loop();
    void enable();
    void disable();
};

#endif /* POWER_SAVING2_HPP */