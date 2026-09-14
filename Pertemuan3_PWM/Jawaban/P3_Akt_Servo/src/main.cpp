#include <Arduino.h>

const int servoPin = 16;
const int freq = 50;
const int channel = 0;
const int res = 12;

void setup() {
  ledcSetup(channel, freq, res);
  ledcAttachPin(servoPin, channel);
}

void loop() {
  for(int d = 102; d <= 512; d++) {
    ledcWrite(channel, d);
    delay(15);
  }
  for(int d = 512; d >= 102; d--) {
    ledcWrite(channel, d);
    delay(15);
  }
}