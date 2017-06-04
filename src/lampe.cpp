
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <hw_timer.h>
#include "Dimmer.h"
#include <Fader.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include "FS.h"
#include "settings.h"

// declarations
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void setupMQTTClient();


// OTHER SETTINGS
const byte mqttDebug = 1;
const byte outPin1 = 16;
const byte outPin2 = 14;
const byte outPin3 = 12;
const byte outPin4 = 13;
bool fade = false;
int preset = 1;
byte brightness = 100;
byte spread = 50;
bool state = true;

// lamp setup
#define LAMP_NUM 4

// const char* ssid = "my-ssid";
// const char* password = "password";

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

ESP8266WebServer server (80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WebSocketsServer webSocket = WebSocketsServer(80);

void lampCallback(byte id, uint8_t brightness);
void getLampsBrightness(int *brightnessArray, byte lampCount, int loverBoundary, int upperBoundary, int spread, int brightness);
void updateLamps();
void bubbleSort(float A[],int len);
void bubbleUnsort(int *list, int length);
bool saveSettings();
bool loadSettings();


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

int brightnessArray[LAMP_NUM];

bool loadSettings(){
  DynamicJsonBuffer jsonSettings;
  File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
      File configFile = SPIFFS.open("/config.json", "w");
      configFile.print("{}");
      Serial.println("Failed to open config file");
      return false;
    }

    size_t size = configFile.size();
    if (size > 1024) {
      Serial.println("Config file size is too large");
      return false;
    }
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);

    configFile.close();

    JsonObject& settings = jsonSettings.parseObject(buf.get());


    if (!settings.success()) {
      Serial.println("Failed to parse config file");
      return false;
    }
    if (preset == 0) {
      preset = settings["activePreset"];
    }
  brightness = settings["presets"][String(preset)]["brightness"];
  fade = settings["presets"][String(preset)]["fade"];
  state = settings["presets"][String(preset)]["state"];
  spread = settings["presets"][String(preset)]["spread"];
  return true;
}

bool saveSettings() {
  DynamicJsonBuffer jsonSettings;
  File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
      Serial.println("Failed to open config file");
      return false;
    }

    size_t size = configFile.size();
    if (size > 1024) {
      Serial.println("Config file size is too large");
      return false;
    }
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);

    configFile.close();

    JsonObject& settings = jsonSettings.parseObject(buf.get());
  settings["activePreset"] = preset;
  if(!settings["presets"]) {
     settings.createNestedObject("presets");
  }
  JsonObject& presets = settings["presets"];
  JsonObject& currentPreset =  presets.createNestedObject(String(preset));

  settings["presets"][String(preset)]["brightness"] = brightness;
  settings["presets"][String(preset)]["fade"] = fade;
  settings["presets"][String(preset)]["state"] = state;
  settings["presets"][String(preset)]["spread"] = spread;
  File configFileSave = SPIFFS.open("/config.json", "w");

  if (!configFileSave) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  settings.prettyPrintTo(Serial);
  settings.printTo(configFileSave);

  configFile.close();
  return true;
}

void lampCallback(byte id, uint8_t brightness){
      yield();
      Dimmer *dimmer= &dimmers[id-1];
      dimmer->set(brightness);
      yield();
}

void getLampsBrightness(int *brightnessArray, byte lampCount, int lowerBoundary, int upperBoundary,int spread, int brightness) {
    //spread /= 2;
    int loopCount = lampCount;

    if (lampCount % 2 == 1) {
      loopCount--;
    }

    for (int i = 0; i < loopCount; i += 2) {
      int randomValue = random(spread / 2, spread);
      int upperOverflow = (brightness + randomValue) - upperBoundary;
      int lowerOverflow = (brightness -  randomValue) + lowerBoundary;

      if(upperOverflow < 0) {
        upperOverflow = 0;
      }

      if (lowerOverflow > 0) {
        lowerOverflow = 0;
      }
      brightnessArray[i] = min(brightness + randomValue, upperBoundary) + lowerOverflow;
      brightnessArray[i + 1] = max(brightness - randomValue, lowerBoundary) + upperOverflow ;
    }

    bubbleUnsort(brightnessArray,lampCount);
}


void updateLamps() {
  Fader *lamp = &lamps[0];
  if(fade) {
    int duration = random(1000, 3000);
    if (lamp->is_fading() == false) {
      getLampsBrightness(brightnessArray, LAMP_NUM, 1, 255, spread, brightness);
      Serial.println(brightnessArray[0]);
      Serial.println(brightnessArray[1]);
      Serial.println((brightnessArray[0] + brightnessArray[1]) /2 );
      Serial.println("---");

      for (byte i = 0; i < LAMP_NUM; i++) {
        Fader *lamp = &lamps[i];
        lamp->fade(brightnessArray[i], duration);
        lamp->update();
      }
    } else {
      for (byte i = 0; i < LAMP_NUM; i++) {
        Fader *lamp = &lamps[i];
        lamp->update();
      }
    }
  } else {
    for (byte i = 0; i < LAMP_NUM; i++) {
      Fader *lamp = &lamps[i];
      lamp->set_value(brightness);
      lamp->update();
    }
  }
}



