void customDelay(int milliseconds, bool inLockout, float remainingTime, bool inITI) {
    unsigned long startTime = millis(); // get the current time

    while (millis() - startTime < milliseconds) {
        float updatedRemainingTime = remainingTime - (millis() - startTime) / 1000.0; // Calculate dynamic remaining time
        UpdateDisplay(inLockout, updatedRemainingTime > 0 ? updatedRemainingTime : 0.0, inITI); // Update display during delay
    }
}
