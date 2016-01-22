#include "common.h"
#include "Adafruit_PWMServoDriver.h"
#include <Arduino.h>
// #include "PID_v1.h"

#define KP 0  // Proportional drive constant
#define GAP 1 // Don't use drive values lower than this
#define RES 2 // Spatial resolution
#define KVTARGET 3  // Multiplier for target velocity as function of positional error
#define KVBOOST 4 // How much to boost drive per velocity error
#define GOALSMOOTHING 5 // Amount of smoothing to apply to goal [0-1]
#define DRIVESMOOTHING 6  // Amount of smoothing to apply to drive [0-1]
#define METRICS 7 // > 0.5 means diagnostics are output

class DriveUnit {
 private:
  long _posLow;
  long _posHigh;
  char* _channelName;
  int _pinEn;
  int _chRPWM;
  int _chLPWM;
  Adafruit_PWMServoDriver* _pwm;
  RotaryEncoder* _encoder;
  long _target;
  long _goal;
  long _drive;
  long _boost;
  long _pos;
  long _lastPos;
  long _lastT;
  double _v;
  double _targetV;
  Direction _direction;
  double _params[8];
  

  /* // PID control */
  /* double _pos; */
  /* double _pidOutput; */
  /* double _pidSetpoint; */
  /* PID* _pid; */

 public:
  DriveUnit(char* channelName,
            Direction direction,
            int pinEn,
            int chRPWM, int chLPWM,
            Adafruit_PWMServoDriver* pwm,
            RotaryEncoder* encoder) {
    _pinEn = pinEn;
    _chRPWM = chRPWM;
    _chLPWM = chLPWM;
    _pwm = pwm;
    _encoder = encoder;
    _drive = 0;
    _channelName = channelName;
    _direction = direction;
    _boost = 0;
    _params[KP] = 8.0;
    _params[GAP] = 500;
    _params[RES] = 0;
    _params[KVTARGET] = 0.5;
    _params[KVBOOST] = 0;
    _params[GOALSMOOTHING] = 0.1;
    _params[DRIVESMOOTHING] = 0.1;
    _params[METRICS] = 0;
  }

  boolean setParam(char* slot, double val) {
    if (eq(slot, "KP")) {
      _params[KP] = val;
    }
    else if (eq(slot, "GAP")) {
      _params[GAP] = val;
    }
    else if (eq(slot, "RES")) {
      _params[RES] = val;
    }
    else if (eq(slot, "KVTARGET")) {
      _params[KVTARGET] = val;
    }
    else if (eq(slot, "KVBOOST")) {
      _params[KVBOOST] = val;
    }
    else if (eq(slot, "GOALSMOOTHING")) {
      _params[GOALSMOOTHING] = val;
    }
    else if (eq(slot, "DRIVESMOOTHING")) {
      _params[DRIVESMOOTHING] = val;
    }
    else if (eq(slot, "METRICS")) {
      _params[METRICS] = val;
    }
    else {
      return false;
    }
    return true;
  }

  void calibrate() {
    int threshold = 500; // ms until we give up
    int now;
    int observed = _encoder->getPos();
    int minObserved = observed;
    long minObservedAt = millis();

    setDrive(-3750);

    do {
      now = millis();
      observed = _encoder->getPos();
      if (observed < minObserved) {
        minObserved = observed;
        minObservedAt = now;
      }
      delay(10);
    } while ((now - minObservedAt) < threshold);

    _posLow = minObserved;
    setDrive(0);
  }

  void setup() {
    pinMode(_pinEn, OUTPUT);

    // TODO: Dynamic calibration
    calibrate();
    _posHigh = _posLow + 600;

    _goal = 2500;
    seek(2500);
    /* _pid = new PID(&_pos, &_pidOutput, &_pidSetpoint, */
    /*                1.0, 0.0, 0.0, DIRECT); */
    /* _pid->SetOutputLimits(-10000.0, 10000.0); */
    /* //_pid->SetSampleTime(50); */
    /* _pid->SetMode(AUTOMATIC); */

    _lastPos = readPos();
    _lastT = millis();
  }

