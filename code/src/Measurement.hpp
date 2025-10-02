#pragma once

#include "publisher.hpp"
#include "secrets.h"
#include <SPI.h>
#include <cstring>
#include <string>

#include <BME280I2C.h>
#include <Wire.h> // Needed for legacy versions of Arduino.
#define SERIAL_BAUD 74880

class Measurement {
public:
  Measurement(double currentVoltage);
  Measurement();
  void messure();
  void printMeasurement();
  double currentVoltage;
  double temperature;
  double airPressure;
  double humidity;
  std::string toJson();

private:
  BME280I2C::Settings settings = {
      BME280::OSR_X1,
      BME280::OSR_X1,
      BME280::OSR_X1,
      BME280::Mode_Forced,
      BME280::StandbyTime_1000ms,
      BME280::Filter_Off,
      BME280::SpiEnable_False,
      BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.
  };

  BME280I2C bme{settings};
};
