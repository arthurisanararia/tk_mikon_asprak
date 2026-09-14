#include <Arduino.h>

const int potPin = 34;
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
  int potValue = analogRead(potPin);
  
  int pwmValue = map(potValue, 0, 4095, 0, 255);
  
  if (pwmValue < 0) pwmValue = 0;
  if (pwmValue > 255) pwmValue = 255;
  
  ledcWrite(channel, pwmValue);
  delay(15);
}