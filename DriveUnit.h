#include "common.h"
#include "Adafruit_PWMServoDriver.h"

class DriveUnit {
 private:
  int _pos;
  int _posLow;
  int _posHigh;
  String _channelName;
  int _pinEn;
  int _chRPWM;
  int _chLPWM;
  Adafruit_PWMServoDriver* _pwm;
  RotaryEncoder* _encoder;
  int _target;
  int _drive;
  Direction _direction;

 public:
  DriveUnit(String channelName,
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
    setDrive(-5000);

    do {
      now = millis();
      observed = _encoder->getPos();
      if (observed < minObserved) {
        minObserved = observed;
        minObservedAt = now;
      }
      delay(10);
    } while ((now - minObservedAt) < threshold);

    int maxObserved = minObserved;
    long maxObservedAt = millis();
    setDrive(5000);
    
    do {
      now = millis();
      observed = _encoder->getPos();
      if (observed > maxObserved) {
        maxObserved = observed;
        maxObservedAt = now;
      }
      delay(10);
    } while ((now - maxObservedAt) < threshold);

    _posLow = minObserved;
    _posHigh = maxObserved;
    
  }
  
  void setup() {
    pinMode(_pinEn, OUTPUT);

    // TODO: Dynamic calibration
    int initialPos = _encoder->getPos();
    _posLow = initialPos - 10;
    _posHigh = initialPos + 10;
  }

  String getChannelName() {
    return _channelName;
  }

  int getTarget() {
    return _target;
  }

  int getDrive() {
    return _drive;
  }

  int getPos() {
    return _pos;
  }

  // Set the seek position, normalized to the range [-10000 10000]
  void seek(int target) {
    _target = constrain(target, 0, 10000);
  }

  // Drive is in the range -10000 to 10000. The higher it is, the more
  // time the signal spends low.
  void setDrive(int drive)
  {
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

  void update() {
    // Everything is normalized to the range either 0-10000 or -10000-10000,
    // depending on which makes more sense.

    // The encoder, of course, just counts up and down.
    int raw = _encoder->getPos();
    _pos = map(raw, _posHigh, _posLow, 0, 10000);

    int diff = _target - _pos;
    int drive = constrain(diff * 2, -10000, 10000);

    setDrive(drive);
  }
};
