#include "Adafruit_FreeTouch.h"

Adafruit_FreeTouch qt_4 = Adafruit_FreeTouch(A2, OVERSAMPLE_64, RESISTOR_0, FREQ_MODE_NONE);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  qt_4.begin();
}

void loop() {
  int counter, result = 0;

  counter = millis();
  result = qt_4.measure();
  Serial.print("QT 4: "); Serial.print(result);
  Serial.print(" (");  Serial.print(millis() - counter); Serial.println(" ms)");

  analogWrite(A0, result);

  if (result > 500) {
    digitalWrite (LED_BUILTIN, HIGH);
  }
  else {
    digitalWrite (LED_BUILTIN, LOW);
  }
}
