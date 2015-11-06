enum Port {
  B, C, D, E, F
};

class RotaryEncoder {
 public:
  volatile long _pos = 0;
 private:
  volatile unsigned char* _pin1;
  volatile unsigned char* _pin2;
  volatile unsigned char* _ddr1;
  volatile unsigned char* _ddr2;
  volatile unsigned char* _port1;
  volatile unsigned char* _port2;
  unsigned char _bit1;
  unsigned char _bit2;

 private:
  volatile unsigned char* getPortAddr(Port port) {
    switch (port) {
    /* case A: */
    /*   return &PORTA; */
    case B:
      return &PORTB;
    case C:
      return &PORTC;
    case D:
      return &PORTD;
    case E:
      return &PORTE;
    case F:
      return &PORTF;
    }
  }
  volatile unsigned char* getPortDirAddr(Port port) {
    switch (port) {
    /* case A: */
    /*   return &PORTA; */
    case B:
      return &DDRB;
    case C:
      return &DDRC;
    case D:
      return &DDRD;
    case E:
      return &DDRE;
    case F:
      return &DDRF;
    }
  }
  volatile unsigned char* getPinAddr(Port port) {
    switch (port) {
    /* case A: */
    /*   return &PORTA; */
    case B:
      return &PINB;
    case C:
      return &PINC;
    case D:
      return &PIND;
    case E:
      return &PINE;
    case F:
      return &PINF;
    }
  }

 public:
  RotaryEncoder(Port port1, unsigned char bit1,
                Port port2, unsigned char bit2) {

    _port1 = getPortAddr(port1);
    _pin1 = getPinAddr(port1);
    _ddr1 = getPortDirAddr(port1);
    _bit1 =  bit1;
    _port2 = getPortAddr(port2);
    _pin2 = getPinAddr(port2);
    _ddr2 = getPortDirAddr(port2);
    _bit2 = bit2;
  }

  long getPos() {
    return _pos;
  }

  void setup() {
    // Configure the selected pins as input and turn on the pull-up
    // resistor.
    *_ddr1 &= ~(1<<_bit1);
    *_port1 |= (1<<_bit1);

    *_ddr1 &= ~(1<<_bit2);
    *_port1 |= (1<<_bit2);
  }

  void update() {
    // I stole this extremely clever code from here:
    // https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
    static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    static uint8_t old_AB = 0;

    int8_t new_AB = (bitRead(*_pin1, _bit1) << 1) | bitRead(*_pin2, _bit2);

    old_AB <<= 2;      //remember previous state
    old_AB |= new_AB;  //add current state

    _pos += enc_states[( old_AB & 0x0f )];
  }
};
