#include <FS.h>
#include <XPT2046_Touchscreen.h>

#ifndef _TOUCH_CONTROLLERWSH_
#define _TOUCH_CONTROLLERWSH_

typedef void (*CalibrationCallback)(int16_t x, int16_t y);

class TouchControllerWS {
  public:
    TouchControllerWS(XPT2046_Touchscreen *touchScreen);
    bool loadCalibration();
    bool saveCalibration();
    void startCalibration(CalibrationCallback *callback);
    void continueCalibration();
    bool isCalibrationFinished();
    bool isTouched();
    bool isTouched(int16_t debounceMillis);
    TS_Point getPoint();

  private:
    XPT2046_Touchscreen *touchScreen;
    float dx = 0.0;
    float dy = 0.0;
    int ax = 0;
    int ay = 0;
    int state = 0;
    long lastStateChange = 0;
    long lastTouched = 0;
    CalibrationCallback *calibrationCallback;
    TS_Point p1, p2;

};

#endif
