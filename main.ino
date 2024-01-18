#include "SoftPWM.h"

const int SOUND_PIN = 0;
const int BUTTON_PIN = 1;
const int POT_PIN = 19;

const int SAMPLE_TIME = 10;
unsigned long millisCurrent = 0;
unsigned long millisLast = 0;
unsigned long millisElapsed = 0;
int sampleBufferValue = 0;
int lastSampleBufferValue = 0;
int soundThreshold = 0;
int ledsToTurnOn = 0;

const int numLeds = 15;   
const int ledPins[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2}; 

const int numTransitionList = 8;
const int transitionList[numTransitionList][2] = {
    {12, 12},
    {13, 11},
    {14, 10},
    {15, 9},
    {16, 8},
    {2, 7},
    {3, 6},
    {4, 5}
};


int currentLed = 0;
int turnLedInterval = 50;

unsigned long listMillisLast[numLeds];
unsigned long randomInterval[numLeds];
int fadeDirection[numLeds];

int pot_value = 0;

int mode = 0;
int number_of_modes = 3;


void setup() {
  Serial.begin(9600);

  SoftPWMBegin();

  for (int i = 0; i < numLeds; i++) {
    randomInterval[i] = random(100, 1000);
    listMillisLast[i] = millis();
    fadeDirection[i] = 1;
    SoftPWMSet(ledPins[i], 0);
    SoftPWMSetFadeTime(ledPins[i], randomInterval[i], randomInterval[i]);
  }

  pinMode(POT_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);


}

void turn_off_leds() {
  for (int i = 0; i < numLeds; i++) {
    SoftPWMSetFadeTime(ledPins[i], 0, 0);
    SoftPWMSet(ledPins[i], 0);
  }
}

void change_mode_animation() {
  for (int i = 0; i < numTransitionList; i++) {
    SoftPWMSetFadeTime(transitionList[i][0], 50, 200);
    SoftPWMSetFadeTime(transitionList[i][1], 50, 200);
    SoftPWMSetPercent(transitionList[i][0], 100);
    SoftPWMSetPercent(transitionList[i][1], 100);
    delay(50);
    SoftPWMSetPercent(transitionList[i][0], 0);
    SoftPWMSetPercent(transitionList[i][1], 0);
  }
  delay(200);
}

void turn_leds() {
  millisCurrent = millis();
  millisElapsed = millisCurrent - millisLast;

  if (millisElapsed >= (turnLedInterval + pot_value)) {
    millisLast = millisCurrent;
    SoftPWMSetPercent(ledPins[currentLed], 0);
    currentLed = (currentLed + 1) % numLeds;
    SoftPWMSetPercent(ledPins[currentLed], 100);
  }
}

void update_sound() {
  millisCurrent = millis();
  millisElapsed = millisCurrent - millisLast;

  if (digitalRead(SOUND_PIN) == LOW) {
    sampleBufferValue++;
  }

  if (millisElapsed > SAMPLE_TIME) {
    if (sampleBufferValue >= lastSampleBufferValue) {
      lastSampleBufferValue = sampleBufferValue;
    }
    else {
      lastSampleBufferValue /= 1.04;
      sampleBufferValue = lastSampleBufferValue;
    }

    ledsToTurnOn = numLeds * sampleBufferValue / pot_value;
    if (ledsToTurnOn > numLeds) {
      ledsToTurnOn = numLeds;
    }

    for (int i = 0; i < ledsToTurnOn; i++) {
      SoftPWMSetPercent(ledPins[i], 100);
    }

    for (int i = ledsToTurnOn; i < numLeds; i++) {
      SoftPWMSetPercent(ledPins[i], 0);
    }
    sampleBufferValue = 0;
    millisLast = millisCurrent;
  }
}

void fade_random() {
  for (int i = 0; i < numLeds; i++) {
    millisCurrent = millis();
    
    if (millisCurrent - listMillisLast[i] >= randomInterval[i]) {
      listMillisLast[i] = millisCurrent;
      
      randomInterval[i] = random(100, 1000 + (pot_value * 2));
      SoftPWMSetFadeTime(ledPins[i], randomInterval[i], randomInterval[i]);
      if (fadeDirection[i] == 1) {
        SoftPWMSetPercent(ledPins[i], 0);
      }
      else {
        SoftPWMSetPercent(ledPins[i], 100);
      }
      fadeDirection[i] *= -1;
    }
  }
}

void loop() {
  pot_value = analogRead(POT_PIN);
  if (digitalRead(BUTTON_PIN) == LOW) {

    turn_off_leds();
    delay(200);
    change_mode_animation();
    mode = (mode + 1) % number_of_modes;

    if (mode == 1) {
      for (int i = 0; i < numLeds; i++) {
        SoftPWMSetFadeTime(ledPins[i], 0, 0);
      }
    }
    else if (mode == 2) {
      for (int i = 0; i < numLeds; i++) {
        SoftPWMSetFadeTime(ledPins[i], 50, 350);
      }
    }
  }
  if (mode == 0) {
    fade_random();
  }
  else if (mode == 1) {
    update_sound();
  }
  else if (mode == 2) {
    turn_leds();
  }
}
