#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <FastLED.h>


//#define BUTTON_PIN 7

#define LED_PIN     3
#define NUM_LEDS    8
#define BRIGHTNESS  120
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

const byte BTN_PINS[NUM_LEDS] = {7, 6};

const int BASIC_SLEEP_TIME = 3000;
const int SLEEP_TIME_RANGE = 3000;
const int AWAIT_TIME = 1000;

CRGB leds[NUM_LEDS];
bool bIsOpen[NUM_LEDS];
bool bIsAwait[NUM_LEDS];
bool bCanClose[NUM_LEDS];
bool bIsPressed = false;
long coolDownLED[NUM_LEDS];

long coolDownRed = 0;
long coolDownBlue = 0;
unsigned long lastTime = 0;
unsigned long deltaTime = 0;

enum colorLED{
  RED,
  BLUE
};

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  for(int i = 0; i <= NUM_LEDS - 1; i++){
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    coolDownLED[i] = -1;

    bIsOpen[i] = false;
    bIsAwait[i] = false;
    bCanClose[i] = false;
  }
  
  Serial.begin(9600);

  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();

  lastTime = millis();
  srand(time(NULL));
  openRandLED(RED);
  openRandLED(BLUE);
}

void loop() {
  checkCoolDown();
  /*
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    closeAllLED();
  }*/

  for (byte i = 0; i < NUM_LEDS; i++) {
    bool pressed = (digitalRead(BTN_PINS[i]) == LOW);
    if(pressed){
      awaitClose(i);
    }

    if(!pressed && bCanClose[i]){
      closeLED(i);
    }
  }
    
  FastLED.show();
}

void openRandLED(colorLED color){
  int randomLED = rand() % 8;
  if(isLedOn(randomLED) || coolDownLED[randomLED] > 0){
    Serial.print("LED Opened!!!!");
    coolDownRed += 10;
    coolDownBlue += 10;
    return;
  }

  int sleepTime = BASIC_SLEEP_TIME + rand() % SLEEP_TIME_RANGE;

  if(color == RED){
    leds[randomLED].setRGB(255, 0, 0);
    coolDownRed = sleepTime;
  }else if(color == BLUE){
    leds[randomLED].setRGB(0, 0, 255);
    coolDownBlue = sleepTime;
  }

  bIsOpen[randomLED] = true;
  FastLED.show();

  Serial.print("LED Open:");
  Serial.print(randomLED);
  Serial.print("\n");
}

void awaitClose(int index){
  if(isLedOn(index)){
    coolDownLED[index] = AWAIT_TIME;
    bIsOpen[index] = false;
    bIsAwait[index] = true;
    leds[index].setRGB(0, 255, 0);
  }
}

void closeLED(int index){
  leds[index].setRGB(0, 0, 0);
  coolDownLED[index] = -1;
  bIsAwait[index] = false;
  bCanClose[index] = false;
}

void closeAllLED(){
  for(int i = 0; i <= NUM_LEDS - 1; i++){
    closeLED(i);
  }
}

bool isLedOn(int index) {
  return coolDownLED[index] <= 0 && bIsOpen[index];
}

void checkCoolDown(){
  unsigned long now = millis();
  deltaTime = now - lastTime;
  lastTime = now;

  coolDownRed -= deltaTime;
  coolDownBlue -= deltaTime;

  for(int i = 0; i <= NUM_LEDS - 1; i++){
    if(coolDownLED[i] > 0){
      coolDownLED[i] -= deltaTime;
    }

    if(coolDownLED[i] <= 0 && bIsAwait[i]){
      bCanClose[i] = true;
    }
  }

  if(coolDownRed <= 0){
    openRandLED(RED);
  }

  if(coolDownBlue <= 0){
    openRandLED(BLUE);
  }
}
