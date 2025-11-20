#pragma once

#include <Arduino.h>
#include "publisher.hpp"
#include "secrets.h"
#include <SPI.h>
#include <cstring>
#include <string>

#include <Wire.h>
#include "bsec.h"

#define SERIAL_BAUD 74880

class Measurement {
public:
  Measurement();

  // Call once from setup()
  void setup();

  // Call regularly from loop()
  void measure();

  // Debug print
  void printMeasurement();

  boolean newMeasurement = false;

  // Last measured values
  double temperature;          // °C (heater-compensated)
  double airPressure;          // hPa
  double humidity;             // %RH
  double voc;                  // gas resistance (Ohm)
  double airQuality;           // IAQ index (0–500, BSEC)
  double co2;                  // CO₂ equivalent (ppm)
  double breathVoc;            // bVOC equivalent (ppm)
  uint8_t airQualityAccuracy;  // 0–3

  std::string toJson();

private:
  Bsec iaqSensor;

  void checkIaqSensorStatus();
};
