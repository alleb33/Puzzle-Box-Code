#include <Servo.h>
#include <Adafruit_NeoPixel.h>

int button1 = 12;
int button2 = 11;
int button3 = 10;
int button4 = 9;
int button5 = 8;
int button6 = 7;
int button7 = 6;
int button8 = 5;
const int BUTTON_PINS[] = {12, 11, 10, 9, 8, 7, 6, 5};

int simonSequence[6];
int maxSimonLevels = sizeof(simonSequence) / sizeof(simonSequence[0]);
int curSimonLevel = 1;

int neoPin = 2;
int neoAmt = 8;

Adafruit_NeoPixel strip(neoAmt, neoPin);

unsigned long startTime;
unsigned long maxWaitTime = 2000;

int reactSequence[6];
int maxReactLevels = sizeof(reactSequence) / sizeof(reactSequence[0]);
int currentReactLevel = 0;

int resetButtonPin = 13;
Servo boxServo;

int lockedPosition = 0;
int unlockedPosition = 90;

bool servoUnlocked = false;
bool resetTriggered = false;

Servo myservo;
int servoPin = 3;
int pos = 0;





void setup() {
  myservo.attach(servoPin);
  
  Serial.begin(9600);
  strip.begin();
  strip.show();
  strip.setBrightness(50);

  for (int i = 0; i < 8; i++) {
    pinMode(BUTTON_PINS[i], INPUT);
  }

  pinMode(resetButtonPin, INPUT);

  // randomize every reset
  randomSeed(analogRead(A0));

  lockServo();
  strip.clear();
  strip.show();
}




void loop() {
  //buttonPosTest();

  if (digitalRead(resetButtonPin) == HIGH) {
    resetGame();
    return;
  }

  if (!servoUnlocked) {
    if (curSimonLevel <= maxSimonLevels) {
      if (!playSimonLevel()) return;
    } else if (currentReactLevel < maxReactLevels) {
      if (!playReactLevel()) return;
    } else {
      unlockServo();
    }
  }
}




void lockServo() {
  for (pos = 0; pos <= 70; pos++) {
    myservo.write(pos);
    delay(15);
  }
  servoUnlocked = false;
  Serial.println("servo locked");
}



void unlockServo() {
  servoUnlocked = true;

  for (pos = 70; pos >= 0; pos--) {
    myservo.write(pos);
    delay(15);
  }
  // turn all LEDs green
  for (int i = 0; i < neoAmt; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();

  Serial.println("servo unlocked");
}



void resetGame() {
  Serial.println("game reset, starting over");
  lockServo();

  resetTriggered = true;

  strip.clear();
  strip.show();

  servoUnlocked = false;
  curSimonLevel = 1;
  currentReactLevel = 0;

  // generate new sequences randomly
  randomSequence(simonSequence, maxSimonLevels);
  randomSequence(reactSequence, maxReactLevels);

  // flash red for reset
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < neoAmt; j++) {
      strip.setPixelColor(j, strip.Color(255, 0, 0));
    }
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }

  resetTriggered = false;
}




void randomSequence(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    arr[i] = random(0, 8); // random numbers between 0 and 7
  }
}




bool playSimonLevel() {
  Serial.print("playing simon level: ");
  Serial.println(curSimonLevel);

  for (int i = 0; i < curSimonLevel; i++) {
    if (digitalRead(resetButtonPin) == HIGH) {
      resetGame();
      return false; // leave this level
    }

    int pixel = simonSequence[i];
    strip.setPixelColor(pixel, strip.Color(0, 0, 255));
    strip.show();
    delay(500);
    strip.setPixelColor(pixel, 0);
    strip.show();
    delay(100);
  }

  for (int i = 0; i < curSimonLevel; i++) {
    int correctButton = simonSequence[i];
    bool buttonPressed = false;

    while (!buttonPressed) {
      if (digitalRead(resetButtonPin) == HIGH) {
        resetGame();
        return false; // leave this level
      }

      for (int j = 0; j < 8; j++) {
        if (digitalRead(BUTTON_PINS[j]) == HIGH) {
          buttonPressed = true;
          if (j != correctButton) {
            Serial.println("wrong button");
            delay(1000);
            return false;
          } else {
            Serial.println("correct!");
            strip.setPixelColor(correctButton, strip.Color(0, 255, 0));
            strip.show();
            delay(500);
            strip.setPixelColor(correctButton, 0);
            strip.show();
            delay(100);
          }
        }
      }
    }
  }

  curSimonLevel++;
  return true;
}




bool playReactLevel() {
  Serial.print("playing react level: ");
  Serial.println(currentReactLevel);

  int correctButton = reactSequence[currentReactLevel];
  strip.setPixelColor(correctButton, strip.Color(255, 0, 0));
  strip.show();

  startTime = millis();
  bool buttonPressed = false;

  while (millis() - startTime < maxWaitTime) {
    if (digitalRead(resetButtonPin) == HIGH) {
      resetGame();
      return false; // leave this level
    }

    for (int i = 0; i < 8; i++) {
      if (digitalRead(BUTTON_PINS[i]) == HIGH) {
        buttonPressed = true;
        if (i == correctButton) {
          Serial.println("correct!");
          strip.setPixelColor(correctButton, strip.Color(0, 255, 0));
          strip.show();
          delay(500);
          strip.setPixelColor(correctButton, 0);
          strip.show();
          currentReactLevel++;
          return true;
        } else {
          Serial.println("wrong button");
          strip.clear();
          strip.show();
          currentReactLevel = 0;
          delay(100);
          return false;
        }
      }
    }
  }

  if (!buttonPressed) {
    Serial.println("no reaction in time");
    strip.clear();
    strip.show();
    currentReactLevel = 0;
    delay(100);
    return false;
  }

  return true;
}




void buttonPosTest() {
  if (digitalRead(BUTTON_PINS[0]) == HIGH) {
    lockServo();
    Serial.println("locking servo");
    delay(100);
  }

  if (digitalRead(BUTTON_PINS[7]) == HIGH) {
    unlockServo();
    Serial.println("unlocking servo");
    delay(100);
  }
}