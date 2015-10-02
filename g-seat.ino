#include <Arduino.h>

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
  const int POS_LOW = 750;
  const int POS_HIGH = 1020;
  const int HISTORY = 9;
  String _channelName;
  int _pinEn;
  int _pinDis;
  int _pinRPWM;
  int _pinLPWM;
  int _pinPos;
  float _target;
  int _seeking;
  int* _vals;
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
    _target = 0.25;
    _seeking = seeking(_target);
    _channelName = channelName;
    _counter = 0;
    _vals = (int*) malloc(HISTORY * sizeof(int));
    for (int i = 0; i < HISTORY; ++i) {
      _vals[i] = _seeking;
    }
    _direction = direction;
  }

  int seeking(float target) {
    if (_direction == Right) {
      return map(target * 1000000, 0, 1000000, POS_HIGH, POS_LOW);
    }
    else {
      return map(target * 1000000, 0, 1000000, POS_LOW, POS_HIGH);
    }      
  }

  void setup() {
    pinMode(_pinEn, OUTPUT);
    pinMode(_pinDis, OUTPUT);
    pinMode(_pinRPWM, OUTPUT);
    pinMode(_pinLPWM, OUTPUT);

    digitalWrite(_pinDis, LOW);
  }

  String channelName() {
    return _channelName;
  }
  
  void seek(float target) {
    // Inside this object we use potentiometer coordinates. Outside,
    // we use the range [0.0, 1.0]
    float t = constrain(target, 0.0, 1.0);
    _target = t;
    _seeking = seeking(t);
  }

  float target() {
    return _target;
  }

  float pos() {
    return (_vals[0] + _vals[1] + _vals[2]) / 3;
  }

  void update() {
    ++_counter;

    // TODO: Do we still need to bother with this smoothing?
    int aval = analogRead(_pinPos);
    _vals[_counter % HISTORY] = aval;

    int val = 0;
    for (int i = 0; i < HISTORY; ++i) {
      val += _vals[i];
    }
    val /= HISTORY;

    int diff = abs(_seeking - val);

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

    debug(_channelName);
    debug(" > V");
    debug(val);
    debug(" (");
    debug(aval);
    debug(" : ");
    debug(_vals[0]);
    debug(", ");
    debug(_vals[1]);
    debug(", ");
    debug(_vals[2]);
    debug(") => S");
    debug(_seeking);
    debug(" = D");
    debug(diff);
    debug(" : Dr");
    debug(drive);

    if (drive == 0) {
      digitalWrite(_pinEn, HIGH);
      digitalWrite(_pinRPWM, HIGH);
      digitalWrite(_pinLPWM, HIGH);
    }
    else if (((val < _seeking) && (_direction == Left)) ||
             ((val > _seeking) && (_direction == Right))) {
      digitalWrite(_pinEn, HIGH);
      //digitalWrite(_pinRPWM, LOW);
      analogWrite(_pinRPWM, (250 - drive));
      digitalWrite(_pinLPWM, HIGH);
      debug("L");
    }
    else {
      digitalWrite(_pinEn, HIGH);
      digitalWrite(_pinRPWM, HIGH);
      analogWrite(_pinLPWM, (250 - drive));
      //digitalWrite(_pinLPWM, LOW);
      debug("R");
    }

    debug();
  }

};

DriveUnit* driveUnits[] = {
  //new DriveUnit("BL", 2, 4, 3, 11, 0, Left),
  new DriveUnit("BR", 2, 4, 3, 11, 0, Right),
  //new DriveUnit("BR", 7, 8, 9, 10, 1)
  /* TODO: Others, although we only have seven PWM outputs, and we
     need 8 to drive four motors. One possibility is to use digital
     outputs and do a software PWM. Something along the lines of high
     if (currentTimeMills % 1000) > duty-cycle. Need to experiment.
  */
};

const int driveUnitCount = sizeof(driveUnits)/sizeof(DriveUnit*);

void dispatchCommand(String command) {
  Serial.print("Parsing ");
  Serial.println(command);

  if (command.startsWith("M ")) {
    String args = command.substring(2);
    if (_logLevel >= Debug) {
      Serial.println("Command is move");
      Serial.print("args: ");
      Serial.println(args);
    }
    for (int i = 0; i < driveUnitCount; ++i) {
      DriveUnit* unit = driveUnits[i];
      String channel = unit->channelName();
      String val = args.substring(channel.length());
      if (_logLevel >= Debug) {
        Serial.print("Checking ");
        Serial.println(channel);
        Serial.print("val: '");
        Serial.print(val);
        Serial.println("'");
      }
      if (args.startsWith(channel) && val.startsWith(" ")) {
        float target = val.substring(1).toFloat();
        if (_logLevel >= Debug) {
          Serial.print("New target: ");
          Serial.println(target);
        }
        unit->seek(target);
        break;
      }
    }
  }
  else if (command.equals("Q")) {
    for (int i = 0; i < driveUnitCount; ++i) {
      DriveUnit* unit = driveUnits[i];
      Serial.print(unit->channelName());
      Serial.print(" at ");
      Serial.print(unit->pos());
      Serial.print(" seeking ");
      Serial.print(unit->seeking(unit->target()));
      Serial.print(" (");
      Serial.print(unit->target());
      Serial.print(") ");
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
  static int vals[] = { 950, 950, 950 };

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

}