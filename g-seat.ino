#include <Wire.h>
#include <Arduino.h>
#include "common.h"
#include "Adafruit_PWMServoDriver.h"
#include <stdio.h>
#include "TimerThree.h"
#include "RotaryEncoder.h"
#include "DriveUnit.h"

#define DEBUG TRUE

RotaryEncoder* encoders[] = {
  new RotaryEncoder(F, 7, F, 6)
};

const int encoderCount = sizeof(encoders)/sizeof(RotaryEncoder*);

Adafruit_PWMServoDriver pwm(0x40);

#define NORMAL 0
#define DEBUG 1

int _mode = NORMAL;

long _debug = 0;
long _diagnosticsUntil;

DriveUnit* driveUnits[] = {
  //            Ch,   Dir,              En, RPWM, LPWM, pwm, encoder
  new DriveUnit("BL", CounterClockwise,  5,    0,    1, &pwm, encoders[0])
};

const int driveUnitCount = sizeof(driveUnits)/sizeof(DriveUnit*);

Token nextToken(char* s) {
  // Serial.println(s);
  Token result;
  char* nextSpace = strchr(s, ' ');
  if (nextSpace == NULL) {
    strcpy(result.val, s);
    result.next = "";
  }
  else {
    int bytes = nextSpace - s;
    // Serial.println(bytes);
    strncpy(result.val, s, bytes);
    result.val[bytes] = '\0';
    for (char* p = nextSpace; *p == ' '; ++p) {
      result.next = p;
    }
    result.next += 1;
  }

  return result;           
}

Token nextToken(Token t) {
  return nextToken(t.next);
}

DriveUnit* lookupUnit(char* channelName) {
  for (int i = 0; i < driveUnitCount; ++i) {
    DriveUnit* unit = driveUnits[i];
    if (eq(channelName, unit->getChannelName())) {
      return unit;
    }
  }
  return NULL;
}

void runTest2(DriveUnit* u, int drive1, int drive2)
{
  bool up1 = drive1 < 0;
  bool up2 = drive2 < 0;
  if (up1) {
    u->setDrive(3750);
  }
  else {
    u->setDrive(-3750);
  }
  while ((up1 && (u->getPos() < 9000)) ||
         (!up1 && (u->getPos() > 1000))) {
    delay(1);
  }
  u->setDrive(0);
  for (int i = 0; i < 25; ++i) {
    // Gather motion stats so they're accurate at the beginning.
    u->update();
    delay(10);
  }

  long start = micros();
  long now = start;
  long pos;
  long lastPos = u->getPos();
  bool switched = false;
  char buffer[128];
  long drive = drive1;
  u->setDrive(drive);
  do {
    now = micros();
    pos = u->getPos();
    if ((((lastPos - 5000) * (pos - 5000)) <= 0) && !switched) {
      switched = true;
      drive = drive2;
      u->setDrive(drive);
    }
    u->printDiagnostics();
    lastPos = pos;
    u->update();
    delay(20);
  } while (((now - start) < 1000000) &&
           ((pos > 500) && (pos < 9500)));
            
  u->setDrive(0);

  Serial.println();
  Serial.println();
}

void runTest(DriveUnit* u, long drive)
{
  bool up = drive < 0;
  if (up) {
    u->setDrive(3750);
    while (u->readPos() < 9000) {
      delay(1);
    }
  }
  else {
    u->setDrive(-3750);
    while (u->readPos() > 1000) {
      delay(1);
    }
  }
  u->setDrive(0);
  for (int i = 0; i < 25; ++i) {
    // Gather motion stats so they're accurate at the beginning.
    u->update();
    delay(10);
  }

  long start = micros();
  long now = start;
  long pos;
  bool switched = false;
  char buffer[128];
  u->setDrive(drive);
  do {
    now = micros();
    u->update();
    pos = u->getPos();
    if (!switched) {
      if (up && (pos < 5000)) {
        drive = -5000;
        switched = true;
        u->setDrive(drive);
      }
      else if (!up && (pos > 5000)) {
        drive = 5000;
        switched = true;
        u->setDrive(drive);
      }
    }
    u->printDiagnostics();
    //delay(10);
  } while (((now - start) < 10000000) &&
           ((up && (pos > 500)) ||
            (!up && (pos < 9500))));
            
  u->setDrive(0);

  Serial.println();
  Serial.println();
}


