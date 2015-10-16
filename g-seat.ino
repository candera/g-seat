#include <Wire.h>
#include <Arduino.h>
#include "Adafruit_PWMServoDriver.h"
#include <stdio.h>
#include "TimerThree.h"
#include "RotaryEncoder.h"
#include "DriveUnit.h"

#define DEBUG false

RotaryEncoder* encoders[] = {
  new RotaryEncoder(A0, A1)
};

const int encoderCount = sizeof(encoders)/sizeof(RotaryEncoder*);

Adafruit_PWMServoDriver pwm(0x40);

DriveUnit* driveUnits[] = {
  //            Ch,    Dir, En, RPWM, LPWM, pwm, encoder
  new DriveUnit("BL", Left,  5,    0,    1, &pwm, encoders[0])
};

const int driveUnitCount = sizeof(driveUnits)/sizeof(DriveUnit*);

void dispatchCommand(String command) {
  debug("Parsing ");
  debug(command);
  debug();

  if (command.startsWith("M ")) {
    String args = command.substring(2);
    debug("Command is move");
    debug("args: ");
    debug(args);
    debug();

    for (int i = 0; i < driveUnitCount; ++i) {
      DriveUnit* unit = driveUnits[i];
      String channel = unit->getChannelName();
      String val = args.substring(channel.length());

      if (args.startsWith(channel) && val.startsWith(" ")) {
        int target = val.substring(1).toInt();
        debug("New target: ");
        debug(target);
        debug();
        unit->seek(target);
        break;
      }
    }
  }
  else if (command.startsWith("DR ")) {
    Serial.println("DR");
    String args = command.substring(3);

    for (int i = 0; i < driveUnitCount; ++i) {
      DriveUnit* unit = driveUnits[i];
      String channel = unit->getChannelName();
      String val = args.substring(channel.length());

      Serial.println(channel);
      Serial.println(val);
      
      if (args.startsWith(channel) && val.startsWith(" ")) {
        int target = val.substring(1).toInt();
        Serial.print("Driving ");
        Serial.print(channel);
        Serial.print(" to ");
        Serial.println(target);
        unit->setDrive(target);
        break;
      }
    }
  }
  else if (command.equals("Q")) {
    for (int i = 0; i < driveUnitCount; ++i) {
      DriveUnit* unit = driveUnits[i];
      Serial.print(unit->getChannelName());
      Serial.print(" at ");
      Serial.print(unit->getPos());
      Serial.print(" seeking ");
      Serial.print(unit->getTarget());
      Serial.println();
    }
  }
  else if (command.equals("LOG DEBUG")) {
    _logLevel = Debug;
  }
  else if (command.equals("LOG INFO")) {
    _logLevel = Info;
  }
  else if (command.equals("PING")) {
    Serial.println("ACK G-SEAT");
  }
  else {
    Serial.print("Invalid command - ignoring: ");
    Serial.println(command);
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
  //Timer3.attachInterrupt(timerIsr, 100);

  pwm.begin();
  pwm.setPWMFreq(1000);

  for (int i = 0; i < driveUnitCount; ++i) {
    driveUnits[i]->setup();
  }
}

void loop() {
  static String pending = "";
  static long lastDebug = millis();

  while (Serial.available() > 0) {
    int inChar = Serial.read();

    if (inChar == '\n') {
      dispatchCommand(pending);
      pending = "";
    }
    else {
      pending += (char) inChar;
    }
  }

  /* for (int i = 0; i < driveUnitCount; ++i) { */
  /*   driveUnits[i]->update(); */
  /* } */

}
