#include "AirSensor.hpp"
#include <Arduino.h>

AirSensor AirSensor;

void setup() { AirSensor.setup(); }

void loop() {
  AirSensor.run();
  delay(10);
}
