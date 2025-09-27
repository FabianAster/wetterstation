#include "Measurement.hpp"

Measurement::Measurement(double currentVoltage)
    : currentVoltage(currentVoltage) {

  while (1) {

    if (!bme.begin()) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      Serial.println("\n");
      delay(2000);
    }
  }

  bme_temp->printSensorDetails();
  bme_pressure->printSensorDetails();
  bme_humidity->printSensorDetails();
}

Measurement::Measurement()
    : currentVoltage(0), temperature(0), airPressure(0), humidity(0) {}

void Measurement::messure() {
  Serial.println("starting measurement");
  sensors_event_t temp_event, pressure_event, humidity_event;
  this->bme_temp->getEvent(&temp_event);
  this->bme_pressure->getEvent(&pressure_event);
  this->bme_humidity->getEvent(&humidity_event);

  this->temperature = temp_event.temperature;
  this->airPressure = temp_event.pressure;
  this->humidity = temp_event.relative_humidity;

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
