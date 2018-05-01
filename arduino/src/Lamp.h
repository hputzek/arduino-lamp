/**
 * 
 *  Note: Cable from arduino to dimmer (starts with red)
 *  
 * 1: GND
 * 2: VCC
 * 3: SYNC
 * 4: Lamp 1
 * 5: Lamp 2
 * 6: Lamp 3
 * 7: Lamp 4
 *
 * Brown/White cable from arduino to esp
 * brown: GND
 * white: VCC
 * 
 * TX/RX Cable from arduino to esp
 * black: RX
 * normal/white: TX  
 * 
 **/

#define DIMMER_1_PIN 3
#define DIMMER_2_PIN 4
#define DIMMER_3_PIN 5
#define DIMMER_4_PIN 6
// SYNC PIN: 2
#define AC_FREQUENCY 50
#define RAMP_TIME 0
#define NUMBER_OF_LAMPS 4

uint8_t masterBrightness = 5;
bool isFading = false;
bool singleModeEnabled = false;
uint8_t singleBrightess[NUMBER_OF_LAMPS] = {0, 0, 0, 0};

int fadeLowerBoundary = 3000;
int fadeUpperBoundary = 5000;
uint8_t spread = 5;

Dimmer dimmers[NUMBER_OF_LAMPS] = {
    Dimmer(DIMMER_1_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY),
    Dimmer(DIMMER_2_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY),
    Dimmer(DIMMER_3_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY),
    Dimmer(DIMMER_4_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY)};

void setBrightnessCallback(uint8_t id, uint8_t brightness);
Fader faders[NUMBER_OF_LAMPS] = {
    Fader(0, setBrightnessCallback),
    Fader(1, setBrightnessCallback),
    Fader(2, setBrightnessCallback),
    Fader(3, setBrightnessCallback)};

int brightnessArray[NUMBER_OF_LAMPS];
void initializeDimmers();
void updateState();
void getRandomizedBrightness(int *brightnessArray, uint8_t lampCount, int lowerBoundary, int upperBoundary, int spread, int brightness);
void bubbleUnsort(int *list, int length);
void getSerialData();
void stopFaders();