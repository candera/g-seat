#include <Arduino.h>

void setup() {
  Serial.begin(9600);

  pinMode(32, INPUT);

  RoReg* reg = portInputRegister(digitalPinToPort(32));
  long mask = digitalPinToBitMask(32);

  long loops = 1000000;
  long start = millis();
  long total = 0;
  for (int i = 0; i < loops; ++i) {
    //total += digitalRead(32);
    total += ((*reg) & mask) ? 1 : 0;
  }
  
  long end = millis();
  Serial.println(loops/(double)(end-start));
  Serial.println(total);
  
}

void loop() {

}