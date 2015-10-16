enum LogLevel {
  Info = 0,
  Debug = 1
};

LogLevel _logLevel = Info;

enum Direction {
  Left,
  Right
};

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

