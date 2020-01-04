#include "FS.h"          //this needs to be first, or it all crashes and burns...
#include "ESP8266WiFi.h" //https://github.com/esp8266/Arduino
#include "DNSServer.h"
#include "ESPAsyncWiFiManager.h"       //https://github.com/tzapu/WiFiManager
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"
#include "ArduinoJson.h"
#include "Arduino.h"
#include "main.h"

#define ENABLE_LOGGING false

// state
TOPICS topics;
STATE state;

AsyncWebServer server(80);
AsyncWebSocket wSet("/set");
AsyncWebSocket wSave("/save");
DNSServer dns;

void onSet(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    //client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      //Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.println(msg.c_str());
      if(info->opcode == WS_TEXT)
        wSet.textAll(msg.c_str());
        wSave.cleanupClients();
        wSet.cleanupClients();
    } 
  }
}

void onSave(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      //Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.println(msg.c_str());
      if(info->opcode == WS_TEXT)
        client->text("{status: \"true\"}");
        wSet.cleanupClients();
        wSave.cleanupClients();
    } 
  }
}

void connect()
{

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.setDebugOutput(ENABLE_LOGGING);

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
}

void setup()
{
  Serial.begin(115200);
  connect();
  wSet.onEvent(onSet);
  wSave.onEvent(onSave);
  server.addHandler(&wSet);
  server.addHandler(&wSave);
  server.begin();
}

void loop()
{
}

void log(const char *message)
{
  if (ENABLE_LOGGING == true)
  {
    Serial.println(message);
  }
}