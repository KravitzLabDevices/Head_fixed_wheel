// Inter-trial delay, specify low to high range in seconds
void timeout(int low, int high) {
  random_delay = random(low, high);
  startTimeout = millis();
  
  // Print delay and countdown for timeout
  while (millis() - startTimeout < random_delay * 1000) {
    float remainingTime = random_delay - (millis() - startTimeout) / 1000.0;
    UpdateDisplay(false, remainingTime, false); // Not in lockout or ITI, show trial countdown
  }
  
  Trial++; // Increment trial count after timeout
}
