#include "MyM5.hpp"

MyM5::MyM5()
{
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(2);
    height = M5.Lcd.height();
    width = M5.Lcd.width();
}

void MyM5::setup(JsonDocument deviceObj)
{
    if (deviceObj["measurmentIntervalMs"])
        measurmentIntervalMs = deviceObj["measurmentIntervalMs"];
    if (deviceObj["resetIntervalSec"])
        resetIntervalSec = deviceObj["resetIntervalSec"];
    String topic_ = deviceObj["mqtt"]["topic"];
    DEBUG_PRINT("deviceObj[\"topic\"]: %s\n", topic_.c_str());
    if (topic_ != "null")
        topic = topic_;
}

bool MyM5::timeout(int currentTime)
{
    if ((currentTime - lastMeasurment) >= measurmentIntervalMs)
    {
        DEBUG_PRINT("millis() - lastMeasument: %ld - %ld >= measurmentIntervalMs: %d\n", currentTime, lastMeasurment, measurmentIntervalMs);
        lastMeasurment = currentTime;
        return true;
    }
    else
        return false;
}

bool MyM5::resetTimeout(int currentTime)
{
    if ((currentTime - lastReset) >= resetIntervalSec)
    {
        DEBUG_PRINT("getTime() - lastReset: %ld - %ld >= resetIntervalSec: %d\n", currentTime, lastReset, resetIntervalSec);
        lastReset = currentTime;
        return true;
    }
    else
        return false;
}

void MyM5::powerSave(int status)
{
    if (status)
    {
        println("sleeping in 3 sec");
        delay(3000);
        M5.Lcd.sleep();
        M5.Axp.SetLcdVoltage(0);
        lcdState = 0;
        M5.Axp.SetLed(0);
        ledState = 0;
    }
    else
    {
        M5.Lcd.wakeup();
        M5.Axp.SetLcdVoltage(3000);
        lcdState = 1;
        M5.Axp.SetLed(1);
        ledState = 1;
    }
}

void MyM5::lcdSwitch(int state)
{
    if (state)
    {
        M5.Lcd.wakeup();
        M5.Axp.SetLcdVoltage(3000);
        lcdState = 1;
    }
    else
    {
        M5.Lcd.sleep();
        M5.Axp.SetLcdVoltage(0);
        lcdState = 0;
    }
}

void MyM5::ledSwitch(int state)
{
    if (state)
    {
        M5.Axp.SetLed(1);
        ledState = 1;
    }
    else
    {
        M5.Axp.SetLed(0);
        ledState = 0;
    }
}

void MyM5::shutdown(int sec)
{
    M5.shutdown(sec);
}

void MyM5::reset()
{
    // esp_sleep_enable_timer_wakeup(1);
    // esp_deep_sleep(1);
    ESP.restart();
}

JsonDocument MyM5::getState()
{
    JsonDocument doc;
    doc["numberOfScan"] = numberOfScan;
    doc["numberOfPoi"] = numberOfPoi;
    doc["numberOfBleDevices"] = numberOfConnectedBMS;
    doc["numberOfThermoDevices"] = numberOfConnectedThermo;
    doc["lcdStatus"] = lcdState;
    doc["ledStatus"] = ledState;
    doc["voltage"] = vMaterLipoInfo.lipoVolt;
    doc["current"] = vMaterLipoInfo.lipoCurrent;
    return doc;
}

void MyM5::detectButton()
{
    M5.update(); // Read the press state of the key.
    if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 200))
    {
        if (lcdState)
        {
            lcdSwitch(0);
            ledSwitch(0);
        }
        else
        {
            lcdSwitch(1);
            ledSwitch(1);
        }
    }
    else if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 200))
    {
        if (++bmsIndexShown > numberOfConnectedBMS - 1)
            bmsIndexShown = 0;
        DEBUG_PRINT("Button B pushed with bmsIndex: %d", +bmsIndexShown);
        showBatteryInfo();
    }
    else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 200))
    {
        M5.shutdown();
    }
}

void MyM5::println(String text)
{
    if (M5.Lcd.getCursorY() > height - 10)
    {
        M5.Lcd.clear();
        M5.Lcd.setCursor(1, 1);
    }
    if (isBatteryInfoShown)
    {
        M5.Lcd.clear();
        M5.Lcd.setTextFont(1);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(1, 1);
        M5.Lcd.setTextColor(WHITE, BLACK);
        isBatteryInfoShown = false;
    }
    M5.Lcd.println(text);
}

void MyM5::createBmsInfoVec(String deviceName, String mac)
{
    BmsInfoStruct bmsInfo;
    bmsInfo.deviceName = deviceName;
    bmsInfo.mac = mac;
    bmsInfoVec.push_back(bmsInfo);
}

