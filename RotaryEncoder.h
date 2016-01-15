#include <Arduino.h>

#define bitRead(port, mask) (((*port) & mask) ? 1 : 0)

class RotaryEncoder {
 public:
  volatile long _pos = 0;

 private:
  uint8_t _pin1;
  uint8_t _pin2;
  RoReg* _port1;
  RoReg* _port2;
  unsigned long _mask1;
  unsigned long _mask2;

 public:
  RotaryEncoder(uint8_t pin1, uint8_t pin2) {
    _pin1 = pin1;
    _pin2 = pin2;

    _port1 = portInputRegister(digitalPinToPort(pin1));
    _port2 = portInputRegister(digitalPinToPort(pin2));

    _mask1 = digitalPinToBitMask(pin1);
    _mask2 = digitalPinToBitMask(pin2);
  }

  long getPos() {
    return _pos;
  }

  void setup() {
    // Configure the selected pins as input and turn on the pull-up
    // resistor.
    pinMode(_pin1, INPUT_PULLUP);
    pinMode(_pin2, INPUT_PULLUP);
  }

  void update() {
    // I stole this extremely clever code from here:
    // https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
    static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    static uint8_t old_AB = 0;


    int8_t new_AB = (bitRead(_port1, _mask1) << 1) | bitRead(_port2, _mask2);

    old_AB <<= 2;      //remember previous state
    old_AB |= new_AB;  //add current state

    _pos += enc_states[( old_AB & 0x0f )];
  }
};
