#include "Arduino.h"
#include "Dimmer.h"
#include "Fader.h"
#include "ArduinoJson.h"
#include "Lamp.h"

void setBrightnessCallback(uint8_t id, uint8_t brightness)
{
  dimmers[id].set(brightness);
}

void setup()
{
  randomSeed(analogRead(0));
  Serial.begin(115200);
  delay(100);
  getRandomizedBrightness(brightnessArray, NUMBER_OF_LAMPS, 1, 100, spread, masterBrightness);
  initializeDimmers();
}

void loop()
{
  getSerialData();

  updateState();
}

void initializeDimmers()
{
  for (uint8_t i = 0; i < NUMBER_OF_LAMPS; i++)
  {
    dimmers[i].begin();
  }
}

void updateState()
{
  if (isFading == true && state == true && singleModeEnabled == false)
  {
    int duration = random(fadeLowerBoundary, fadeUpperBoundary);
    if (faders[0].is_fading() == false)
    {
      getRandomizedBrightness(brightnessArray, NUMBER_OF_LAMPS, 1, 100, spread, masterBrightness);
      for (byte i = 0; i < NUMBER_OF_LAMPS; i++)
      {
        faders[i].fade(brightnessArray[i], duration);
        faders[i].update();
      }
    }
    else
    {
      for (byte i = 0; i < NUMBER_OF_LAMPS; i++)
      {
        faders[i].update();
      }
    }
  }
  else
  {
    for (byte i = 0; i < NUMBER_OF_LAMPS; i++)
    {
      if (state == true)
      {
        faders[i].set_value(singleModeEnabled ? singleBrightess[i] : brightnessArray[i]);
      }
      else
      {
        faders[i].set_value(0);
      }
      faders[i].update();
    }
  }
}

void stopFaders()
{
  for (byte i = 0; i < NUMBER_OF_LAMPS; i++)
  {
    faders[i].stop_fade();
  }
}

void getRandomizedBrightness(int *brightnessArray, uint8_t numberOfLamps, int fadeLowerBoundary, int fadeUpperBoundary, int spread, int brightness)
{
  if (spread < 2)
  {
    return;
  }
  spread /= 2;
  int loopCount = NUMBER_OF_LAMPS;

  if (numberOfLamps % 2 == 1)
  {
    loopCount--;
  }

  for (int i = 0; i < loopCount; i += 2)
  {
    int randomValue = random(spread / 2, spread);
    int upperOverflow = (masterBrightness + randomValue) - fadeUpperBoundary;
    int lowerOverflow = (masterBrightness - randomValue) + fadeLowerBoundary;

    if (upperOverflow < 0)
    {
      upperOverflow = 0;
    }

    if (lowerOverflow > 0)
    {
      lowerOverflow = 0;
    }
    brightnessArray[i] = min(masterBrightness + randomValue, fadeUpperBoundary) + lowerOverflow;
    brightnessArray[i + 1] = max(masterBrightness - randomValue, fadeLowerBoundary) + upperOverflow;
  }

  bubbleUnsort(brightnessArray, numberOfLamps);
}

void bubbleUnsort(int *list, int length)
{
  for (int a = length - 1; a > 0; a--)
  {
    int r = random(a + 1);
    //int r = rand_range(a+1);
    if (r != a)
    {
      int temp = list[a];
      list[a] = list[r];
      list[r] = temp;
    }
  }
}

void getSerialData()
{
  if (Serial.available())
  {
    Serial.flush();
    const size_t bufferSize = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(7) + 256;
    DynamicJsonBuffer jsonBuffer(bufferSize);

    // example json:  "{\"singleMode\":true,\"state\":true,\"brightness\":[30,50,80,100],\"fade\":true,\"fadeLowerBoundary\":1,\"fadeUpperBoundary\":100,\"spread\":5}";

    JsonObject &root = jsonBuffer.parseObject(Serial);

    // empty buffer if parsing does not succeed
    while (Serial.available() > 0)
    {
      Serial.read();
    }

    // Test if parsing succeeds.
    if (!root.success())
    {
      return;
    }

    stopFaders();

    singleModeEnabled = root["singleMode"];

    if (singleModeEnabled)
    {
      JsonArray &brightness = root["brightness"];
      singleBrightess[0] = brightness[0];
      singleBrightess[1] = brightness[1];
      singleBrightess[2] = brightness[2];
      singleBrightess[3] = brightness[3];
    }
    else
    {
      masterBrightness = root["brightness"];
      getRandomizedBrightness(brightnessArray, NUMBER_OF_LAMPS, 1, 100, spread, masterBrightness);
      for (byte i = 0; i < NUMBER_OF_LAMPS; i++)
      {
        faders[i].fade(brightnessArray[i], 500);
        faders[i].update();
      }
    }

    isFading = root["fade"];
    state = root["state"];
    fadeLowerBoundary = root["fadeLowerBoundary"];
    fadeUpperBoundary = root["fadeUpperBoundary"];
    spread = root["spread"];
  }
}