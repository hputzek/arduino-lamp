#include "Arduino.h"
#include "Dimmer.h"
#include "Fader.h"
#include "ArduinoJson.h"
#include "Lamp.h"

void setBrightnessCallback(uint8_t id, uint8_t brightness){
      dimmers[id-1].set(brightness);
}

void setup()
{
  initializeDimmers();
}

void loop()
{
  updateFading();
}

void initializeDimmers()
{
  for (int i = 0; i < sizeof(dimmers) / sizeof(Dimmer); i++)
  {
    dimmers[i].begin();
  }
}

void updateFading()
{
  if (isFading)
  {
    int duration = random(fadeLowerBoundary, fadeUpperBoundary);
    if (faders[0].is_fading() == false)
    {
      getRandomizedBrightness(brightnessArray, NUMBER_OF_LAMPS, 1, 100, spread, masterBrightness);

      for (byte i = 0; i < NUMBER_OF_LAMPS; i++) {
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
      faders[i].set_value(masterBrightness);
      faders[i].update();
    }
  }
}

void getRandomizedBrightness(int *brightnessArray, uint8_t numberOfLamps, int fadeLowerBoundary, int fadeUpperBoundary, int spread, int brightness)
{
  //spread /= 2;
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