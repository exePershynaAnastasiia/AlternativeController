#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <FastLED.h>


#define LED_PIN     13
#define NUM_LEDS    18
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

const byte BTN_PINS[NUM_LEDS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, A1, A2, A3, A4, A5};
const int PORT_NUM = 3;

const int BASIC_SLEEP_TIME = 6000;
const int SLEEP_TIME_RANGE = 4000;
const int AWAIT_TIME = 10000;
const int FLASH_COOLDOWN = 100;
const int MAX_AWAIT_TIME = 10000;
const long WIN_TIME = 90000;
const bool bCAN_LOSE = true;

CRGB leds[NUM_LEDS];

//CRGB leds[NUM_LEDS];
bool bIsOpen[NUM_LEDS];
bool bIsAwait[NUM_LEDS];
bool bCanClose[NUM_LEDS];
bool bIsPressed = false;
long coolDownLED[NUM_LEDS];
long coolDownFlash[NUM_LEDS];
long awaitTime[NUM_LEDS];

int lastPortRed = -1;
int lastPortBlue = -1;
long coolDownRed = 0;
long coolDownBlue = 0;
unsigned long startTime = 0;
unsigned long lastTime = 0;
unsigned long deltaTime = 0;

enum colorLED{
  RED,
  BLUE
};

enum gameState{
  WIN,
  LOSE,
  PLAYING
};

gameState currentGameState = PLAYING;

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  for(int i = 0; i <= NUM_LEDS - 1; i++){
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    coolDownLED[i] = -1;
    awaitTime[i] = MAX_AWAIT_TIME;

    bIsOpen[i] = false;
    bIsAwait[i] = false;
    bCanClose[i] = false;
  }
  
  //Serial.begin(9600);
  startTime = millis();

  fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255));
  FastLED.show();

  lastTime = millis();
  //srand(time(NULL));
  randomSeed(analogRead(A7));
  //openRandLED(RED);
  //openRandLED(BLUE);

  int sleepTime = BASIC_SLEEP_TIME + random(0, SLEEP_TIME_RANGE);
  coolDownRed = sleepTime;
  sleepTime = BASIC_SLEEP_TIME + random(0, SLEEP_TIME_RANGE);
  coolDownBlue = sleepTime;
}

void loop() {
  
  if(currentGameState == PLAYING){
    checkCoolDown();
    checkWinState();

    for (byte i = 0; i < NUM_LEDS; i++) {
      bool pressed = (digitalRead(BTN_PINS[i]) == LOW);
      if(pressed){
        awaitClose(i);
      }

      if(!pressed && bCanClose[i]){
        closeLED(i);
      }

      if(!pressed && bIsAwait[i] && !bCanClose[i] && bCAN_LOSE){
        lose();
      }
    }
  }

  FastLED.show();
  
}

void openRandLED(colorLED color){
  //int randomLED = rand() % NUM_LEDS;
  int randomPort = random(0, PORT_NUM);
  if(PORT_NUM > 1){
    if(color == RED){
      if(randomPort == lastPortRed){
        coolDownRed += 10;
        return;
      }
    }else if(color == BLUE){
      if(randomPort == lastPortBlue){
        coolDownBlue += 10;
        return;
      }
    }
  }

  int ledsEachPort = NUM_LEDS / PORT_NUM;
  int randomLED = random(0, ledsEachPort) + randomPort * ledsEachPort;
  if(isLedOn(randomLED) || bIsAwait[randomLED]){
    //Serial.print("LED Opened!!!!");
    coolDownRed += 10;
    coolDownBlue += 10;
    return;
  }
  //Serial.print("Port: ");
  //Serial.print(randomPort);
  //Serial.print(" LED: ");
  //Serial.print(randomLED);
  //Serial.print(" Not Opened!\n");

  //int sleepTime = BASIC_SLEEP_TIME + rand() % SLEEP_TIME_RANGE;
  int sleepTime = BASIC_SLEEP_TIME + random(0, SLEEP_TIME_RANGE);
  awaitTime[randomLED] = MAX_AWAIT_TIME;

  if(color == RED){
    leds[randomLED].setRGB(255, 0, 0);
    coolDownRed = sleepTime;
    lastPortRed = randomPort;
    //Serial.print("RED ");
  }else if(color == BLUE){
    leds[randomLED].setRGB(0, 0, 255);
    coolDownBlue = sleepTime;
    lastPortBlue = randomPort;
    //Serial.print("BLUE ");
  }

  bIsOpen[randomLED] = true;
  FastLED.show();

  //Serial.print("LED Open:");
  //Serial.print(randomLED);
  //Serial.print("\n");
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
  bIsAwait[index] = false;
  bCanClose[index] = false;
  endFlash(index);
  coolDownLED[index] = -1;
  leds[index].setRGB(255, 255, 255);
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
    if(bIsOpen[i] && !bIsAwait[i]){
      awaitTime[i] -= deltaTime;
      if(awaitTime[i] <= 0 && bCAN_LOSE){
        lose();
      }
    }

    if(coolDownLED[i] > 0){
      coolDownLED[i] -= deltaTime;
    }

    
    if(coolDownLED[i] <= 0 && bIsAwait[i] && !bCanClose[i]){
      bCanClose[i] = true;
      startFlash(i);
    }

    if(bIsAwait[i] && bCanClose[i]){
      coolDownFlash[i] -= deltaTime;
      if(coolDownFlash[i] <= 0){
        coolDownFlash[i] = FLASH_COOLDOWN;
        if(leds[i].getAverageLight() >= BRIGHTNESS / 2){
          leds[i].setRGB(0, 255, 0);
        }else{
          leds[i].setRGB(255, 255, 255);
        }
      }
    }

  }

  if(coolDownRed <= 0){
    openRandLED(RED);
  }

  if(coolDownBlue <= 0){
    openRandLED(BLUE);
  }
}

void startFlash(int index){
  coolDownFlash[index] = FLASH_COOLDOWN;
  leds[index].setRGB(255, 255, 255);
}

void endFlash(int index){
  coolDownFlash[index] = 0;
  if(bIsAwait[index]){
    leds[index].setRGB(0, 255, 0);
  }else{
    leds[index].setRGB(255, 255, 255);
  }
}

bool checkWinState(){
  unsigned long activeTime = millis() - startTime;
  if(activeTime >= WIN_TIME){
    win();
  }
}

void win(){
  currentGameState = WIN;
  for(int i = 0; i <= NUM_LEDS - 1; i++){
    leds[i].setRGB(0, 255, 0);
  }
  FastLED.show();
  //Serial.print("WIN!");
  return;
}

void lose(){
  currentGameState = LOSE;
  for(int i = 0; i <= NUM_LEDS - 1; i++){
    leds[i].setRGB(255, 0, 0);
  }
  FastLED.show();
  //Serial.print("LOSE!");
  return;
}
