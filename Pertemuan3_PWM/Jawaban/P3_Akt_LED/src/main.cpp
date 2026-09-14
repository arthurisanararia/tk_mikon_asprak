#include <Arduino.h>

const int led1 = 16;
const int led2 = 17;
const int led3 = 18;

const int freq = 5000;
const int res = 8;
const int ch1 = 0;
const int ch2 = 1;
const int ch3 = 2;

void setup() {
  ledcSetup(ch1, freq, res);
  ledcSetup(ch2, freq, res);
  ledcSetup(ch3, freq, res);
  
  ledcAttachPin(led1, ch1);
  ledcAttachPin(led2, ch2);
  ledcAttachPin(led3, ch3);
}

void loop() {
  for(int d = 0; d <= 255; d++) {
    ledcWrite(ch1, d);
    delay(10);
  }
  for(int d = 255; d >= 0; d--) {
    ledcWrite(ch1, d);
    delay(10);
  }
  
  for(int d = 0; d <= 255; d++) {
    ledcWrite(ch2, d);
    delay(10);
  }
  for(int d = 255; d >= 0; d--) {
    ledcWrite(ch2, d);
    delay(10);
  }
  
  for(int d = 0; d <= 255; d++) {
    ledcWrite(ch3, d);
    delay(10);
  }
  for(int d = 255; d >= 0; d--) {
    ledcWrite(ch3, d);
    delay(10);
  }
}