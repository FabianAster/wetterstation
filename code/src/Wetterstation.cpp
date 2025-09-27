#include "Wetterstation.hpp"
#include "Measurement.hpp"

// INFO: with the deepsleep, the esp restarts each time. Therefore the
// measurements will be up to date
Wetterstation::Wetterstation()
    : currentVoltage(this->readBatteryVoltage()),
      newMeasurement(this->currentVoltage),
      pub(MQTT_CLIENT_NAME, WIFI_SSID, WIFI_PASSWORD, MQTT_HOST, MQTT_USER,
          MQTT_PASSWORD) {
  Serial.begin(74880);
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("started");
}

void Wetterstation::run() {
  this->newMeasurement.messure();

  this->pub.init();
  this->pub.publish("Wetterstation/measurements",
                    this->newMeasurement.toJson().c_str());
  this->pub.run();

  Serial.println("entering deep sleep");
  ESP.deepSleep(5e6); // Deep sleep for 5 seconds
}

double Wetterstation::readBatteryVoltage() {
  int analogValue = analogRead(A0);
  double voltage = (analogValue / 1023.0) * 3.3;
  Serial.println(voltage);
  return voltage;
}
