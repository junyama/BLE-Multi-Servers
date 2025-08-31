#ifndef MY_LOG_HPP
#define MY_LOG_HPP

// White
#define DEBUG_PRINT(...)                                              \
    if (MyLog::DEBUG)                                                 \
    {                                                                 \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
    }

// Cyan
#define DEBUG2_PRINT(...)                                             \
    if (MyLog::DEBUG2)                                                \
    {                                                                 \
        Serial.print("\033[36m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
        Serial.print("\033[0m");                                      \
    }

// Magenda
#define DEBUG3_PRINT(...)                                             \
    if (MyLog::DEBUG3)                                                \
    {                                                                 \
        Serial.print("\033[35m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
        Serial.print("\033[0m");                                      \
    }

// Blue
#define DEBUG4_PRINT(...)                                             \
    if (MyLog::DEBUG4)                                                \
    {                                                                 \
        Serial.print("\033[34m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
        Serial.print("\033[0m");                                      \
    }

// Green
#define INFO_PRINT(...)                                               \
    if (MyLog::INFO)                                                  \
    {                                                                 \
        Serial.print("\033[32m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
        Serial.print("\033[0m");                                      \
    }

// Yellow
#define WARN_PRINT(...)                                               \
    if (MyLog::WARN)                                                  \
    {                                                                 \
        Serial.print("\033[33m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
        Serial.print("\033[0m");                                      \
    }

// Red
#define ERROR_PRINT(...)                                              \
    if (MyLog::ERROR)                                                 \
    {                                                                 \
        Serial.print("\033[31m");                                     \
        Serial.printf("[%s] %s: ", DateTime.toString().c_str(), TAG); \
        Serial.printf(__VA_ARGS__);                                   \
        Serial.print("\033[0m");                                      \
    }

#include <ESPDateTime.h>
#include <M5Core2.h>

class MyLog
{
public:
    static bool DEBUG;
    static bool DEBUG2;
    static bool DEBUG3;
    static bool DEBUG4;

    static bool INFO;
    static bool WARN;
    static bool ERROR;
};

#endif /* MY_LOG_HPP  */