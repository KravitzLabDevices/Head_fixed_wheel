void UpdateDisplay(bool inLockout, float remainingTime, bool inITI, bool inTone, bool inPreDispense) {
  display.clearDisplay();

  // Display Trial Number
  display.setCursor(0, 0);
  display.print("Trial: ");
  display.println(Trial);

  // Display phase-specific countdown
  if (inLockout) {
    display.print("Lockout: ");
    display.print(remainingTime > 0 ? remainingTime : 0, 2);
  } else if (inITI) {
    display.print("ITI: ");
    display.print(remainingTime > 0 ? remainingTime : 0, 2);
  } else if (inTone) {
    display.print("Tone: ");
    display.print(remainingTime > 0 ? remainingTime : 0, 2);
  } else if (inPreDispense) {
    display.print("Pre-disp: ");
    display.print(remainingTime > 0 ? remainingTime : 0, 2);
  } else {
    display.print("Trial time: ");
    display.print(remainingTime > 0 ? remainingTime : 0, 2);
  }
  display.print(" sec");

  // Display Spout Position
  display.setCursor(0, 24);
  display.print("Spout: ");
  display.println(selection);

  // Display Offset
  display.setCursor(0, 34);
  display.print("Offset: ");
  display.println(offset);

  display.display();
}