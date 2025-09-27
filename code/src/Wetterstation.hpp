#pragma once

#include "Measurement.hpp"
#include <Arduino.h>

#define WIFI_CUTOFF_VOLTAGE 2.3

class Wetterstation {
public:
  Wetterstation();
  void run();

private:
  double readBatteryVoltage();
  double currentVoltage;
  Measurement newMeasurement;
  Publisher pub;
};
