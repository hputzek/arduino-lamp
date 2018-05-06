#include "FS.h"          //this needs to be first, or it all crashes and burns...
#include "ESP8266WiFi.h" //https://github.com/esp8266/Arduino
#include "DNSServer.h"
#include "ESP8266WebServer.h"
#include "WiFiManager.h" //https://github.com/tzapu/WiFiManager
#include "AsyncMqttClient.h"
#include "ArduinoJson.h"
#include "Arduino.h"
#include "main.h"

#define NUMBER_OF_LAMPS 4
#define MINIMUM_FADING_TIME 100
#define MAXIMUM_FADING_TIME 30000
#define ENABLE_LOGGING false
const char *mqtt_server = "192.168.178.23";
const char *mqtt_port = "1883";
const char *mqtt_device_address = "livingRoom";

// state
TOPICS topics;
STATE state;

//flag for saving data
bool shouldSaveConfig = false;

AsyncMqttClient mqttClient;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  shouldSaveConfig = true;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  if (WiFi.isConnected())
  {
    mqttClient.connect();
  }
}

void onMqttConnect(bool sessionPresent)
{
  mqttClient.subscribe(getTopic(topics.mqtt_single_mode_topic), 2);
  mqttClient.subscribe(getTopic(topics.mqtt_state_topic), 2);
  mqttClient.subscribe(getTopic(topics.mqtt_brightness_topic), 2);
  mqttClient.subscribe(getTopic(topics.mqtt_fade_topic), 2);
  mqttClient.subscribe(getTopic(topics.mqtt_fade_upper_boundary_topic), 2);
  mqttClient.subscribe(getTopic(topics.mqtt_fade_lower_boundary_topic), 2);
  mqttClient.subscribe(getTopic(topics.mqtt_spread_topic), 2);
  // handle status
  mqttClient.publish(getTopic(topics.mqtt_status_topic), 2, true, "online");
  mqttClient.setWill(getTopic(topics.mqtt_status_topic), 2, true, "offline");
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  if (ENABLE_LOGGING == true)
  {
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
  }
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  if (strcmp(getTopic(topics.mqtt_single_mode_topic), topic) == 0)
  {
    setBoolean(payload, state.singleMode);
  };
  if (strcmp(getTopic(topics.mqtt_state_topic), topic) == 0)
  {
    setBoolean(payload, state.state);
  };
  if (strcmp(getTopic(topics.mqtt_brightness_topic), topic) == 0)
  {
    if (state.singleMode == true)
    {
     setSingleBrightnessArray(payload);
    }
    else
    {
      state.brightness = constrain(atoi(payload), 0, 100);
    }
  };
  if (strcmp(getTopic(topics.mqtt_fade_topic), topic) == 0)
  {
    setBoolean(payload, state.fade);
  };
  if (strcmp(getTopic(topics.mqtt_fade_lower_boundary_topic), topic) == 0)
  {
    state.fadeLowerBoundary = constrain(atoi(payload), MINIMUM_FADING_TIME, MAXIMUM_FADING_TIME);
  };
  if (strcmp(getTopic(topics.mqtt_fade_upper_boundary_topic), topic) == 0)
  {
    state.fadeUpperBoundary = constrain(atoi(payload), state.fadeLowerBoundary, MAXIMUM_FADING_TIME);
  };
  if (strcmp(getTopic(topics.mqtt_spread_topic), topic) == 0)
  {
    state.spread = constrain(atoi(payload), 0, 100) / 2;
  };

  publishState();
};

void setBoolean(char *&payload, bool &state)
{
  if (strcmp(payload, (char *)"ON") == 0 || strcmp(payload, (char *)"TRUE") == 0 || strcmp(payload, (char *)"true") == 0)
  {
    state = true;
  }
  else
  {
    state = false;
  }
}

void publishState()
{
  const size_t bufferSize = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(8);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject &root = jsonBuffer.createObject();
  root["state"] = state.state;
  root["singleMode"] = state.singleMode;
  if (state.singleMode)
  {
    JsonArray &brightness = root.createNestedArray("brightness");
    brightness.add(state.brightnessSingle[0]);
    brightness.add(state.brightnessSingle[1]);
    brightness.add(state.brightnessSingle[2]);
    brightness.add(state.brightnessSingle[3]);
  }
  else
  {
    root["brightness"] = state.brightness;
  }

  root["fadeLowerBoundary"] = (state.fadeLowerBoundary < state.fadeUpperBoundary) ? state.fadeLowerBoundary : state.fadeUpperBoundary;
  root["fadeUpperBoundary"] = state.fadeUpperBoundary;
  root["spread"] = state.spread;
  root["fade"] = state.fade;
  String output;
  root.printTo(output);
  mqttClient.publish(getTopic(topics.mqtt_log_topic), 0, false, output.c_str());
  root.printTo(Serial);
}
bool loadConfiguration()
{
  bool success = false;
  //read configuration from FS json
  if (SPIFFS.begin())
  {
    if (SPIFFS.exists("/config.json"))
    {
      //file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());

        if (json.success())
        {
          mqtt_server = json["mqtt_server"];
          mqtt_port = json["mqtt_port"];
          mqtt_device_address = json["mqtt_device_address"];
          success = true;
        }
      }
    }
  }
  return success;
}

void connect()
{
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_address("address", "mqtt device address", mqtt_device_address, 64);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setDebugOutput(ENABLE_LOGGING);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_address);
  if (!loadConfiguration())
  {
    wifiManager.startConfigPortal("LampSetup", "esp8266");
  }
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("LampSetup", "esp8266"))
  {
    log("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //save the custom parameters to FS
  if (shouldSaveConfig)
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_address"] = mqtt_device_address;

    File configFile = SPIFFS.open("/config.json", "w");
    json.printTo(configFile);
    configFile.close();
  }

  //read updated parameters
  mqtt_server = custom_mqtt_server.getValue();
  mqtt_port = custom_mqtt_port.getValue();
  mqtt_device_address = custom_mqtt_address.getValue();

  mqttClient.setServer(mqtt_server, atoi(mqtt_port));
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setMaxTopicLength(128);
  mqttClient.connect();
}

void setup()
{
  Serial.begin(115200);
  connect();
}

void loop()
{
}

const char *getTopic(const char *topic)
{
  String total = String(mqtt_device_address) + String(topic);
  return total.c_str();
}

void setSingleBrightnessArray(char *brightnessTopic)
{
  const size_t bufferSize = JSON_ARRAY_SIZE(4) + 20;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonArray &root = jsonBuffer.parseArray(brightnessTopic);

  JsonArray &root_ = root;
  state.brightnessSingle[0] = root_[0];
  state.brightnessSingle[1] = root_[1];
  state.brightnessSingle[2] = root_[2];
  state.brightnessSingle[3] = root_[3];
}

void log(const char *message)
{
  if (ENABLE_LOGGING == true)
  {
    Serial.println(message);
  }
}