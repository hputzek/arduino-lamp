#include <Arduino.h>
#include <FastLED.h>
#include <Fader.h>

// How many leds in your strip?
#define NUM_LEDS 15
#define DATA_PIN 3
#define BRIGHTNESS 200

// lamp setup
#define LAMP_NUM 15


void setLed(byte realLedNum, byte brightness);
void lampCallback(byte id, uint8_t brightness);
void getLampsBrightness(int *brightnessArray, byte lampCount, int loverBoundary, int upperBoundary, int spread, int targetBrightness);
void updateLamps();
void bubbleSort(float A[],int len);
void bubbleUnsort(int *list, int length);

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

int brightnessArray[14];

void lampCallback(byte id, uint8_t brightness){
	setLed(id, brightness);
}

void getLampsBrightness(int *brightnessArray, byte lampCount, int lowerBoundary, int upperBoundary,int spread, int targetBrightness) {
    //spread /= 2;
    int loopCount = lampCount;

    if (lampCount % 2 == 1) {
      loopCount--;
    }

    for (int i = 0; i <= loopCount; i += 2) {
      int randomValue = random8(spread / 2, spread);
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
  int duration = random(1000, 2000);
  if (lamp->is_fading() == false) {
    getLampsBrightness(brightnessArray, LAMP_NUM, 0, 255,120, 120);
    Serial.println(brightnessArray[0]);
    Serial.println(brightnessArray[1]);
    Serial.println((brightnessArray[0] + brightnessArray[1]) / 2 );
    for (byte i = 0; i < LAMP_NUM - 1; i++) {
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
