#include <Arduino.h>
#include <Fader.h>
#include <Dimmer.h>

#define DATA_PIN 3

// lamp setup
#define LAMP_NUM 4

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
  Dimmer(7, DIMMER_NORMAL, 1.5, 50),
  Dimmer(8, DIMMER_NORMAL, 1.5, 50),
  Dimmer(9, DIMMER_NORMAL, 1.5, 50),
  Dimmer(10, DIMMER_NORMAL, 1.5, 50)
};

int brightnessArray[LAMP_NUM];

void lampCallback(byte id, uint8_t brightness){
	dimmers[id - 1].set(brightness);
}

void getLampsBrightness(int *brightnessArray, byte lampCount, int lowerBoundary, int upperBoundary,int spread, int targetBrightness) {
    //spread /= 2;
    int loopCount = lampCount;

    if (lampCount % 2 == 1) {
      loopCount--;
    }

    for (int i = 0; i <= loopCount; i += 2) {
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
  int duration = random(3000, 7000);
  if (lamp->is_fading() == false) {
    getLampsBrightness(brightnessArray, LAMP_NUM, 0, 100,5, 5);
    Serial.println(brightnessArray[0]);
    Serial.println(brightnessArray[1]);
    Serial.println((brightnessArray[0] + brightnessArray[1]) /2 );
    Serial.println("---");

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
  for(int i = 0; i < sizeof(dimmers) / sizeof(Dimmer); i++) {
    dimmers[i].begin();
  }
}

void loop() {
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
