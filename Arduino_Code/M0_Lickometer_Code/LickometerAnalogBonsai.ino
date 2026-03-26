/*
  Simple SD card datalogger
  Records digital pulses on A1 at 20Hz

  Originally created  24 Nov 2010   modified 9 Apr 2012   by Tom Igoe
  Modified by Lex Kravitz Feb 2020
  Modified by Justin Wang Jun 2024

  This example code is in the public domain.
*/

//Include libraries
#include "Adafruit_FreeTouch.h"

//setup touch sensing
Adafruit_FreeTouch qt_4 = Adafruit_FreeTouch(A2, OVERSAMPLE_64, RESISTOR_0, FREQ_MODE_HOP);
int baseline;
int counter = 0, result = 0;


void setup() {
  Serial.begin(115200);                                         //Open serial communication

  //Set pinModes
  pinMode(LED_BUILTIN, OUTPUT);                                 //Red LED light for feedback
  pinMode(A4, OUTPUT);       //This is for triggering an output when lick is detected
  pinMode(13, OUTPUT);       //For sending digital output to red LED    
  pinMode(12, OUTPUT);       //For sending digital output to RWD photom                         

  //start touch sensor9
  qt_4.begin();

  //baseline touch sensor
  baseline = qt_4.measure();

}

void loop() {
  counter++;
  result = qt_4.measure();
  analogWrite(A0, result);
  Serial.println(result);
  delay (10);

  //licked detected
  if (result - baseline > 100) {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(A4, HIGH); 
    digitalWrite(13, HIGH); 
    digitalWrite(12, HIGH); 

  } else {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(A4, LOW);
    digitalWrite(13, LOW);
    digitalWrite(12, LOW);
  }

  if ((counter > 20000) and (result - baseline < 10)) {
    baseline = qt_4.measure();
    counter = 0;
  }
}
