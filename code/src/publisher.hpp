#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <functional>
#include <string>
#include <vector>

class Publisher {
public:
  Publisher(const char *client_name, const char *ssid, const char *password,
            const char *mqtt_server);
  Publisher(const char *client_name, const char *ssid, const char *password,
            const char *mqtt_server, const char *mqtt_user,
            const char *mqtt_pass);
  void init();
  void run();
  void publish(const char *topic, const char *message);
  void subscribe(const std::string &topic,
                 std::function<void(const std::string &)> callback);
  void callBack(char *topic, byte *payload, unsigned int length);

private:
  const char *ssid;
  const char *password;
  const char *mqtt_server;
  const char *mqtt_pass;
  const char *mqtt_user;
  const char *client_name;

  const bool debug_log = true;

  std::vector<std::pair<std::string, std::function<void(const std::string &)>>>
      subscriptions;

  void setup_wifi();
  WiFiClient espClient;
  PubSubClient client;

  void reconnect();

  void logInfo(const char *msg);
};

// Global pointer declaration
extern Publisher *g_publisher_instance;

// Global callback declaration
void globalMqttCallback(char *topic, byte *payload, unsigned int length);

// Eample:
//
//
/* Publisher pub("Wetterstation test", "AchtungLenaNuisWlan", "123581321", */
/*               "10.0.0.15", "root", "&Oftv$^%05QQ7&"); */

/* void setup() { */
/*   pub.init(); */
/*   pub.subscribe("test/halloBack", */
/*                 [](const std::string &msg) { Serial.println(msg.c_str()); });
 */
/* } */

/* void loop() { */
/*   pub.run(); */
/*   pub.publish("test", "Hallo"); */
/* } */