  char* getChannelName() {
    return _channelName;
  }

  long getTarget() {
    return _target;
  }

  long getDrive() {
    return _drive;
  }

  long getPos() {
    return _pos;
  }

  double getV() {
    return _v;
  }

  double getTargetV() {
    return _targetV;
  }

  // Set the seek position, normalized to the range [0 10000]
  void seek(long target) {
    _target = constrain(target, 0, 10000);
    //_pidSetpoint = (double) _target;
  }

  // Drive is in the range -10000 to 10000. The higher it is, the more
  // time the signal spends low.
  void setDrive(long drive)
  {
    _drive = drive;

    if (_direction == Clockwise) {
      drive *= -1;
    }

    // TODO: Do we want to be braking all the time?
    if (drive == 0) {
      digitalWrite(_pinEn, HIGH);
      _pwm->setPin(_chRPWM, 4095);
      _pwm->setPin(_chLPWM, 4095);
    }
    else if (drive > 0) {
      digitalWrite(_pinEn, HIGH);
      _pwm->setPin(_chLPWM, 4095);
      _pwm->setPin(_chRPWM, map(drive, 10000, 0, 40, 4095));
    }
    else {
      digitalWrite(_pinEn, HIGH);
      _pwm->setPin(_chRPWM, 4095);
      _pwm->setPin(_chLPWM, map(drive, 0, -10000, 4095, 40));
    }
  }

  long readPos() {
    long raw = _encoder->getPos();
    return map(raw, _posLow, _posHigh, 0, 10000);
  }

  void update() {
    long t = millis();
    long deltaT = t - _lastT;

    _goal = ((1.0 - _params[GOALSMOOTHING]) * _target) + _params[GOALSMOOTHING] * _goal;

    long pos = readPos();
    long posError = _goal - pos;

    long pDrive = posError * _params[KP];

    long deltaX = pos - _lastPos;
    _v = 0.0;
    if (deltaT != 0) {
      _v = deltaX / deltaT;
    }

    _targetV = posError * _params[KVTARGET];
    double deltaV = _targetV - _v;
    deltaV = constrain(deltaV, -10.0, 10.0);

    if (abs(posError) > 1000) {
      _boost = 0;
    }
    else {
      _boost += deltaV * _params[KVBOOST];
      _boost = constrain(_boost,
                        (posError < 0) ? -2000 : 0,
                        (posError < 0) ? 0 : 1000);
    }

    long drive = pDrive + _boost;

    // Ignore the center of the drive region - it doesn't move at all
    long gap = (long) _params[GAP];
    if ((abs(posError) < (long) _params[RES]) || (abs(drive) < 2)) {
      drive = 0;
    }
    else if (drive > 0) {
      drive = (drive * (10000 - gap) / 10000) + gap;
    }
    else if (drive < 0) {
      drive = (drive * (10000 - gap) / 10000) - gap;
    }

    drive = constrain(((1.0 - _params[DRIVESMOOTHING]) * drive) + _params[DRIVESMOOTHING] * _drive, 
                      -10000,
                      10000);
    setDrive(drive);

    _lastPos = pos;
    _lastT = t;

  }

  void printDiagnostics() {
    if (_params[METRICS] > 0.5) {
      char buf[128];
      char vs[32];
      dtos(vs, _v);
      char tvs[32];
      dtos(tvs, _targetV);
      sprintf(buf, "ch=%s,t=%ld,pos=%ld,target=%ld,drive=%ld,boost=%ld,v=%s,target-v=%s,goal=%ld",
              _channelName,
              micros(),
              readPos(),
              _target,
              _drive,
              _boost,
              vs,
              tvs,
              _goal);
      Serial.println(buf);
    }
  }

};
