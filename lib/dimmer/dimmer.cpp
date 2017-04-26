/** @file
 * This is a library to control the intensity of dimmable AC lamps or other AC loads using triacs.
 *
 * Copyright (c) 2015 Circuitar
 * This software is released under the MIT license. See the attached LICENSE file for details.
 */
#include "Dimmer.h"
#include "Arduino.h"
#include <hw_timer.h>

// Dimmer registry
static Dimmer* dimmmers[DIMMER_MAX_TRIAC];     // Pointers to all registered dimmer objects
static uint8_t dimmerCount = 0;                // Number of registered dimmer objects

// Triac pin and timing variables. Using global arrays to make ISR fast.
static volatile uint8_t triacPinPorts[DIMMER_MAX_TRIAC]; // Triac ports for registered dimmers
static bool triacPinMasks[DIMMER_MAX_TRIAC];           // Triac pin mask for registered dimmers
static volatile uint8_t triacTimes[DIMMER_MAX_TRIAC];              // Triac time for registered dimmers

// Timer ticks since zero crossing
static uint8_t tmrCount = 0;

// Global state variables
bool Dimmer::started = false; // At least one dimmer has started

// Timer interrupt
void dimmerISR() {
  // Increment ticks
  if (tmrCount < 254) {
    tmrCount++;
  }

  // Process each registered triac and turn it on if needed
  for (uint8_t i = 0; i < dimmerCount; i++) {
    if (tmrCount == triacTimes[i]) {
      digitalWrite(triacPinPorts[i], HIGH);
    }
  }
}

// Zero cross interrupt
void callZeroCross() {
    tmrCount = 0;
  // Turn off all triacs and disable further triac activation before anything else
  for (uint8_t i = 0; i < dimmerCount; i++) {
    digitalWrite(triacPinPorts[i], LOW);
    triacTimes[i] = 255;
  }

  // Process each registered dimmer object
  for (uint8_t i = 0; i < dimmerCount; i++) {
    dimmmers[i]->zeroCross();

    // If triac time was already reached, activate it
    if (tmrCount >= triacTimes[i]) {
      digitalWrite(triacPinPorts[i], HIGH);
    }
  }
}



// Constructor
Dimmer::Dimmer(uint8_t pin,uint8_t freq) :
        triacPin(pin),
        lampState(false),
        lampValue(0),
        acFreq(freq) {
  if (dimmerCount < DIMMER_MAX_TRIAC) {
    // Register dimmer object being created
    dimmerIndex = dimmerCount;
    dimmmers[dimmerCount++] = this;
    triacPinPorts[dimmerIndex] = pin;
    triacPinMasks[dimmerIndex] = false;
  }
}

void Dimmer::begin(uint8_t value, bool on) {
  // Initialize lamp state and value
  set(value, on);

  // Initialize triac pin
  pinMode(triacPin, OUTPUT);
  digitalWrite(triacPin, LOW);

  if (!started) {
    // Start zero cross circuit if not started yet
    pinMode(DIMMER_ZERO_CROSS_PIN, INPUT);
    // attach interrupt and setup timer
    hw_timer_init(NMI_SOURCE, 1);
    hw_timer_set_func(dimmerISR);
    hw_timer_arm(39);
    attachInterrupt(DIMMER_ZERO_CROSS_PIN, callZeroCross, RISING);
    started = true;
  }
}

void Dimmer::restart() {
  hw_timer_init(NMI_SOURCE, 1);
  hw_timer_set_func(dimmerISR);
  hw_timer_arm(39);
}

void Dimmer::off() {
  lampState = false;
}

void Dimmer::on() {
  lampState = true;
}

void Dimmer::toggle() {
  lampState = !lampState;
}

bool Dimmer::getState() {
  return lampState;
}

uint8_t Dimmer::getValue() {
  return (int32_t) lampValue;
}

void Dimmer::set(uint8_t value) {
  if (value > 255) {
    value = 255;
  }

  if (value < minValue) {
    value = minValue;
  }

  if (value != lampValue) {
    lampValue = 255 - value;
  }
}

void Dimmer::set(uint8_t value, bool on) {
  set(value);
  lampState = on;
}

void Dimmer::setMinimum(uint8_t value) {
  if (value > 255) {
    value = 255;
  }

  minValue = value;

  if (lampValue < minValue) {
    set(value);
  }
}

void Dimmer::zeroCross() {
  // Calculate triac time for the current cycle
  uint8_t value = getValue();
  if (value > 0 && lampState) {
    triacTimes[dimmerIndex] = value;
  }
}
