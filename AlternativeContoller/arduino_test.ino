#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h> 
#include <FastLED.h>


//#define BUTTON_PIN 7

#define LED_PIN     3
#define NUM_LEDS    8
#define BRIGHTNESS  120
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

const byte BTN_PINS[NUM_LEDS] = {7, 6};

const int BASIC_SLEEP_TIME = 2000;
const int SLEEP_TIME_RANGE = 2000;

CRGB leds[NUM_LEDS];
bool bIsOpen[NUM_LEDS];
bool bIsPressed = false;

long coolDown = 0;
unsigned long lastTime = 0;
unsigned long deltaTime = 0;


void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  for(int i = 0; i <= NUM_LEDS; i++){
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }
  
  Serial.begin(9600);

  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();

  lastTime = millis();
  srand(time(NULL));
  openRandLED();
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
      closeLED(i);
    }
  }
    
  FastLED.show();
}

void openRandLED(){
  int randomLED = rand() % 8;
  if(isLedOn(randomLED)){
    Serial.print("LED Opened!!!!");
    coolDown = 10;
    return;
  }

  bIsOpen[randomLED] = true;
  leds[randomLED].setRGB(255, 0, 0);
  FastLED.show();

  Serial.print("LED Open:");
  Serial.print(randomLED);
  Serial.print("\n");

  int sleepTime = BASIC_SLEEP_TIME + rand() % SLEEP_TIME_RANGE;
  coolDown = sleepTime;
}

void closeLED(int index){
  leds[index].setRGB(0, 0, 0);
  bIsOpen[index] = false;
}

void closeAllLED(){
  for(int i = 0; i <= 7; i++){
    closeLED(i);
  }
}

bool isLedOn(int index) {
  return bIsOpen[index];
}

void checkCoolDown(){
  unsigned long now = millis();
  deltaTime = now - lastTime;
  lastTime = now;

  coolDown -= deltaTime;

  if(coolDown <= 0){
    openRandLED();
  }
}