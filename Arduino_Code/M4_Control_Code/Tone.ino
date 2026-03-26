  void customDelay(int milliseconds, bool inLockout = false, float remainingTime = 0.0, bool inITI = false);

  /**
    Plays a tone and sends output to Bonsai.
    Tone frequency and duration are specified in the function call.
    Output is sent to Bonsai via digital pin 7.
    The function also updates the display.

    @param frequency The frequency of the tone to be played.
    @param duration The duration of the tone in milliseconds.
  */

  void playTone(int frequency, int duration) {
    Serial.println("Playing Tone");
    isToneActive = true; // Set tone status to active

    tone(12, frequency, duration); // Play tone on pin 12

    // Signal tone start on pin 13
    digitalWrite(13, HIGH);
    
    // REMOVED: TTL pulses on pin 25 for spout position
    
    // Display the tone status during the tone duration
    unsigned long toneStartTime = millis(); 
    while (millis() - toneStartTime < duration) {
      float remainingTime = (duration - (millis() - toneStartTime)) / 1000.0;
      UpdateDisplay(false, remainingTime, false);
    }

    // Stop the tone and signal
    digitalWrite(13, LOW);
    noTone(12);
    isToneActive = false;
  }


