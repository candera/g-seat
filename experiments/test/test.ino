void setup() {
  Serial.begin(9600);
}


void loop() {
  Serial.print("0: ");
  Serial.print(analogRead(0));
  Serial.print(" 1: ");
  Serial.println(analogRead(1));
  delay(200);
}