#include <Arduino.h>
#include "FastLED.h"
#include <Fader.h>

// How many leds in your strip?
#define NUM_LEDS 15
#define DATA_PIN 3
#define BRIGHTNESS 255

// lamp setup
#define LAMP_NUM 4


void setLed(byte realLedNum, byte brightness);
void lampCallback(byte id, uint8_t brightness);
void updateLamps();

// Define the array of leds
CRGB leds[NUM_LEDS];

Fader lamps[LAMP_NUM] = {
  Fader(1,lampCallback),
  Fader(2,lampCallback),
  Fader(3,lampCallback),
  Fader(4,lampCallback)
};

void lampCallback(byte id, uint8_t brightness){
	setLed(id, brightness);
}

void updateLamps() {
	for (byte i = 0; i < LAMP_NUM; i++) {
    Fader *lamp = &lamps[i];
    lamp->update();

    // Set new fade
    if (lamp->is_fading() == false) {
      int duration = random(1000, 5000); // between 1 - 3 seconds

      // Up
      if (lamp->get_value() == 0) {
        byte color = random(100, 255);
        lamp->fade(color, duration);
      }
      // Down
      else {
        lamp->fade(0, duration);
      }
    }
  }
}



void setup() {
	FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
	LEDS.setBrightness(BRIGHTNESS);
}

void loop() {
	updateLamps();
	FastLED.show();
}

void setLed(byte realLedNum, byte brightness){
	byte ledNum = 0;
	byte colorNum = 0;
	float tool =  (realLedNum-1) /3;
    ledNum = byte(tool+.5);
    colorNum = (realLedNum-1) % 3;
    leds[ledNum][colorNum] = brightness;
}
