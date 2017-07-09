#include "TouchController.h"

TouchController::TouchController(XPT2046_Touchscreen *touchScreen) {
  this->touchScreen = touchScreen;
}

bool TouchController::loadCalibration() {
  // always use this to "mount" the filesystem
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);

  // this opens the file "f.txt" in read-mode
  File f = SPIFFS.open("/calibration.txt", "r");

  if (!f) {
    return false;
  } else {

      //Lets read line by line from the file
      String dxStr = f.readStringUntil('\n');
      String dyStr = f.readStringUntil('\n');
      String axStr = f.readStringUntil('\n');
      String ayStr = f.readStringUntil('\n');

      dx = dxStr.toFloat();
      dy = dyStr.toFloat();
      ax = axStr.toInt();
      ay = ayStr.toInt();

  }
  f.close();
  
}

bool TouchController::saveCalibration() {
  bool result = SPIFFS.begin();

  // open the file in write mode
  File f = SPIFFS.open("/calibration.txt", "w");
  if (!f) {
    Serial.println("file creation failed");
  }
  // now write two lines in key/value style with  end-of-line characters
  f.println(dx);
  f.println(dy);
  f.println(ax);
  f.println(ay);

  f.close();
}

void TouchController::startCalibration(CalibrationCallback *calibrationCallback) {
  state = 0;
  this->calibrationCallback = calibrationCallback;
}

void TouchController::continueCalibration() {
    TS_Point p = touchScreen->getPoint();

    if (state == 0) {
      (*calibrationCallback)(10, 10);
      if (touchScreen->touched()) {
        p1 = p;
        state++;
        lastStateChange = millis();
      }

    } else if (state == 1) {
      (*calibrationCallback)(230, 310);
      if (touchScreen->touched() && (millis() - lastStateChange > 1000)) {
        
        p2 = p;
        state++;
        lastStateChange = millis();
        dx = 240.0 / abs(p1.y - p2.y);
        dy = 320.0 / abs(p1.x - p2.x);
        ax = p1.y < p2.y ? p1.y : p2.y;
        ay = p1.x < p2.x ? p1.x : p2.x;
      }

    }
}
bool TouchController::isCalibrationFinished() {
  return state == 2;
}

bool TouchController::isTouched() {
  touchScreen->touched();
}

bool TouchController::isTouched(int16_t debounceMillis) {
  if (touchScreen->touched() && millis() - lastTouched > debounceMillis) {
    lastTouched = millis();
    return true;
  }
  return false;
}
TS_Point TouchController::getPoint() {
    TS_Point p = touchScreen->getPoint();
    int x = (p.y - ax) * dx;
    int y = 320 - (p.x - ay) * dy;
    p.x = x;
    p.y = y;
    return p;
}