void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  Serial.println("Mounting FS...");

    if (!SPIFFS.begin()) {
      Serial.println("Failed to mount file system");
      return;
    }

    if (!loadSettings()) {
      Serial.println("Failed to load config");
    } else {
      Serial.println("Config loaded");
    }

    randomSeed(micros());
    for (byte i = 0; i < LAMP_NUM; i++) {
      Dimmer *dimmer= &dimmers[i];
      dimmer->begin(brightness,true);
      updateLamps();
      delay(500);
      yield();
    }

  yield();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    ESP.restart();
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/index.js", SPIFFS, "/index.js");
  server.serveStatic("/vendor.js", SPIFFS, "/vendor.js");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  yield();
  server.handleClient();
  webSocket.loop();
  yield();
  setupMQTTClient();
  mqttClient.loop();
  yield();
  updateLamps();
  yield();
}

void bubbleSort(float A[],int len) {
  unsigned long newn;
  unsigned long n=len;
  float temp=0.0;
  do {
    newn=1;
    for(int p=1;p<len;p++){
      if(A[p-1]>A[p]){
        temp=A[p];           //swap places in array
        A[p]=A[p-1];
        A[p-1]=temp;
        newn=p;
      } //end if
    } //end for
    n=newn;
  } while(n>1);
}

void bubbleUnsort(int *list, int length)
{
 for (int a=length-1; a>0; a--)
 {
   int r = random(a+1);
   //int r = rand_range(a+1);
   if (r != a)
   {
     int temp = list[a];
     list[a] = list[r];
     list[r] = temp;
   }
 }
}


void setupMQTTClient() {
   while (!mqttClient.connected()) {
      yield();
      int connectResult;
      // Create a random client ID
         String clientId = "ESP8266Client-";
         clientId += String(random(0xffff), HEX);
         // Attempt to connect
      if (mqtt_server != "") {
        Serial.print("MQTT client: ");
        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setCallback(mqttCallback);
        if (mqtt_user == "") {
          connectResult = mqttClient.connect(clientId.c_str());
        }
        else {
          connectResult = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password);
        }

        if (connectResult) {
          Serial.println("Connected");
        }
        else {
          Serial.print("Failed (");
          Serial.print(mqttClient.state());
          Serial.println(")");
        }

        if (mqttClient.connected()) {
          Serial.print("MQTT topic '");
          Serial.print(mqtt_state_topic);
          if (mqttClient.subscribe(mqtt_state_topic)) {
            Serial.println("': Subscribed");
          }
          else {
            Serial.print("': Failed");
          }

          Serial.print("MQTT topic '");
          Serial.print(mqtt_brightness_topic);
          if (mqttClient.subscribe(mqtt_brightness_topic)) {
            Serial.println("': Subscribed");
          }
          else {
            Serial.print("': Failed");
          }

          Serial.print("MQTT topic '");
          Serial.print(mqtt_fade_topic);
          if (mqttClient.subscribe(mqtt_fade_topic)) {
            Serial.println("': Subscribed");
          }
          else {
            Serial.print("': Failed");
          }

          Serial.print("MQTT topic '");
          Serial.print(mqtt_preset_topic);
          if (mqttClient.subscribe(mqtt_preset_topic)) {
            Serial.println("': Subscribed");
          }
          Serial.print("MQTT topic '");
          Serial.print(mqtt_spread_topic);
          if (mqttClient.subscribe(mqtt_spread_topic)) {
            Serial.println("': Subscribed");
          }
          else {
            Serial.print("': Failed");
          }
       }
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char c_payload[length];
  memcpy(c_payload, payload, length);
  c_payload[length] = '\0';

  String s_topic = String(topic);
  String s_payload = String(c_payload);

  if (mqttDebug) {
    Serial.print("MQTT in: ");
    Serial.print(s_topic);
    Serial.print(" = ");
    Serial.print(s_payload);
  }

  if (s_topic == mqtt_state_topic) {
    if (mqttDebug) { Serial.println(""); }

    if (s_payload == "ON") {
      state = true;
    }
    else if (s_payload == "OFF") {
      state = false;
    }
  }
  else if (s_topic == mqtt_fade_topic) {
    if (mqttDebug) { Serial.println(""); }

    if (s_payload == "ON") {
      fade = true;
    }
    else if (s_payload == "OFF") {
      fade = false;
    }
  }
  else if (s_topic == mqtt_brightness_topic) {
    if (mqttDebug) { Serial.println(""); }

    if (s_payload.toInt() != 0) {
      brightness = map((byte)s_payload.toInt(), 1, 100, 1, 255);
    }
  }
  else if (s_topic == mqtt_preset_topic) {
    if (mqttDebug) { Serial.println(""); }

    if (s_payload.toInt() != 0) {
      preset = constrain(s_payload.toInt(),1,5);
      hw_timer_stop();
      loadSettings();
      Dimmer *dimmer = &dimmers[0];
      dimmer->restart();
    }
  }
  else if (s_topic == mqtt_spread_topic) {
    if (mqttDebug) { Serial.println(""); }

    if (s_payload.toInt() != 0) {
      spread = constrain((byte)s_payload.toInt(), 0, 100);
    }
  }
  else {
    if (mqttDebug) { Serial.println(" [unknown message]"); }
  }
  hw_timer_stop();
  saveSettings();
 Dimmer *dimmer = &dimmers[0];
 dimmer->restart();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        				// send message to client
        				webSocket.sendTXT(num, "3");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}
