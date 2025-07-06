#include "MyM5.hpp"

MyM5::MyM5(int *numberOfBleDevices_) : numberOfBleDevices(numberOfBleDevices_)
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
    String topic_ = deviceObj["mqtt"]["topic"];
    DEBUG_PRINT("deviceObj[\"topic\"]: %s\n", topic_.c_str());
    if (topic_ != "null")
        topic = topic_;
}

bool MyM5::timeout(int currentTime)
{
    if ((currentTime - lastMeasurment) >= measurmentIntervalMs)
    {
        DEBUG_PRINT("millis() - lastMeasument: %d - %d >= measurmentIntervalMs: %d\n", currentTime, lastMeasurment, measurmentIntervalMs);
        lastMeasurment = currentTime;
        return true;
    }
    else
        return false;
}

void MyM5::powerSave(int status)
{
    if (status)
    {
        println("goint to sleap in 2 seconds");
        delay(2000);
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

JsonDocument MyM5::getState()
{
    JsonDocument doc;
    doc["lcdState"] = lcdState;
    doc["ledState"] = ledState;
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
        if (++bmsIndexShown > *numberOfBleDevices - 1)
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
        M5.Lcd.setTextSize(1);
        isBatteryInfoShown = false;
    }
    M5.Lcd.println(text);
}

void MyM5::updateBmsInfo(int bmsIndex, float volt, float current, float cellDiff, float temparature1, float temparature2, int capacityRemain)
{
    // LOGD(TAG, "showBatteryInfo called with bmsIndex: " + String(bmsIndex));
    // LOGD(TAG, "show volt: " + String(volt));
    // LOGD(TAG, "show current: " + String(current));

    bmsInfoArr[bmsIndex].volt = volt;
    bmsInfoArr[bmsIndex].current = current;
    bmsInfoArr[bmsIndex].cellDiff = cellDiff;
    bmsInfoArr[bmsIndex].temparature1 = temparature1;
    bmsInfoArr[bmsIndex].temparature2 = temparature2;
    bmsInfoArr[bmsIndex].capacityRemain = capacityRemain;

    showBatteryInfo();
}

void MyM5::showBatteryInfo()
{
    // LOGD(TAG, "showBatteryInfo() called with bmsIndex: " + String(bmsIndexShown));
    char str[16];
    M5.Lcd.clear();
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 1, 2);
    M5.Lcd.setTextColor(WHITE, BLACK);
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
