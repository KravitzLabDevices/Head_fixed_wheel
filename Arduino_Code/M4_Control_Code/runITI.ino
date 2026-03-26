void runITI() {
  float itiDuration = random(ITI_MIN * 1000, ITI_MAX * 1000) / 1000.0;
  unsigned long itiStartTime = millis();
  unsigned long lastDisplayUpdate = 0;
  const unsigned long DISPLAY_UPDATE_INTERVAL = 50;
  
  lickDetectedFlag = false;
  lickDetectionTime = 0;  // Clear timestamp
  inITIPhase = true;
  
  while (millis() - itiStartTime < itiDuration * 1000) {
    unsigned long currentTime = millis();
    
    if (lickDetectedFlag) {
      lickDetectedFlag = false;
      inITIPhase = false;
      lockout();  // Will use lickDetectionTime from ISR
      Serial.println("ITI restarting after lockout");
      runITI();
      return;
    }
    
    if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
      float itiRemainingTime = itiDuration - (currentTime - itiStartTime) / 1000.0;
      UpdateDisplay(false, itiRemainingTime, true, false, false);
      lastDisplayUpdate = currentTime;
    }
  }
  
  if (lickDetectedFlag) {
    lickDetectedFlag = false;
    inITIPhase = false;
    lockout();
    Serial.println("ITI restarting after lockout (caught at end)");
    runITI();
    return;
  }
  
  inITIPhase = false;
}