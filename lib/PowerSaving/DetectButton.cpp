#include "MyLog.cpp"
#include "PowerSaving2.hpp"
#include "MyLcd2.hpp"

extern char *TAG;
extern PowerSaving2 powerSaving;
extern MyLcd2 myLcd;

void detectButton(int numberOfPages)
{
  M5.update(); // Read the press state of the key.
  if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 200))
  {
    if (powerSaving.lcdState)
    {
      powerSaving.enable();
    }
    else
    {
      powerSaving.disable();
    }
  }
  else if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 200))
  {
    if (++myLcd.bmsIndexShown > numberOfPages - 1)
      myLcd.bmsIndexShown = 0;
    DEBUG_PRINT("Button B pushed with bmsIndex: %d", +myLcd.bmsIndexShown);
    myLcd.showBatteryInfo();
  }
  else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 200))
  {
    M5.shutdown();
  }
}