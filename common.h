#include <Arduino.h>

#ifndef __COMMON_H_INCLUDED
#define __COMMON_H_INCLUDED
enum LogLevel {
  Info = 0,
  Debug = 1
};

LogLevel _logLevel = Info;

enum Direction {
  Left,
  Right
};

typedef struct token_s {
  char val[16];
  char* next;
} Token;

void debug(int i) {
  if (_logLevel >= Debug) {
    Serial.print(i);
  }
}

void debug(long l) {
  if (_logLevel >= Debug) {
    Serial.print(l);
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

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
   const byte* p = (const byte*)(const void*)&value;
   int i;
   for (i = 0; i < sizeof(value); i++) {
     EEPROM.write(ee++, *p++);
   }
   return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
   byte* p = (byte*)(void*)&value;
   int i;
   for (i = 0; i < sizeof(value); i++) {
     *p++ = EEPROM.read(ee++);
   }
   return i;
}

void dtos(char* buf, double val) {
  dtostrf(val, 10, 4, buf);
}
#endif
