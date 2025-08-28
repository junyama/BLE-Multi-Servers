#ifndef MY_LOG_HPP
#define MY_LOG_HPP

#define DEBUG_PRINT(...) Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); Serial.printf(__VA_ARGS__)

#include <ESPDateTime.h>
#include <M5Core2.h>

class MyLog
{
public:
    static int verbose;
};

#endif /* MY_LOG_HPP  */