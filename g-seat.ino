#include <Arduino.h>
#include <stdio.h>

#define DEBUG false

enum LogLevel {
  Info = 0,
  Debug = 1
};

LogLevel _logLevel = Info;

enum Direction {
  Left,
  Right
};

void debug(int i) {
  if (_logLevel >= Debug) {
    Serial.print(i);
  }
}

void debug(float f) {
  if (_logLevel >= Debug) {
    Serial.print(f);
  }
}

void debug(String s) {
  if (_logLevel >= Debug) {
    Serial.print(s);
  }
}

void debug() {
  if (_logLevel >= Debug) {
    Serial.println();
  }
}

class DriveUnit {
 private:
  const int POS_LOW = 800;
  const int POS_HIGH = 1020;
  String _channelName;
  int _pinEn;
  int _pinDis;
  int _pinRPWM;
  int _pinLPWM;
  int _pinPos;
  int _target;
  int _priorTarget;
  long _targetSetAt;
  int _raw;
  int _pos;
  int _smoothedPos;
  int _drive;
  bool _seeking;
  long _counter;
  Direction _direction;

 public:
  DriveUnit(String channelName, int pinEn, int pinDis,
            int pinRPWM, int pinLPWM, int pinPos, Direction direction) {
    _pinEn = pinEn;
    _pinDis = pinDis;
    _pinRPWM = pinRPWM;
    _pinLPWM = pinLPWM;
    _pinPos = pinPos;
    _target = 25;
    _priorTarget = 25;
    _pos = 25;
    _smoothedPos = 25;
    _drive = 0;
    _channelName = channelName;
    _counter = 0;
    _direction = direction;
    _seeking = false;
  }

  void setup() {
    pinMode(_pinEn, OUTPUT);
    pinMode(_pinDis, OUTPUT);
    pinMode(_pinRPWM, OUTPUT);
    pinMode(_pinLPWM, OUTPUT);

    digitalWrite(_pinDis, LOW);
  }

  String getChannelName() {
    return _channelName;
  }

  int getTarget() {
    return _target;
  }

  int getRaw() {
    return _raw;
  }
  
  int getPos() {
    return _pos;
  }

  int getSmoothedPos() {
    return _smoothedPos;
  }

  int getDrive() {
    return _drive;
  }

  void seek(int target) {
    // Inside this object we use potentiometer coordinates. Outside,
    // we use the range [0.0 100]
    _priorTarget = _target;
    _target = constrain(target, 0, 100);
    _targetSetAt = millis();
    _seeking = true;
  }


  // Poor man's PWM. I may have to ditch this for something like an
  // LED controller. Drive is in the range -100 to 100. The higher it
  // is, the more time the signal spends low.
  void setDrive(int drive)
  {
    _drive = drive;
    static const int pwmFrequency = 500; // Hz
    static const int pwmPeriod = 1000000 / pwmFrequency; // uSec

    int transition = map(abs(drive), 0, 100, 0, pwmPeriod);
    
    // TODO: Do we want to braking all the time?
    if (drive == 0) {
      digitalWrite(_pinEn, HIGH);
      digitalWrite(_pinRPWM, HIGH);
      digitalWrite(_pinLPWM, HIGH);
    }
    else if (drive < 0) {
      digitalWrite(_pinEn, HIGH);
      digitalWrite(_pinLPWM, HIGH);

      if ((micros() % pwmPeriod) < transition) {
        digitalWrite(_pinRPWM, LOW);
      }
      else {
        digitalWrite(_pinRPWM, HIGH);
      }
    }
    else {
      digitalWrite(_pinEn, HIGH);
      digitalWrite(_pinRPWM, HIGH);

      if ((micros() % pwmPeriod) < transition) {
        digitalWrite(_pinLPWM, LOW);
      }
      else {
        digitalWrite(_pinLPWM, HIGH);
      }
    }

  }

  void update() {
    // Everything is normalized to the range either 0-100 or -100-100,
    // depending on which makes more sense.
    
    ++_counter;

    // The pot position normalized into the range 0 (all the way up)
    // to 100 (all the way down).
    _raw = analogRead(_pinPos);
    _pos = map(_raw, POS_HIGH, POS_LOW, 0, 100);

    int diff = _target - _pos;
    int smoothing; // Range 0-100: how much we smooth out the current
                   // value with previous values. 100 completely
                   // ignors the current value. 0 uses exactly the
                   // current value.

    if (abs(diff) < 4) {
      _seeking = false;
    }

    if (_seeking) {
      smoothing = 50;
    }
    else {
      smoothing = 90;
    }
    
    _smoothedPos = ((_smoothedPos * smoothing) + (_pos * (100 - smoothing))) / 100;

    // Smoothly transition to a new target
    long targetAge = millis() - _targetSetAt;
    static const int targetDelay = 1000; // ms
    int currentTarget = 0;
    if (targetAge < targetDelay) {
      currentTarget = map(targetAge, 0, targetDelay, _priorTarget, _target);
    }
    else {
      currentTarget = _target;
    }
    
    int smoothedDiff = currentTarget - _smoothedPos;

    int drive = 0;
    if (smoothedDiff < -12) {
      drive = -40;
    }
    else if (smoothedDiff < -2) {
      drive = map(smoothedDiff, -12, -2, -40, -25);
    }
    else if (smoothedDiff < 3) {
      drive = 0;
    }
    else if (smoothedDiff < 30) {
      drive = map (smoothedDiff, 2, 30, 15, 40);
    }
    else {
      drive = 40;
    }
    
    setDrive(drive);
  }

};

DriveUnit* driveUnits[] = {
  //            Ch,   En, Dis, RPWM, LPWM, pos, dir
  new DriveUnit("BL",  2,   4,    3,   11,   0, Left),
  new DriveUnit("BR", 10,   8,    9,    7,   1, Right)
  //new DriveUnit("BR", 7, 8, 9, 10, 1)
  /* TODO: Others, although we only have seven PWM outputs, and we
     need 8 to drive four motors. One possibility is to use digital
     outputs and do a software PWM. Something along the lines of high
     if (currentTimeMills % 1000) > duty-cycle. Need to experiment.
  */
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
      debug("Checking ");
      debug(channel);
      debug();
      debug("val: '");
      debug(val);
      debug("'");
      debug();

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

  for (int i = 0; i < driveUnitCount; ++i) {
    driveUnits[i]->update();
  }

  if ((millis() - lastDebug) > 500) {
    for (int i = 0; i < driveUnitCount; ++i) {
      DriveUnit* unit = driveUnits[i];
      char str[512];
      sprintf(str, "%s Target:%02d Raw:%04d Pos:%02d (Smooth:%02d) Drive:%02d",
              unit->getChannelName().c_str(),
              unit->getTarget(), unit->getRaw(), unit->getPos(),
              unit->getSmoothedPos(), unit->getDrive());
      debug(str);
      debug();
      lastDebug = millis();
    }
  }
  


}