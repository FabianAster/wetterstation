#pragma once

#include "Measurement.hpp"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#define WIFI_CUTOFF_VOLTAGE 2.3
#define NUM_PIXELS 4
#define PIN_WS2812B D4

#define frameInterval 60

struct RgbColor {
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

class AirSensor {
public:
  AirSensor();
  void run();
  void setup();

private:
  Measurement newMeasurement;
  Publisher pub;
  void setColor(RgbColor color);
  Adafruit_NeoPixel WS2812B;
  void updateCurrentColor();
  void updatePulseAnimation(float speed);

  RgbColor currentColor;
  unsigned long lastUpdate = 0;
};
