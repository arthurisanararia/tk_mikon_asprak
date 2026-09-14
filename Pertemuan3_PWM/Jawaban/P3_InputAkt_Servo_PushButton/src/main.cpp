#include <Arduino.h>

const int buttonPin = 21;
const int servoPin = 16;
const int freq = 50;
const int channel = 0;
const int res = 12;

int duty = 102;
int step = 2;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  ledcSetup(channel, freq, res);
  ledcAttachPin(servoPin, channel);
  ledcWrite(channel, duty);
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    duty += step;
    if (duty >= 512 || duty <= 102) {
      step = -step;
    }
    ledcWrite(channel, duty);
    delay(15);
  }
}