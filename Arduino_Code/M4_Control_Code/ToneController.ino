#define PULSE_DURATION_MS 5
#define PULSE_GAP_MS 10
#define POST_SIGNAL_DELAY 50 // Give M0 time to prepare

void sendTrialSignal(int numPulses) {
  for (int i = 0; i < numPulses; i++) {
    digitalWrite(12, HIGH);
    delay(PULSE_DURATION_MS);
    digitalWrite(12, LOW);
    if (i < numPulses - 1) delay(PULSE_GAP_MS);
  }
  delay(POST_SIGNAL_DELAY);  // Wait before tone window opens
}