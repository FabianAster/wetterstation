#include "Measurement.hpp"

Measurement::Measurement() {
  Serial.begin(SERIAL_BAUD);
  Serial.println();
  Serial.println("Initializing BME680 + BSEC...");

  temperature = airPressure = humidity = voc = airQuality = co2 = breathVoc =
      0.0;
  airQualityAccuracy = 0;
}

void Measurement::setup() {
  // I2C
  Wire.begin();

  // Choose I2C address:
  //   BME68X_I2C_ADDR_HIGH -> 0x77 (SDO = VCC or floating)
  //   BME68X_I2C_ADDR_LOW  -> 0x76 (SDO = GND)
  iaqSensor.begin(BME68X_I2C_ADDR_HIGH, Wire);
  // If that fails, change to BME68X_I2C_ADDR_LOW above.

  // Print BSEC version
  Serial.print("BSEC library version ");
  Serial.print(iaqSensor.version.major);
  Serial.print(".");
  Serial.print(iaqSensor.version.minor);
  Serial.print(".");
  Serial.print(iaqSensor.version.major_bugfix);
  Serial.print(".");
  Serial.println(iaqSensor.version.minor_bugfix);

  checkIaqSensorStatus();

  // Tell BSEC which “virtual sensors” we want
  bsec_virtual_sensor_t sensorList[] = {
      BSEC_OUTPUT_RAW_TEMPERATURE,
      BSEC_OUTPUT_RAW_PRESSURE,
      BSEC_OUTPUT_RAW_HUMIDITY,
      BSEC_OUTPUT_RAW_GAS,
      BSEC_OUTPUT_IAQ,
      BSEC_OUTPUT_STATIC_IAQ,
      BSEC_OUTPUT_CO2_EQUIVALENT,
      BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(
      sensorList, sizeof(sensorList) / sizeof(sensorList[0]),
      BSEC_SAMPLE_RATE_LP // ~3s, good for room monitoring
  );
  checkIaqSensorStatus();

  Serial.println("BME680 + BSEC initialized");
}

void Measurement::measure() {
  // Must be called frequently (e.g. every 1s in loop()).
  // BSEC schedules measurements internally and returns true
  // only when a fresh data set is ready.
  if (iaqSensor.run()) {
    // Heater-compensated values
    temperature = iaqSensor.temperature; // °C
    humidity = iaqSensor.humidity;       // %RH

    // BSEC gives pressure in Pa; convert to hPa
    airPressure = iaqSensor.pressure / 100.0; // hPa

    // Raw gas resistance in Ohms
    voc = iaqSensor.gasResistance;

    // IAQ index 0–500 (lower is better)
    airQuality = iaqSensor.iaq;
    airQualityAccuracy = iaqSensor.iaqAccuracy;

    // Derived estimates
    co2 = iaqSensor.co2Equivalent;             // ppm
    breathVoc = iaqSensor.breathVocEquivalent; // ppm

    printMeasurement();
    this->newMeasurement = true;
  } else {
    // No new data; still keep an eye on status
    checkIaqSensorStatus();
  }
}

void Measurement::printMeasurement() {
  Serial.println("measurement:");
  Serial.print("  temp:      ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("  pressure:  ");
  Serial.print(airPressure);
  Serial.println(" hPa");

  Serial.print("  humidity:  ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("  gas (VOC): ");
  Serial.print(voc);
  Serial.println(" Ω");

  Serial.print("  IAQ:       ");
  Serial.print(airQuality);
  Serial.print(" (acc=");
  Serial.print(airQualityAccuracy);
  Serial.println(")");

  Serial.print("  eCO2:      ");
  Serial.print(co2);
  Serial.println(" ppm");

  Serial.print("  bVOC eq:   ");
  Serial.print(breathVoc);
  Serial.println(" ppm");
}

std::string Measurement::toJson() {
  std::string json = "{";

  json += "\"temp\": " + std::to_string(temperature) + ", ";
  json += "\"pressure\": " + std::to_string(airPressure) + ", ";
  json += "\"humidity\": " + std::to_string(humidity) + ", ";
  json += "\"voc\": " + std::to_string(voc) + ", ";
  json += "\"airQuality\": " + std::to_string(airQuality) + ", ";
  json += "\"airQualityAccuracy\": " +
          std::to_string(static_cast<int>(airQualityAccuracy)) + ", ";
  json += "\"co2\": " + std::to_string(co2) + ", ";
  json += "\"bVoc\": " + std::to_string(breathVoc);

  json += "}";

  return json;
}

void Measurement::checkIaqSensorStatus() {
  // BSEC algorithm status
  if (iaqSensor.bsecStatus != BSEC_OK) {
    Serial.print("BSEC status: ");
    Serial.println(iaqSensor.bsecStatus);

    if (iaqSensor.bsecStatus < BSEC_OK) {
      Serial.println("BSEC error, halting.");
      while (true) {
        delay(1000);
      }
    }
  }

  // BME68x sensor status
  if (iaqSensor.bme68xStatus != BME68X_OK) {
    Serial.print("BME68X status: ");
    Serial.println(iaqSensor.bme68xStatus);

    if (iaqSensor.bme68xStatus < BME68X_OK) {
      Serial.println("BME68X error, halting.");
      while (true) {
        delay(1000);
      }
    }
  }
}
