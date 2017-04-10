#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <hw_timer.h>
#include "Dimmer.h"
#include <Fader.h>

// declarations
void mqttCallback(char* topic, byte* payload, unsigned int length);
void setupMQTTClient();
void zcDetectISR();
void dimTimerISR();


// OTHER SETTINGS
const byte mqttDebug = 1;
const byte outPin1 = 16;
const byte outPin2 = 14;
const byte outPin3 = 12;
const byte outPin4 = 13;
bool fading_enabled = false;

byte targetBrightness = 100;

// lamp setup
#define LAMP_NUM 4

const char* ssid = "***REMOVED***";
const char* password = "***REMOVED***";

// MQTT Settings
//const char* mqtt_server = "m2m.eclipse.org";
const char* mqtt_server = "192.168.178.23";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_state_topic = "office/light1/state";
const char* mqtt_brightness_topic = "office/light1/brightness";
const char* mqtt_fade_topic = "office/light1/fade";
const char* pir_state_topic = "office/light1/motion";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void lampCallback(byte id, uint8_t brightness);
void getLampsBrightness(int *brightnessArray, byte lampCount, int loverBoundary, int upperBoundary, int spread, int targetBrightness);
void updateLamps();
void bubbleSort(float A[],int len);
void bubbleUnsort(int *list, int length);


Fader lamps[LAMP_NUM] = {
  Fader(1,lampCallback),
  Fader(2,lampCallback),
  Fader(3,lampCallback),
  Fader(4,lampCallback)
};

Dimmer dimmers[LAMP_NUM] = {
  Dimmer(outPin1, 50),
  Dimmer(outPin2, 50),
  Dimmer(outPin3, 50),
  Dimmer(outPin4, 50),
};

int brightnessArray[LAMP_NUM];

void lampCallback(byte id, uint8_t brightness){
      yield();
      Dimmer *dimmer= &dimmers[id-1];
      dimmer->set(brightness);
}

void getLampsBrightness(int *brightnessArray, byte lampCount, int lowerBoundary, int upperBoundary,int spread, int targetBrightness) {
    //spread /= 2;
    int loopCount = lampCount;

    if (lampCount % 2 == 1) {
      loopCount--;
    }

    for (int i = 0; i < loopCount; i += 2) {
      int randomValue = random(spread / 2, spread);
      int upperOverflow = (targetBrightness + randomValue) - upperBoundary;
      int lowerOverflow = (targetBrightness -  randomValue) + lowerBoundary;

      if(upperOverflow < 0) {
        upperOverflow = 0;
      }

      if (lowerOverflow > 0) {
        lowerOverflow = 0;
      }
      brightnessArray[i] = min(targetBrightness + randomValue, upperBoundary) + lowerOverflow;
      brightnessArray[i + 1] = max(targetBrightness - randomValue, lowerBoundary) + upperOverflow ;
    }

    bubbleUnsort(brightnessArray,lampCount);
}


void updateLamps() {
  Fader *lamp = &lamps[0];
  if(fading_enabled) {
    int duration = random(1000, 3000);
    if (lamp->is_fading() == false) {
      getLampsBrightness(brightnessArray, LAMP_NUM, 1, 255, 50, targetBrightness);
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
      lamp->set_value(targetBrightness);
      lamp->update();
    }
  }
}



void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
   String type;
   if (ArduinoOTA.getCommand() == U_FLASH)
     type = "sketch";
   else // U_SPIFFS
     type = "filesystem";

   // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
   Serial.println("Start updating " + type);
 });
 ArduinoOTA.onEnd([]() {
   Serial.println("\nEnd");
 });
 ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
 });
 ArduinoOTA.onError([](ota_error_t error) {
   Serial.printf("Error[%u]: ", error);
   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
   else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
   else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
   else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
   else if (error == OTA_END_ERROR) Serial.println("End Failed");
 });

  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  randomSeed(micros());
  for (byte i = 0; i < LAMP_NUM; i++) {
    Dimmer *dimmer= &dimmers[i];
    dimmer->begin(0,true);
    delay(20);
  }
}

void loop() {
  yield();
  ArduinoOTA.handle();
  yield();
  setupMQTTClient();
  mqttClient.loop();
  yield();
  updateLamps();
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
          yield();
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

    }
    else if (s_payload == "OFF") {

    }
  }
  else if (s_topic == mqtt_fade_topic) {
    if (mqttDebug) { Serial.println(""); }

    if (s_payload == "ON") {
      fading_enabled = true;
    }
    else if (s_payload == "OFF") {
      fading_enabled = false;
    }
  }
  else if (s_topic == mqtt_brightness_topic) {
    if (mqttDebug) { Serial.println(""); }

    if (s_payload.toInt() != 0) {
      targetBrightness = (byte)s_payload.toInt();
    }
  }
  else {
    if (mqttDebug) { Serial.println(" [unknown message]"); }
  }
}
