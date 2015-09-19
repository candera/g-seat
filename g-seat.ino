#include <Arduino.h>

const int A_EN = 2;
const int A_RPWM = 3;
const int A_DIS = 4;
const int A_LPWM = 11;

int target = 950;

void setup() {
  // put your setup code here, to run once:
  pinMode(A_EN, OUTPUT);
  pinMode(A_DIS, OUTPUT);
  pinMode(A_RPWM, OUTPUT);
  pinMode(A_LPWM, OUTPUT);

  Serial.begin(9600);

  digitalWrite(A_DIS, LOW);

  // Set the prescaler for timer 2 (pins 11 and 3) to give us close to
  // 1KHz
  TCCR0B = (TCCR0B & 0b11111000) | 0x03;
}

void parseCommand(String command) {
  Serial.print("Parsing ");
  Serial.println(command);
    
  if (command.startsWith("GOTO ")) {
    target = command.substring(5).toInt();
  }
}

void loop() {
  static String pending = "";
  static unsigned long counter = 0;
  static int vals[] = { 950, 950, 950 };

  ++counter;
  
  while (Serial.available() > 0) {
    int inChar = Serial.read();

    if (inChar == '\n') {
      parseCommand(pending);
      pending = "";
    }
    else {
      pending += (char) inChar;
    }
  }
  
  vals[counter % 3] = analogRead(0);

  int val = (vals[0] + vals[1] + vals[2]) / 3;

  Serial.print(val);
  Serial.print(" => ");
  Serial.print(target);
  Serial.print(" = ");

  int diff = abs(target - val);

  Serial.print(diff);
  Serial.print(" : ");
  
  int drive = 0;

  int fullSpeedDiff = 100;
  int fullSpeedDrive = 250;
  int zeroSpeedDiff = 10;
  int maxApproachSpeed = 200;
  int minApproachSpeed = 50;
  
  if (diff > fullSpeedDiff) {
    drive = fullSpeedDrive;
  }
  else if (diff > zeroSpeedDiff) {
    drive = map(diff, zeroSpeedDiff, fullSpeedDiff, minApproachSpeed, maxApproachSpeed);
  }

  Serial.print(drive);
    
  if (drive == 0) {
    digitalWrite(A_EN, HIGH);
    digitalWrite(A_RPWM, HIGH);
    digitalWrite(A_LPWM, HIGH);
  }
  else if (val < target) {
    Serial.print("R");
    digitalWrite(A_EN, HIGH);
    analogWrite(A_RPWM, (250 - drive));
    digitalWrite(A_LPWM, HIGH);
  }
  else {
    Serial.print("L");
    digitalWrite(A_EN, HIGH);
    digitalWrite(A_RPWM, HIGH);
    analogWrite(A_LPWM, (250 - drive));
  }
  Serial.println();
  delay(1);
}