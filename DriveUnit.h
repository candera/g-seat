#include "common.h"
#include "Adafruit_PWMServoDriver.h"

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
  Direction _direction;
  double _v;
  double _targetV;
  long _pos;
  double _t;
  double _acc = 0.001;
  double _driveFactor = 0.01;

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
    _target = 2500;
    _drive = 0;
    _channelName = channelName;
    _direction = direction;
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
      if (((_direction == Left) && (observed < minObserved)) ||
          ((_direction == Right) && (observed > minObserved))) {
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
    if (_direction == Left) { 
      _posHigh = _posLow + 340;
    }
    else {
      _posHigh = _posLow - 340;
    }
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

  long getV() {
    return _v;
  }

  double getT() {
    return _t;
  }

  double getTargetV() {
    return _targetV;
  }

  void setAcc(double acc) {
    _acc = acc;
  }

  void setDriveFactor(double driveFactor) {
    _driveFactor = driveFactor;
  }

  // Set the seek position, normalized to the range [0 10000]
  void seek(long target) {
    _target = constrain(target, 0, 10000);
  }

  // Drive is in the range -10000 to 10000. The higher it is, the more
  // time the signal spends low.
  void setDrive(long drive)
  {
    if (_direction == Right) {
      drive *= -1;
    }
    
    _drive = drive;

    // TODO: Do we want to be braking all the time?
    if (drive == 0) {
      digitalWrite(_pinEn, HIGH);
      _pwm->setPin(_chRPWM, 4095);
      _pwm->setPin(_chLPWM, 4095);
    }
    else if (drive < 0) {
      digitalWrite(_pinEn, HIGH);
      _pwm->setPin(_chLPWM, 4095);
      _pwm->setPin(_chRPWM, map(drive, -10000, 0, 40, 4095));
    }
    else {
      digitalWrite(_pinEn, HIGH);
      _pwm->setPin(_chRPWM, 4095);
      _pwm->setPin(_chLPWM, map(drive, 0, 10000, 4095, 40));
    }
  }

  long readPos() {
    long raw = _encoder->getPos();
    return map(raw, _posLow, _posHigh, 0, 10000);
  }

  void updatePos() {
    _pos = readPos();
  }

  // Time in seconds
  double now() {
    return micros() / 1000000.0;
  }
  
  void updateAdaptive() {
    // Everything is normalized to the range either 0-10000 or -10000-10000,
    // depending on which makes more sense.
    // The encoder, of course, just counts up and down.

    // t: seconds
    // v: units/sec

    static long t0 = micros();
    static long pos0 = readPos();
    static double v0 = 0;

    long pos1 = readPos();
    double t1 = now();
    double dt = t1 - t0;
    double v1 = (pos1 - pos0) / dt; // TODO: Smooth this?
    v1 = (v1 * 0.1) + (v0 * 0.9);
    double a1 = (v1 - v0) / dt;

    long x1 = pos1 - _target;
    long x2 = x1 + v1 * 0.1;
    double vt = constrain(-x2 * 0.01, -250, 250);
    /* if (x2 > 0) { */
    /*   vt *= -1; */
    /* } */

    double a = _acc * x1;
    double b = _acc * v1 * dt;
    double c = 0.5 * _acc * dt * dt;

    double a2 = -(a + b + v1)/(dt + c);

    // long driveAdjustment = (a2 - a1) * _driveFactor * dt;
    //long driveAdjustment = (vt - v1) * _driveFactor;

    if ((x1 < 100) && (x1 > -100) && (v1 < 1)) {
      _drive = 0;
    }
    else {
      if ((abs(v1) > (abs(vt) * 1.5)) ||
          (abs(v1) < (abs(vt) * 0.50))) {
        
        long driveTarget;
        if (vt > v1) {
          if (x1 < 0) {
            driveTarget = 10000;
          }
          else {
            driveTarget = 0;
          }
        }
        else {
          if (x1 < 0) {
            driveTarget = 0;
          }
          else {
            driveTarget = -10000;
          }
        }
        if (driveTarget == 0) {
          if (x1 < 0) {
            _drive = -100;
          }
          else {
            _drive = 100;
          }
        }
        else {
          _drive = ((1.0 - _driveFactor) * _drive) +
            (_driveFactor * driveTarget);
        }
      }
      /* if (vt > 0) { */
      /*   _drive = constrain(_drive + driveAdjustment, 0, 10000); */
      /* } */
      /* else { */
      /*   _drive = constrain(_drive + driveAdjustment, -10000, 0); */
      /* } */
    }

    _v = v1;
    _targetV = vt;
    _pos = pos1;
    _t = t1;

    t0 = t1;
    pos0 = pos1;
    v0 = v1;
  }

  void linearUpdate() {
    static long lastT = micros();
    static long lastPos = readPos();
    static double lastV = 0;

    _pos = readPos();
    _t = now();
    _v = (lastPos - _pos) / (lastT - _t);

    long diff = _target - _pos;
    _targetV = diff;

    long drive;
    if ((diff < 50) && (diff > -50)) {
      drive = 0;
    }
    else if (diff > 0) {
      drive = map(diff, 0, 2000, 1200, 10000);
    }
    else {
      drive = map(diff, 0, -2000, -1200, -10000); 
    }

    if (drive > 0) {
      drive = (long) sqrt((double)drive);
    }
    else {
      drive = - (long) sqrt((double) -drive);
    }
      
    _drive = constrain((long) (drive * _driveFactor), -10000, 10000);

    lastT = _t;
    lastPos = _pos;
    lastV = _v;
  }

  void update() {
    updateAdaptive();
  }
};
