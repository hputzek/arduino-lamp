#define DIMMER_1_PIN 3
#define DIMMER_2_PIN 4
#define DIMMER_3_PIN 5
#define DIMMER_4_PIN 6
#define AC_FREQUENCY 50
#define RAMP_TIME 0
#define NUMBER_OF_LAMPS 4       
#define TEST 5

uint8_t masterBrightness = 80;
bool isFading = false;
int fadeLowerBoundary = 3000;
int fadeUpperBoundary = 5000;
uint8_t spread = 20;

Dimmer dimmers[NUMBER_OF_LAMPS] = {
    Dimmer(DIMMER_1_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY),
    Dimmer(DIMMER_2_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY),
    Dimmer(DIMMER_3_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY),
    Dimmer(DIMMER_4_PIN, DIMMER_NORMAL, RAMP_TIME, AC_FREQUENCY)};

void setBrightnessCallback(uint8_t id, uint8_t brightness);
Fader faders[NUMBER_OF_LAMPS] = {
    Fader(1, setBrightnessCallback),
    Fader(2, setBrightnessCallback),
    Fader(3, setBrightnessCallback),
    Fader(4, setBrightnessCallback)};

int brightnessArray[4];
void initializeDimmers();
void updateFading();
void getRandomizedBrightness(int *brightnessArray, uint8_t lampCount, int lowerBoundary, int upperBoundary, int spread, int brightness);
void bubbleUnsort(int *list, int length);