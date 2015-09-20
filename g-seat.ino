#include <Arduino.h>
#include "types.h"

class DriveUnit {
 private:
  const int POS_LOW = 750;
  const int POS_HIGH = 1020;
  String _channelName;
  int _pinEn;
  int _pinDis;
  int _pinRPWM;
  int _pinLPWM;
  int _pinPos;
  float _target;
  int _vals[3];
  int _counter;

 public:
  DriveUnit(String channelName, int pinEn, int pinDis,
            int pinRPWM, int pinLPWM, int pinPos) {
    _pinEn = pinEn;
    _pinDis = pinDis;
    _pinRPWM = pinRPWM;
    _pinLPWM = pinLPWM;
    _pinPos = pinPos;
    _target = map(30, 0, 100, POS_LOW, POS_HIGH);
  }

  void setup() {
    pinMode(_pinEn, OUTPUT);
    pinMode(_pinDis, OUTPUT);
    pinMode(_pinRPWM, OUTPUT);
    pinMode(_pinLPWM, OUTPUT);

    digitalWrite(_pinDis, LOW);
  }

  void seek(float target) {
    // Inside this object we use potentiometer coordinates. Outside,
    // we use the range [0.0, 1.0]
    _target = map(target * 100000, 0, 100000, POS_LOW, POS_HIGH);
  }

  void update() {
    // TODO: Do we still need to bother with this smoothing?
    _vals[_counter % 3] = analogRead(_pinPos);

    int val = (_vals[0] + _vals[1] + _vals[2]) / 3;

    Serial.print(_channelName);
    Serial.print(" > ");
    Serial.print(val);
    Serial.print(" => ");
    Serial.print(_target);
    Serial.print(" = ");

    int diff = abs(_target - val);

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
      digitalWrite(_pinEn, HIGH);
      digitalWrite(_pinRPWM, HIGH);
      digitalWrite(_pinLPWM, HIGH);
    }
    else if (val < _target) {
      Serial.print("R");
      digitalWrite(_pinEn, HIGH);
      analogWrite(_pinRPWM, (250 - drive));
      digitalWrite(_pinLPWM, HIGH);
    }
    else {
      Serial.print("L");
      digitalWrite(_pinEn, HIGH);
      digitalWrite(_pinRPWM, HIGH);
      analogWrite(_pinLPWM, (250 - drive));
    }
    Serial.println();
  }

};

DriveUnit* driveUnits[] = {
  new DriveUnit("BL", 2, 4, 3, 11, 0)
  /* TODO: Others, although we only have seven PWM outputs, and we
     need 8 to drive four motors. One possibility is to use digital
     outputs and do a software PWM. Something along the lines of high
     if (currentTimeMills % 1000) > duty-cycle. Need to experiment.
  */
};

const int driveUnitCount = sizeof(driveUnits)/sizeof(DriveUnit*);

Command parseCommand(String input) {
  Serial.print("Parsing ");
  Serial.println(input);

  Command command;
  if (input.startsWith("BR ")) {
    command.type = MoveTo;
    command.data.moveTo.channel = BackRight;
    command.data.moveTo.target = input.substring(3).toFloat();
  }
  else if (input.startsWith("BL")) {
    command.type = MoveTo;
    command.data.moveTo.channel = BackLeft;
    command.data.moveTo.target = input.substring(3).toFloat();
  }
  else if (input.startsWith("SR ")) {
    command.type = MoveTo;
    command.data.moveTo.channel = SeatRight;
    command.data.moveTo.target = input.substring(3).toFloat();
  }
  else if (input.startsWith("SL")) {
    command.type = MoveTo;
    command.data.moveTo.channel = SeatLeft;
    command.data.moveTo.target = input.substring(3).toFloat();
  }
  else {
    command.type = None;
    Serial.print("Invalid command - ignoring: ");
    Serial.println(input);
  }

  return command;
}

void dispatchCommand(Command command) {
  switch (command.type) {
  case MoveTo:
    // Safeguard until we implement all of them
    if (command.data.moveTo.channel < driveUnitCount) {
      driveUnits[command.data.moveTo.channel]->seek(command.data.moveTo.target);
    }
    break;
  }
}

void setup() {
  // put your setup code here, to run once:

  for (int i = 0; i < driveUnitCount; ++i) {
    driveUnits[i]->setup();
  }

  Serial.begin(9600);

  // Set the prescaler for timer 2 (pins 11 and 3) to give us close to
  // 1KHz
  TCCR0B = (TCCR0B & 0b11111000) | 0x03;
}


void loop() {
  static String pending = "";
  static unsigned long counter = 0;
  static int vals[] = { 950, 950, 950 };

  ++counter;

  while (Serial.available() > 0) {
    int inChar = Serial.read();

    if (inChar == '\n') {
      Command command = parseCommand(pending);
      dispatchCommand(command);
      pending = "";
    }
    else {
      pending += (char) inChar;
    }
  }

  for (int i = 0; i < driveUnitCount; ++i) {
    driveUnits[i]->update();
  }

}