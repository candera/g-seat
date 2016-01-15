#include <Arduino.h>

#ifndef __COMMON_H_INCLUDED
#define __COMMON_H_INCLUDED
enum LogLevel {
  Info = 0,
  Debug = 1
};

LogLevel _logLevel = Info;

enum Direction {
  Clockwise,
  CounterClockwise
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

void dtos(char* buf, double val) {
  dtostrf(val, 10, 4, buf);
}

boolean eq(char* s1, char* s2) {
  return strcmp(s1, s2) == 0;
}

#endif
