#include <Arduino.h>

const int in1 = 17;
const int in2 = 18;
const int pwmPin = 16;
const int stby = 19;

const int freq = 5000;
const int channel = 0;
const int res = 8;

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(stby, OUTPUT);
  
  digitalWrite(stby, HIGH);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  ledcSetup(channel, freq, res);
  ledcAttachPin(pwmPin, channel);
}

void loop() {
  for(int d = 0; d <= 255; d++) {
    ledcWrite(channel, d);
    delay(15);
  }
  for(int d = 255; d >= 0; d--) {
    ledcWrite(channel, d);
    delay(15);
  }
}