int pinEn = 2;
int pinDis = 4;
int pinRPWM = 3;
int pinLPWM = 11; 

void setup() {
  pinMode(pinEn, OUTPUT);
  pinMode(pinDis, OUTPUT);
  pinMode(pinRPWM, OUTPUT);
  pinMode(pinLPWM, OUTPUT);

}

void loop() {
  digitalWrite(pinDis, LOW);
  digitalWrite(pinEn, HIGH);
  digitalWrite(pinLPWM, HIGH);
  if ((micros() % 255) < 250) {
    digitalWrite(pinRPWM, LOW);
  } else {
    digitalWrite(pinRPWM, HIGH);
  }

}