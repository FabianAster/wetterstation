#include "AirSensor.hpp"

AirSensor::AirSensor()
    : newMeasurement(), pub(MQTT_CLIENT_NAME, WIFI_SSID, WIFI_PASSWORD,
                            MQTT_HOST, MQTT_USER, MQTT_PASSWORD),
      WS2812B(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800),
      currentColor{0, 0, 0} {
  Serial.begin(74880);
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("started");
}

void AirSensor::run() {
  this->newMeasurement.measure();
  if (this->newMeasurement.newMeasurement) {
    this->pub.publish("Wetterstation/measurements/AirSensorZimmer",
                      this->newMeasurement.toJson().c_str());
    this->pub.run();
    this->newMeasurement.newMeasurement = false;
  }

  updateCurrentColor();

  // update color
  unsigned long now = millis();
  if (now - lastUpdate >= frameInterval) {
    lastUpdate = now;
    if (this->newMeasurement.airQualityAccuracy == 0) {
      this->currentColor = RgbColor{0, 0, 255};
      updatePulseAnimation(0.2);
    }
    if (this->newMeasurement.airQualityAccuracy == 1) {
      this->setColor(this->currentColor);
    }
    if (this->newMeasurement.airQualityAccuracy == 2) {
      updatePulseAnimation(0.2);
    }
    if (this->newMeasurement.airQualityAccuracy == 3) {
      updatePulseAnimation(0.5);
    }
    WS2812B.show();
  }
}

void AirSensor::setup() {
  this->pub.init();
  this->newMeasurement.setup();
  WS2812B.begin(); // INITIALIZE WS2812B strip object (REQUIRED)
  WS2812B.clear(); // set all pixel colors to 'off'. It only takes effect if
                   // pixels.show() is called
  WS2812B.show();
  Serial.println("init of air sesnor done");
}

void AirSensor::setColor(RgbColor color) {
  for (int pixel = 0; pixel < NUM_PIXELS; pixel++) { // for each pixel
    WS2812B.setPixelColor(pixel, WS2812B.Color(color.R, color.G, color.B));
  }
  WS2812B.show();
}

void AirSensor::updatePulseAnimation(float speed) {
  float t = millis() * (speed / 1000);
  float brightness = 0.8 * (0.5 + 0.5 * sin(t * 2 * PI)); // 0.2 to 1.0

  RgbColor pulseColor{uint8_t(this->currentColor.R * brightness),
                      uint8_t(this->currentColor.G * brightness),
                      uint8_t(this->currentColor.B * brightness)};

  this->setColor(pulseColor);
}

void AirSensor::updateCurrentColor() {
  int iaq = constrain(this->newMeasurement.airQuality, 0, 500);

  float t;
  uint8_t r, g, b = 0;

  if (iaq <= 300) {
    t = iaq / 300.0f;
    r = (uint8_t)(t * 255);
    g = (uint8_t)((1.0f - t) * 255);
  } else {
    r = 255;
    g = 0;
  }

  this->currentColor = RgbColor{r, g, b};
}
