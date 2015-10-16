/* Adapts a rotary encoder into two DirectX buttons - one for each
 * direction of rotation */
class RotaryEncoder {
 public:
  volatile long _pos;
  volatile long _up1 = 0;
  volatile long _up2 = 0;
  volatile long _down1 = 0;
  volatile long _down2 = 0;
  volatile bool _last1;
  volatile bool _last2;
  volatile int _lastEvent;
  volatile int _penultimateEvent;
 private:
  int _in1;
  int _in2;

 public:
  RotaryEncoder(int in1, int in2) {

    _in1 = in1;
    _in2 = in2;
    _last1 = false;
    _last2 = false;
    _lastEvent = 0;
    _penultimateEvent = 0;
  }

  long getPos() {
    return _pos;
  }

  void setup() {
    pinMode(_in1, INPUT_PULLUP);
    pinMode(_in2, INPUT_PULLUP);
  }

  void update() {
    bool val1 = digitalRead(_in1);
    bool val2 = digitalRead(_in2);

    /* if (!val1) { */
    /*   ++_pos; */
    /* } */
    /* else if (!val2) { */
    /*   --_pos; */
    /* } */

    /* Serial.print("val1:"); */
    /* Serial.println(val1); */
    /* Serial.print("val2:"); */
    /* Serial.println(val2); */
    /* Serial.print("last1:"); */
    /* Serial.println(_last1); */
    /* Serial.print("last2:"); */
    /* Serial.println(_last2); */
    /* Serial.print("last event:"); */
    /* Serial.println(_lastEvent); */
    /* Serial.print("lastlast event:"); */
    /* Serial.println(_penultimateEvent); */

    if (val1 && !_last1) {
      _penultimateEvent = _lastEvent;
      _lastEvent = 1;
      ++_up1;
    }
    else if (!val1 && _last1) {
      _penultimateEvent = _lastEvent;
      _lastEvent = -1;
      ++_down1;
    }

    if (val2 && !_last2) {
      _penultimateEvent = _lastEvent;
      _lastEvent = 2;
      ++_up2;
    }
    else if (!val2 && _last2) {
      _penultimateEvent = _lastEvent;
      _lastEvent = -2;
      ++_down2;
    }

    if (_lastEvent == -2 && _penultimateEvent == -1) {
      ++_pos;
      _lastEvent = 0;
      _penultimateEvent = 0;
    }
    else if (_lastEvent == -1 && _penultimateEvent == -2) {
      --_pos;
      _lastEvent = 0;
      _penultimateEvent = 0;
    }

    _last1 = val1;
    _last2 = val2;
  }
};
