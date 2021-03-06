#ifndef MAIN_H
#define MAIN_H

// OTHER SETTINGS
const byte mqttDebug = 1;

// MQTT Settings
typedef struct
{
  char *mqtt_single_mode_topic = (char *)"/singleMode";
  char *mqtt_state_topic = (char *)"/state";
  char *mqtt_brightness_topic = (char *)"/brightness";
  char *mqtt_fade_topic = (char *)"/fade/state";
  char *mqtt_fade_lower_boundary_topic = (char *)"/fade/lowerBoundary";
  char *mqtt_fade_upper_boundary_topic = (char *)"/fade/upperBoundary";
  char *mqtt_spread_topic = (char *)"/spread";
  char *mqtt_status_topic = (char *)"/status";
  char *mqtt_log_topic = (char *)"/log";
} TOPICS;

typedef struct
{
  bool state = true;
  bool singleMode = false;
  uint8_t brightnessSingle[4] = {5, 10, 15, 20};
  uint8_t brightness = 15;
  int fadeLowerBoundary = 1000;
  int fadeUpperBoundary = 6000;
  uint8_t spread = 10;
  bool fade = false;
} STATE;

// declarations

void saveConfigCallback();
void connect();
bool loadConfiguration();
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttConnect(bool);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
const char* getTopic(const char *topic);
void setBoolean(char* &payload, bool &state);
void setSingleBrightnessArray(char *brightnessTopic);
void publishState();
void log(const char* message);

#endif // MAIN_H
