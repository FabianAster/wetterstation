#include "publisher.hpp"

// Global pointer definition
Publisher *g_publisher_instance = nullptr;

// Global callback definition
void globalMqttCallback(char *topic, byte *payload, unsigned int length) {
  if (g_publisher_instance) {
    g_publisher_instance->callBack(topic, payload, length);
  }
}

Publisher::Publisher(const char *client_name, const char *ssid,
                     const char *password, const char *mqtt_server)
    : client_name(client_name), client(espClient), ssid(ssid),
      password(password), mqtt_server(mqtt_server) {
  logInfo("Publisher constructed (no user/pass)");
}

Publisher::Publisher(const char *client_name, const char *ssid,
                     const char *password, const char *mqtt_server,
                     const char *mqtt_user, const char *mqtt_pass)
    : client_name(client_name), client(espClient), ssid(ssid),
      password(password), mqtt_server(mqtt_server), mqtt_user(mqtt_user),
      mqtt_pass(mqtt_pass) {
  logInfo("Publisher constructed (with user/pass)");
}

void Publisher::setup_wifi() {
  logInfo("Setting up WiFi...");
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tries++;
    if (tries % 5 == 0) logInfo("Waiting for WiFi connection...");
  }

  logInfo("WiFi connected.");
  randomSeed(micros());
}

void Publisher::callBack(char *topic, byte *payload, unsigned int length) {
  logInfo("callBack triggered");
  std::string message(reinterpret_cast<char *>(payload), length);
  std::string topicStr(topic);

  for (const auto &sub : subscriptions) {
    if (sub.first == topicStr) {
      logInfo("Subscription callback matched topic");
      sub.second(message);
    }
  }
}

void Publisher::init() {
  logInfo("Initializing Publisher...");
  g_publisher_instance = this;
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(globalMqttCallback);

  if (this->mqtt_user && this->mqtt_pass && strlen(this->mqtt_user) > 0 &&
      strlen(this->mqtt_pass) > 0) {
    logInfo("Connecting to MQTT with user/pass");
    client.connect(this->client_name, this->mqtt_user, this->mqtt_pass);
  } else {
    logInfo("Connecting to MQTT without user/pass");
    client.connect(this->client_name);
  }
}

void Publisher::reconnect() {
  logInfo("reconnect called");
}

void Publisher::run() {
  if (!client.connected()) {
    logInfo("MQTT not connected, reconnecting...");
    reconnect();
  }
  client.loop();
  logInfo("MQTT client looped");
}

void Publisher::publish(const char *topic, const char *message) {
  logInfo("Publishing message...");
  client.publish(topic, message);
}

void Publisher::subscribe(const std::string &topic,
                          std::function<void(const std::string &)> callback) {
  logInfo("Subscribing to topic...");
  subscriptions.emplace_back(topic, callback);
  client.subscribe(topic.c_str());
}

void Publisher::logInfo(const char *msg) {
  if (this->debug_log) {
    Serial.println(msg);
  }
}
