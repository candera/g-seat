void setup() {
  Serial.begin(9600);

  pinMode(22, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(24, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);

  RoReg* reg = portInputRegister(digitalPinToPort(25));
  long mask = digitalPinToBitMask(25);

  Serial.println(*reg);
  Serial.println(mask);
  
}

void loop() {

}