void dispatchCommand(char* command) {
  Token t = nextToken(command);
  char* directive = t.val;

  if (eq(directive, "TEST")) {
    t = nextToken(t);
    DriveUnit* u = lookupUnit(t.val);
    if (u != NULL) {
      t = nextToken(t);
      long drive = atol(t.val);
      runTest(u, drive);
    }
  }
  else if (eq(directive, "M")) {
    t = nextToken(t);

    DriveUnit* unit = lookupUnit(t.val);
    if (unit != NULL) {
      long target = atol(nextToken(t).val);
      debug("New target: ");
      debug(target);
      debug();
      unit->seek(target);
      _diagnosticsUntil = micros() + 2000000;
    }
  }
  else if (eq(directive, "MONITOR")) {
    t = nextToken(t);
    long msec = atol(t.val);
    _diagnosticsUntil = micros() + (1000 * msec);    
  }
  else if (eq(directive, "CONFIG")) {
    t = nextToken(t);

    DriveUnit* unit = lookupUnit(t.val);
    if (unit != NULL) {
      t = nextToken(t);

      char slot[16];
      strcpy(slot, t.val);
      t = nextToken(t);
      double val = atof(t.val);

      boolean result = unit->setParam(slot, val);

      if (!result) {
        Serial.print("ERROR: Could not set param ");
      }
      else {
        Serial.print("Set param ");
      }
      Serial.print(slot);
      Serial.print(" to value ");
      Serial.print(val);
      Serial.print(" for channel ");
      Serial.println(unit->getChannelName());
    }
  }
  else if (eq(directive, "DR")) {
    Serial.println("DR");
    t = nextToken(t);
    DriveUnit* unit = lookupUnit(t.val);
    if (unit != NULL) {
      int target = atoi(nextToken(t).val);
      Serial.print("Driving ");
      Serial.print(unit->getChannelName());
      Serial.print(" to ");
      Serial.println(target);
      unit->setDrive(target);
    }
  }
  else if (eq(directive, "Q")) {
    printDiagnostics();
  }
  else if (eq(directive, "MODE")) {
    t = nextToken(t);
    if (eq(t.val, "NORMAL")) {
      _mode = NORMAL;
    }
    else if (eq(t.val, "DEBUG")) {
      _mode = DEBUG;
    }
    else {
      Serial.print("Unrecognized mode: ");
      Serial.println(t.val);
    }
  }
  else if (eq(directive, "?")) {
    //Serial.println(encoders[0]->getPos());
    Serial.print("Next: '");
    Serial.print(t.next);
    Serial.println("'");
    Serial.print("Val: '");
    Serial.print(t.val);
    Serial.println("'");
    t = nextToken(t);
    Serial.print("Next: '");
    Serial.print(t.next);
    Serial.println("'");
    Serial.print("Val: '");
    Serial.print(t.val);
    Serial.println("'");
    t = nextToken(t);
    Serial.print("Next: '");
    Serial.print(t.next);
    Serial.println("'");
    Serial.print("Val: '");
    Serial.print(t.val);
    Serial.println("'");
  }
  else if (eq(directive, "LOG DEBUG")) {
    _logLevel = Debug;
  }
  else if (eq(directive, "LOG INFO")) {
    _logLevel = Info;
  }
  else if (eq(directive, "PING")) {
    Serial.println("ACK G-SEAT");
  }
  else {
    Serial.print("Invalid command - ignoring: ");
    Serial.println(command);
  }
}

void printDiagnostics() {
  for (int i = 0; i < driveUnitCount; ++i) {
    DriveUnit* unit = driveUnits[i];
    unit->printDiagnostics();
  }
}

void timerIsr() {
  for (int i = 0; i < encoderCount; ++i) {
    encoders[i]->update();
  }
}

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < encoderCount; ++i) {
    encoders[i]->setup();
  }

  Timer3.initialize();
  Timer3.attachInterrupt(timerIsr, 80);

  pwm.begin();
  pwm.setPWMFreq(1600);

  /* double acc; */
  /* double df; */
  /* EEPROM_readAnything(0, acc); */
  /* EEPROM_readAnything(0, df); */
  
  for (int i = 0; i < driveUnitCount; ++i) {
    driveUnits[i]->setup();
    /* 
    if (acc != 0) {
      driveUnits[i]->setAcc(acc);
    }
    if (df != 0) {
      driveUnits[i]->setDriveFactor(df);
    }
    */
  }

  /* pinMode(A0, INPUT_PULLUP); */
  /* pinMode(A1, INPUT_PULLUP); */
}

void loop() {
  static char pending[256];
  static int pendingIndex = 0;
  static long lastDebug = millis();

  while (Serial.available() > 0) {
    int inChar = Serial.read();

    if ((inChar == '\n') || (pendingIndex == 255)) {
      pending[pendingIndex] = '\0';
      dispatchCommand(pending);
      pendingIndex = 0;
    }
    else {
      pending[pendingIndex++] = (char) inChar;
    }
  }

  if (_mode == NORMAL) {
    for (int i = 0; i < driveUnitCount; ++i) {
      DriveUnit* u = driveUnits[i];
      u->update();
      //u->setDrive(u->getDrive());
    }
  }

  static bool printNewline = false;
  if (micros() < _diagnosticsUntil) {
    printDiagnostics();
    printNewline = true;
  }
  else {
    if (printNewline) {
      Serial.println();
      Serial.println();
      printNewline = false;
    }
  }

  //delay(50);

  //encoders[0]->update();

  #if false
  if ((millis() - lastDebug) > 500) {
    lastDebug = millis();
    long pos = driveUnits[0]->getPos();
    long target = driveUnits[0]->getTarget();
    Serial.print("BL at ");
    Serial.print(pos);
    Serial.print(" seeking ");
    Serial.print(target);
    Serial.print(" diff ");
    Serial.print(pos - target);
    Serial.print(" driving at ");
    Serial.print(driveUnits[0]->getDrive());
    Serial.print(" encoder at ");
    Serial.println(encoders[0]->getPos());
    // Serial.println(encoders[0]->getPos());
  }
  #endif

}