void MyM5::createThermoInfoVec(String deviceName, String mac)
{
    ThermoInfo thermoInfo;
    thermoInfo.deviceName = deviceName;
    thermoInfo.mac = mac;
    thermoInfoVec.push_back(thermoInfo);
}

void MyM5::updateBmsInfo(int bmsIndex, float volt, float current, float cellDiff, float temparature1, float temparature2, int capacityRemain)
{
    /*
    bmsInfoArr[bmsIndex].volt = volt;
    bmsInfoArr[bmsIndex].current = current;
    bmsInfoArr[bmsIndex].cellDiff = cellDiff;
    bmsInfoArr[bmsIndex].temparature1 = temparature1;
    bmsInfoArr[bmsIndex].temparature2 = temparature2;
    bmsInfoArr[bmsIndex].capacityRemain = capacityRemain;
    */

    bmsInfoVec[bmsIndex].volt = volt;
    bmsInfoVec[bmsIndex].current = current;
    bmsInfoVec[bmsIndex].cellDiff = cellDiff;
    bmsInfoVec[bmsIndex].temparature1 = temparature1;
    bmsInfoVec[bmsIndex].temparature2 = temparature2;
    bmsInfoVec[bmsIndex].capacityRemain = capacityRemain;

    showBatteryInfo();
}

void MyM5::updateThermoInfo(int index, float temparature, float humidity)
{
    thermoInfoVec[index].temparature = temparature;
    thermoInfoVec[index].humidity = humidity;
}

void MyM5::showBatteryInfo()
{
    char str[16];
    M5.Lcd.clear();
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 1, 2);
    M5.Lcd.setTextColor(WHITE, BLACK);

    /*
    M5.Lcd.print(bmsInfoArr[bmsIndexShown].deviceName);
    M5.Lcd.print("(" + bmsInfoArr[bmsIndexShown].mac + ")");
    M5.Lcd.setCursor(1, 17, 7);
    M5.Lcd.setTextColor(GREEN, BLACK);
    sprintf(str, "%05.2f", bmsInfoArr[bmsIndexShown].volt / 1000);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("V");
    sprintf(str, "%05.2f", bmsInfoArr[bmsIndexShown].current / 1000);
    M5.Lcd.setCursor(158, 17, 7);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("A");
    M5.Lcd.setCursor(46, 73, 7);
    sprintf(str, "%03.0f", bmsInfoArr[bmsIndexShown].cellDiff);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("mV");
    M5.Lcd.setCursor(202, 73, 7);
    sprintf(str, "%03d", bmsInfoArr[bmsIndexShown].capacityRemain);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("%");
    M5.Lcd.setCursor(1, 131, 7);
    sprintf(str, "%05.2f", bmsInfoArr[bmsIndexShown].temparature1 / 10);
    */

    M5.Lcd.print(bmsInfoVec[bmsIndexShown].deviceName);
    M5.Lcd.print("(" + bmsInfoVec[bmsIndexShown].mac + ")");
    M5.Lcd.setCursor(1, 17, 7);
    M5.Lcd.setTextColor(GREEN, BLACK);
    sprintf(str, "%05.2f", bmsInfoVec[bmsIndexShown].volt / 1000);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("V");
    sprintf(str, "%05.2f", bmsInfoVec[bmsIndexShown].current / 1000);
    M5.Lcd.setCursor(158, 17, 7);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("A");
    M5.Lcd.setCursor(46, 73, 7);
    sprintf(str, "%03.0f", bmsInfoVec[bmsIndexShown].cellDiff);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("mV");
    M5.Lcd.setCursor(202, 73, 7);
    sprintf(str, "%03d", bmsInfoVec[bmsIndexShown].capacityRemain);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("%");
    M5.Lcd.setCursor(1, 131, 7);
    sprintf(str, "%05.2f", bmsInfoVec[bmsIndexShown].temparature1 / 10);

    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("C");
    M5.Lcd.setCursor(158, 131, 7);
    sprintf(str, "%05.2f", vMaterLipoInfo.vMater);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("V");
    M5.Lcd.setCursor(1, 192, 7);
    sprintf(str, "%05.2f", vMaterLipoInfo.lipoVolt);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("V");
    M5.Lcd.setCursor(158, 192, 7);
    sprintf(str, "%05.2f", vMaterLipoInfo.lipoCurrent);
    M5.Lcd.print(str);
    M5.Lcd.setTextFont(4);
    M5.Lcd.print("A");
    isBatteryInfoShown = true;
}

void MyM5::updateVoltMaterInfo(float volt)
{
    vMaterLipoInfo.vMater = volt;
}

void MyM5::updateLipoInfo()
{
    vMaterLipoInfo.lipoVolt = M5.Axp.GetBatVoltage();
    vMaterLipoInfo.lipoCurrent = M5.Axp.GetBatCurrent();
}
