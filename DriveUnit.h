#include "common.h"
#include "Adafruit_PWMServoDriver.h"
#include <Arduino.h>
// #include "PID_v1.h"

#define KP 0  // Proportional drive constant
#define GAP 1 // Don't use drive values lower than this
#define RES 2 // Spatial resolution

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
  long _drive;
  long _pos;
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
    _params[KP] = 1.5;
    _params[GAP] = 1500;
    _params[RES] = 50;
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
    _posHigh = _posLow + 700;

    seek(2500);
    /* _pid = new PID(&_pos, &_pidOutput, &_pidSetpoint, */
    /*                1.0, 0.0, 0.0, DIRECT); */
    /* _pid->SetOutputLimits(-10000.0, 10000.0); */
    /* //_pid->SetSampleTime(50); */
    /* _pid->SetMode(AUTOMATIC); */
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

  void updateLinearWithGap() {
    long pos = readPos();
    long posError = _target - pos;

    long drive = posError * _params[KP];
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

    setDrive(constrain(drive, -10000, 10000));

  }

  void update() {
    updateLinearWithGap();
  }

  void updateBroken() {
    static long lastT = millis();
    static long lastPos = readPos();
    long decisionTime = 100; // msec
    double kv = 0.001; // Controls how quickly velocity should change
                    // based on position from target. [units/ms]
    double kd = 12.0;  // How much should we change the drive per unit of
                    // velocity we're off by?

    long t = millis();
    _pos = readPos();
    long deltaT = t - lastT;
    if (deltaT > decisionTime) {
      _v = (_pos - lastPos) / (double) deltaT;
      long posErr = _target - _pos;
      if (abs(posErr) < 150) {
        _targetV = 0;
      }
      else {
        _targetV = posErr * kv;
      }

      double deltaV = _targetV - _v;

      long drive = constrain((long) (_drive + (deltaV * abs(deltaV) * kd)),
                             -10000, 10000);

      setDrive(drive);

      lastT = t;
      lastPos = _pos;
    }
  }

  void printDiagnosticHeader() {
    Serial.println("ch,         t,       pos,    target,     drive,         v,   targetV");
  }

  void printDiagnostics() {
    char buf[128];
    /* char ts[32]; */
    /* dtos(ts, micros() / 1000000.0); */
    char vs[32];
    dtos(vs, _v);
    char tvs[32];
    dtos(tvs, _targetV);
    sprintf(buf, "%2s,%10ld,%10ld,%10ld,%10ld,%10s,%10s",
            _channelName,
            //ts,
            micros(),
            readPos(),
            _target,
            _drive,
            vs,
            tvs);
    Serial.println(buf);
  }

};