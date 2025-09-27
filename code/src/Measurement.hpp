#pragma once

#include "publisher.hpp"
#include "secrets.h"
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <cstring>
#include <string>

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
  Adafruit_BME280 bme; // use I2C interface
  Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
  Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
  Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();
};
