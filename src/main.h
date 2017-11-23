#ifndef MAIN_H
#define MAIN_H

// OTHER SETTINGS
const byte mqttDebug = 1;
const byte outPin1 = 14;
const byte outPin2 = 12;
const byte outPin3 = 13;
const byte outPin4 = 15;
bool fade = false;
int preset = 1;
byte brightness = 100;
byte spread = 50;
bool state = true;

// lamp setup
#define LAMP_NUM 4

// MQTT Settings
//const char* mqtt_server = "m2m.eclipse.org";
const char* mqtt_server = "192.168.178.23";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_preset_topic = "office/light1/preset";
const char* mqtt_state_topic = "office/light1/state";
const char* mqtt_brightness_topic = "office/light1/brightness";
const char* mqtt_fade_topic = "office/light1/fade";
const char* mqtt_spread_topic = "office/light1/spread";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

int brightnessArray[LAMP_NUM];

// declarations

void setupMQTTClient();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void lampCallback(byte id, uint8_t brightness);
void getLampsBrightness(int *brightnessArray, byte lampCount, int loverBoundary, int upperBoundary, int spread, int brightness);
void updateLamps();
void bubbleSort(float A[],int len);
void bubbleUnsort(int *list, int length);
bool saveSettings();
bool loadSettings();
void publishState();
byte getMappedBrightness();
const char* getCharFromByte(byte);

Fader lamps[LAMP_NUM] = {
  Fader(1,lampCallback),
  Fader(2,lampCallback),
  Fader(3,lampCallback),
  Fader(4,lampCallback)
};

Dimmer dimmers[LAMP_NUM] = {
  Dimmer(outPin1, 0),
  Dimmer(outPin2, 0),
  Dimmer(outPin3, 0),
  Dimmer(outPin4, 0),
};

#endif // MAIN_H
