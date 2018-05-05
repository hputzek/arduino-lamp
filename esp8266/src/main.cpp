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

void onMqttConnect(bool sessionPresent)
{
  Serial.println(getTopic(topics.mqtt_single_mode_topic));
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
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  if (strcmp(getTopic(topics.mqtt_single_mode_topic), topic) == 0)
  {
    Serial.println("single Mode is:");
    state.singleMode = (bool)payload;
    Serial.println(state.singleMode);
  };
  if (strcmp(getTopic(topics.mqtt_state_topic), topic) == 0)
  {
    state.state = (bool)payload;
  };
  if (strcmp(getTopic(topics.mqtt_brightness_topic), topic) == 0)
  {
    if (state.singleMode)
    {
      char **brightness = NULL;
      int count = split(payload, ',', &brightness);
      if (count < NUMBER_OF_LAMPS)
      {
        for (uint8_t i = 0; i < count; i++)
          state.brightnessSingle[i] = atoi(brightness[i]);
      }
    }
    else
    {
      state.brightness = constrain(atoi(payload), 0, 100);
    }
  };
  if (strcmp(getTopic(topics.mqtt_fade_topic), topic) == 0)
  {
    state.fade = (bool)payload;
  };
  if (strcmp(getTopic(topics.mqtt_fade_lower_boundary_topic), topic) == 0)
  {
    state.fadeLowerBoundary = constrain(atoi(payload), 100, 60000);
  };
  if (strcmp(getTopic(topics.mqtt_fade_upper_boundary_topic), topic) == 0)
  {
    state.fadeUpperBoundary = constrain(atoi(payload), state.fadeLowerBoundary, 60000);
  };
  if (strcmp(getTopic(topics.mqtt_spread_topic), topic) == 0)
  {
    state.spread = constrain(atoi(payload), 0, 100);
  };

  publishState();
};

void publishState()
{
  const size_t bufferSize = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject &root = jsonBuffer.createObject();
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

  root["fadeLowerBoundary"] = state.fadeLowerBoundary;
  root["fadeUpperBoundary"] = state.fadeUpperBoundary;
  root["spread"] = state.spread;
  root["fade"] = state.fade;
  String output;
  root.printTo(output);
  mqttClient.publish("debug", 0, false, output.c_str());
  root.printTo(Serial);
}
bool loadConfiguration()
{
  yield();
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
    Serial.println("failed to connect and hit timeout");
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

  Serial.println(mqtt_server);
  Serial.println(atoi(mqtt_port));

  mqttClient.setServer(mqtt_server, atoi(mqtt_port));
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onSubscribe(onMqttSubscribe);
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

int split(char *str, char c, char ***arr)
{
  int count = 1;
  int token_len = 1;
  int i = 0;
  char *p;
  char *t;

  p = str;
  while (*p != '\0')
  {
    if (*p == c)
      count++;
    p++;
  }

  *arr = (char **)malloc(sizeof(char *) * count);
  if (*arr == NULL)
    exit(1);

  p = str;
  while (*p != '\0')
  {
    if (*p == c)
    {
      (*arr)[i] = (char *)malloc(sizeof(char) * token_len);
      if ((*arr)[i] == NULL)
        exit(1);

      token_len = 0;
      i++;
    }
    p++;
    token_len++;
  }
  (*arr)[i] = (char *)malloc(sizeof(char) * token_len);
  if ((*arr)[i] == NULL)
    exit(1);

  i = 0;
  p = str;
  t = ((*arr)[i]);
  while (*p != '\0')
  {
    if (*p != c && *p != '\0')
    {
      *t = *p;
      t++;
    }
    else
    {
      *t = '\0';
      i++;
      t = ((*arr)[i]);
    }
    p++;
  }

  return count;
}