#ifndef MY_LOG_HPP
#define MY_LOG_HPP

// White
#define DEBUG_PRINT(...)                                              \
    if (MyLog::DEBUG)                                                 \
    {                                                                 \
        Serial.print("\033[0;0m");                                      \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

// Cyan
#define DEBUG2_PRINT(...)                                             \
    if (MyLog::DEBUG2)                                                \
    {                                                                 \
        Serial.print("\033[0;36m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

// Magenda
#define DEBUG3_PRINT(...)                                             \
    if (MyLog::DEBUG3)                                                \
    {                                                                 \
        Serial.print("\033[0;35m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

// Blue
#define DEBUG4_PRINT(...)                                             \
    if (MyLog::DEBUG4)                                                \
    {                                                                 \
        Serial.print("\033[1;34m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

// Green
#define INFO_PRINT(...)                                               \
    if (MyLog::INFO)                                                  \
    {                                                                 \
        Serial.print("\033[0;32m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

// Yellow
#define WARN_PRINT(...)                                               \
    if (MyLog::WARN)                                                  \
    {                                                                 \
        Serial.print("\033[0;33m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

// Red
#define ERROR_PRINT(...)                                              \
    if (MyLog::ERROR)                                                 \
    {                                                                 \
        Serial.print("\033[0;31m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

#include <ESPDateTime.h>
#include <M5Core2.h>

class MyLog
{
public:
    static const bool DEBUG;
    static const bool DEBUG2;
    static const bool DEBUG3;
    static const bool DEBUG4;

    static const bool INFO;
    static const bool WARN;
    static const bool ERROR;
};

#endif /* MY_LOG_HPP  */