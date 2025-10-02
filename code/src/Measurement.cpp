#include "Measurement.hpp"

Measurement::Measurement(double currentVoltage)
    : currentVoltage(currentVoltage) {

  Serial.begin(74880);
  Wire.begin();
  while (!bme.begin()) {
    Serial.println("Could not find BME280I2C sensor!");
    delay(1000);
  }

  settings.tempOSR = BME280::OSR_X4;

  Serial.println("found valid sensor");
  bme.setSettings(this->settings);
}

Measurement::Measurement()
    : currentVoltage(0), temperature(0), airPressure(0), humidity(0) {}

void Measurement::messure() {
  Serial.println("starting measurement");

  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  this->temperature = temp;
  this->airPressure = pres;
  this->humidity = hum;

  Serial.println("Printing the measurements:");
  this->printMeasurement();
}

void Measurement::printMeasurement() {
  Serial.println("measurement:");
  Serial.println("temp: " + String(this->temperature));
  Serial.println("pressure: " + String(this->airPressure));
  Serial.println("humidity: " + String(this->humidity));
  Serial.println("currentVoltage: " + String(this->currentVoltage));
}

std::string Measurement::toJson() {
  return "{"
         "\"temp\": " +
         std::to_string(this->temperature) +
         ", "
         "\"pressure\": " +
         std::to_string(this->airPressure) +
         ", "
         "\"humidity\": " +
         std::to_string(this->humidity) +
         ", "
         "\"currentVoltage\": " +
         std::to_string(this->currentVoltage) + "}";
}
