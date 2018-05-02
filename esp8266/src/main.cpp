#include "FS.h"          //this needs to be first, or it all crashes and burns...
#include "ESP8266WiFi.h" //https://github.com/esp8266/Arduino
#include "DNSServer.h"
#include "ESP8266WebServer.h"
#include "WiFiManager.h" //https://github.com/tzapu/WiFiManager
#include "AsyncMqttClient.h"
#include "ArduinoJson.h"
#include "Arduino.h"
#include "main.h"

char mqtt_server[40];
char mqtt_port[6] = "8080";
char mqtt_device_address[64] = "livingRoom";

// state
TOPICS topics;
STATE state;

//flag for saving data
bool shouldSaveConfig = false;

AsyncMqttClient mqttClient;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void onMqttConnect(bool sessionPresent)
{
  mqttClient.subscribe(getTopic(topics.mqtt_single_mode_topic), 1);
  mqttClient.subscribe(getTopic(topics.mqtt_state_topic), 1);
  mqttClient.subscribe(getTopic(topics.mqtt_brightness_topic), 1);
  mqttClient.subscribe(getTopic(topics.mqtt_fade_topic), 1);
  mqttClient.subscribe(getTopic(topics.mqtt_fade_upper_boundary_topic), 1);
  mqttClient.subscribe(getTopic(topics.mqtt_fade_lower_boundary_topic), 1);
  mqttClient.subscribe(getTopic(topics.mqtt_spread_topic), 1);
  // handle status
  mqttClient.publish(getTopic(topics.mqtt_status_topic), 2, true, "online");
  mqttClient.setWill(getTopic(topics.mqtt_status_topic), 2, true, "offline");
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  if (getTopic(topics.mqtt_single_mode_topic) == topic)
  {
    state.singleMode = (bool)payload;
  };
  if (getTopic(topics.mqtt_state_topic) == topic)
  {
    state.state = (bool)payload;
  };
  if (getTopic(topics.mqtt_brightness_topic) == topic)
  {
    if (state.singleMode)
    {
      char **brightness = NULL;
      int count = split(payload, ',', &brightness);

    for (uint8_t i = 0; i < count; i++)
        state.brightnessSingle[i] = atoi(brightness[i]);
    }
    else
    {
      state.brightness = constrain(atoi(payload), 0, 100);
    }
  };
  if (getTopic(topics.mqtt_fade_topic) == topic)
  {
    state.fade = (bool)payload;
  };
  if (getTopic(topics.mqtt_fade_lower_boundary_topic) == topic)
  {
    state.fadeLowerBoundary = constrain(atoi(payload), 100, 60000);
  };
  if (getTopic(topics.mqtt_fade_upper_boundary_topic) == topic)
  {
    state.fadeUpperBoundary = constrain(atoi(payload), state.fadeLowerBoundary, 60000);
  };
  if (getTopic(topics.mqtt_spread_topic) == topic)
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

  root.printTo(Serial);
}
void loadConfiguration()
{
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

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_device_address, json["mqtt_device_address"]);
        }
      }
    }
  }
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

  //set static ip
  wifiManager.setSTAStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_address);
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password"))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_device_address, custom_mqtt_address.getValue());

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
  // convert content of mqtt_server to a (hopefully) valid ip address
  IPAddress mqttServerIp;
  if (mqttServerIp.fromString(mqtt_server))
  {
    mqttClient.setServer(mqttServerIp, atoi(mqtt_port));
    mqttClient.connect();
  }

  mqttClient.onMessage(onMqttMessage);
  mqttClient.onConnect(onMqttConnect);
}

void setup()
{
  Serial.begin(115200);
  connect();
}

void loop()
{
}

char *getTopic(char *topic)
{
  return strcat(mqtt_device_address, (char *)topic);
}

int split (char *str, char c, char ***arr)
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

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
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