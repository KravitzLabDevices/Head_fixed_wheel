void lockout() {
  float locktime = truncatedExponential(LOCKOUT_MIN, LOCKOUT_MAX, LOCKOUT_LAMBDA);
  
  unsigned long currentTime = millis();
  unsigned long lockStartTime;
  
  // Only use lickDetectionTime if it's recent (within last 100ms)
  // Otherwise, there's a stale timestamp that will cause premature lockout end
  if (lickDetectionTime > 0 && (currentTime - lickDetectionTime) < 100) {
    lockStartTime = lickDetectionTime;
    Serial.print("Using ISR timestamp from ");
    Serial.print(currentTime - lickDetectionTime);
    Serial.println(" ms ago");
  } else {
    lockStartTime = currentTime;
    if (lickDetectionTime > 0) {
      Serial.print("WARNING: Stale lickDetectionTime detected (");
      Serial.print(currentTime - lickDetectionTime);
      Serial.println(" ms old), using current time instead");
    }
  }
  lickDetectionTime = 0;  // Reset for next use
  
  unsigned long lastDisplayUpdate = 0;
  const unsigned long DISPLAY_UPDATE_INTERVAL = 10;
  
  // Variables for debouncing lick detection during lockout
  unsigned long lastLockoutLickTime = 0;
  const unsigned long LOCKOUT_LICK_DEBOUNCE = 50;
  
  Serial.print("Lockout duration: ");
  Serial.print(locktime);
  Serial.println(" seconds");
  
  Serial.print("Starting lockout at time: ");
  Serial.println(lockStartTime);
  Serial.print("Current time: ");
  Serial.println(currentTime);
  Serial.print("Time already elapsed: ");
  Serial.print(currentTime - lockStartTime);
  Serial.println(" ms");

  // LED ON at start of lockout
  digitalWrite(1, HIGH);
  UpdateDisplay(true, locktime, false, false, false);
  lastDisplayUpdate = millis();

  while (millis() - lockStartTime < locktime * 1000) {
    currentTime = millis();
    
    // Update display periodically
    if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
      float lockoutRemainingTime = locktime - (currentTime - lockStartTime) / 1000.0;
      UpdateDisplay(true, lockoutRemainingTime, false, false, false);
      lastDisplayUpdate = currentTime;
    }

    // Detect licks with debouncing
    if (digitalRead(4) == HIGH) {
      unsigned long timeSinceLastLockoutLick = currentTime - lastLockoutLickTime;
      
      if (timeSinceLastLockoutLick >= LOCKOUT_LICK_DEBOUNCE) {
        Serial.println("Lick detected during lockout, resetting");
        
        // Reset lockout timer to NOW
        lockStartTime = currentTime;
        lastLockoutLickTime = currentTime;
        
        // Ensure LED stays HIGH
        digitalWrite(1, HIGH);
        
        // Update display with full lockout time
        UpdateDisplay(true, locktime, false, false, false);
        lastDisplayUpdate = currentTime;
        
        // Wait for lick to be released (with timeout)
        unsigned long waitStart = currentTime;
        while (digitalRead(4) == HIGH && (millis() - waitStart < 100)) {
          // Just wait
        }
      }
    }
  }
  
  // LED OFF at end of lockout
  digitalWrite(1, LOW);
  
  Serial.print("Lockout completed at time: ");
  Serial.println(millis());
  Serial.print("Total lockout duration: ");
  Serial.print((millis() - lockStartTime) / 1000.0);
  Serial.println(" seconds");
}