#include <Arduino.h>
#include "FastLED.h"
#include <Fader.h>

// How many leds in your strip?
#define NUM_LEDS 15
#define DATA_PIN 3
#define BRIGHTNESS 8

// lamp setup
#define LAMP_NUM 15


void setLed(byte realLedNum, byte brightness);
void lampCallback(byte id, uint8_t brightness);
void getLampsBrightness(float *brightnessArray, byte lampCount, int loverBoundary, int upperBoundary, float spread, float targetBrightness);
void updateLamps();
void bubbleSort(float A[],int len);

// Define the array of leds
CRGB leds[NUM_LEDS];

Fader lamps[LAMP_NUM] = {
  Fader(1,lampCallback),
  Fader(2,lampCallback),
  Fader(3,lampCallback),
  Fader(4,lampCallback),
  Fader(5,lampCallback),
  Fader(6,lampCallback),
  Fader(7,lampCallback),
  Fader(8,lampCallback),
  Fader(9,lampCallback),
  Fader(10,lampCallback),
  Fader(11,lampCallback),
  Fader(12,lampCallback),
  Fader(13,lampCallback),
  Fader(14,lampCallback),
  Fader(15,lampCallback)
};

float brightnessArray[14];

void lampCallback(byte id, uint8_t brightness){
	setLed(id, brightness);
}

void getLampsBrightness(float *brightnessArray, byte lampCount, int lowerBoundary, int upperBoundary,float spread, float targetBrightness) {
//	0 1 2 3 4 5 6 7 8 9 10  # The universe.
//	|                   |   # Place fixed dividers at 0, 10.
//	|   |     |       | |   # Add 4 - 1 randomly chosen dividers in [1, 9]
//	  a    b      c    d    # Compute the 4 differences: 2 3 4 1

  spread = min(targetBrightness, spread);

  float minimum = max(targetBrightness - spread * lampCount, lowerBoundary);
	float maximum = min(targetBrightness + spread * lampCount, upperBoundary);

  if(targetBrightness + spread > upperBoundary) {
    minimum -= targetBrightness + spread - upperBoundary;
  }

  if(targetBrightness - spread < lowerBoundary) {
    maximum += spread - targetBrightness + lowerBoundary;
    //maximum = maximum / 2;
  }


	float dividers[lampCount + 1];
	dividers[0] = minimum;
	dividers[lampCount] = maximum;

	for (int i = 1; i < lampCount; i = i + 1) {
		dividers[i] = random(minimum,maximum);
	}

  bubbleSort(dividers, lampCount + 1);

	for (int i = 0; i < lampCount; i = i + 1) {
		brightnessArray[i] = max(dividers[i],dividers[i+1]) - min(dividers[i],dividers[i+1]) + targetBrightness - spread / lampCount * 2;
	}
  float mysum = 0;
	for(int i = 0; i < lampCount; i++)
	{
    mysum += brightnessArray[i];
	}
  if(int(mysum / lampCount) != int(targetBrightness)) {
    Serial.println("-----------------");
    Serial.print("Result: ");
    Serial.println(mysum / lampCount);
    Serial.print("Target value: ");
    Serial.println(targetBrightness);
  }
}

void updateLamps() {
  Fader *lamp = &lamps[0];
  int duration = random(1000, 2000);
  if (lamp->is_fading() == false) {
    getLampsBrightness(brightnessArray, LAMP_NUM, 0, 255,50, 200);
    Serial.println(brightnessArray[0]);
    for (byte i = 0; i < LAMP_NUM; i++) {
      Fader *lamp = &lamps[i];
      lamp->fade(int(brightnessArray[i]), duration);
      lamp->update();
    }
  } else {
    for (byte i = 0; i < LAMP_NUM; i++) {
      Fader *lamp = &lamps[i];
      lamp->update();
    }
  }
}



void setup() {
	Serial.begin(115200);
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
