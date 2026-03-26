// Specify which spout position to dispense
void dispense(int selection, bool updateDisplay = true) {
  Serial.println("Dispense");

  // Dispense based on the selected spout position
  if (selection == 1) {
    digitalWrite(A1, HIGH);
    delay(volume * 10);
    digitalWrite(A1, LOW);
  }

  if (selection == 2) {
    digitalWrite(A2, HIGH);
    //digitalWrite(1, HIGH);
    delay(volume * 18);
    digitalWrite(A2, LOW);
    //digitalWrite(1, LOW);
  }

  if (selection == 3) {
    //digitalWrite(A3, HIGH);
    //delay(volume * 5);
    //digitalWrite(A3, LOW);
  }

  if (selection == 4) {
    digitalWrite(A4, HIGH);
    delay(volume * 20);
    digitalWrite(A4, LOW);
  }

  if (selection == 5) {
    //digitalWrite(A5, HIGH);
    //delay(volume * 20);
    //digitalWrite(A5, LOW);
  }

  // Update the display only if requested
  if (updateDisplay) {
    UpdateDisplay(false, 0.0, false);
  }
}
