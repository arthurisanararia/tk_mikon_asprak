#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

const int ledPin = 16;
const int freq = 5000;
const int channel = 0;
const int res = 8;

void setup() {
  ledcSetup(channel, freq, res);
  ledcAttachPin(ledPin, channel);
  
  Wire.begin();
  bme.begin(0x76);
}

void loop() {
  float temp = bme.readTemperature();
  
  int pwmValue = map(temp, 25, 35, 0, 255);
  
  if(pwmValue < 0) pwmValue = 0;
  if(pwmValue > 255) pwmValue = 255;
  
  ledcWrite(channel, pwmValue);
  delay(100);
}