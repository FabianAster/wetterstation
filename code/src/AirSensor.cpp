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
      updatePulseAnimation(0.3);
    }
    if (this->newMeasurement.airQualityAccuracy == 2) {
      updatePulseAnimation(0.1);
    }
    if (this->newMeasurement.airQualityAccuracy == 3) {
      this->setColor(this->currentColor);
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
  uint8_t r = 0, g = 0, b = 0;

  if (iaq <= 50) {
    // Green (0,255,0) to Yellow (255,255,0)
    t = iaq / 50.0f;
    r = (uint8_t)(t * 255);
    g = 255;
    b = 0;
  } else if (iaq <= 100) {
    // Yellow (255,255,0) to Orange (255,165,0)
    t = (iaq - 50) / 50.0f;
    r = 255;
    g = (uint8_t)(255 - t * (255 - 165));
    b = 0;
  } else if (iaq <= 250) {
    // Orange (255,165,0) to Red (255,0,0)
    t = (iaq - 100) / 50.0f;
    r = 255;
    g = (uint8_t)(165 - t * 165);
    b = 0;
  } else if (iaq <= 400) {
    // Red (255,0,0) to Violet (128,0,128) -- extend reddish range to 300
    t = (iaq - 150) / 150.0f;
    r = (uint8_t)(255 - t * (255 - 128));
    g = 0;
    b = (uint8_t)(t * 128);
  } else if (iaq <= 500) {
    // Violet (128,0,128) to Deep Red/Violet (128,0,255)
    t = (iaq - 300) / 100.0f;
    r = 128;
    g = 0;
    b = (uint8_t)(128 + t * (255 - 128));
  } else {
    // Deep Red/Violet (128,0,255) to Deep Red/Violet (255,0,255)
    t = (iaq - 400) / 200.0f;
    r = (uint8_t)(128 + t * (255 - 128));
    g = 0;
    b = 255;
  }

  this->currentColor = RgbColor{r, g, b};
}
