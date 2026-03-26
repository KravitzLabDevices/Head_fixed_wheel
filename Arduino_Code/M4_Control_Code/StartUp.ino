void startup() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Press A to start");
  display.println("Press B to flush");
  display.display();

  bool washActive = false;  // Flag to keep wash function active once Button B is pressed

  while (digitalRead(BUTTON_A) == HIGH) {  // Wait until Button A is pressed
    if (digitalRead(BUTTON_B) == LOW) {    // Check if Button B is pressed
      washActive = true;                   // Set the washActive flag
    }
    
    if (washActive) {
      wash();                              // Call wash repeatedly if washActive is true
    }
  }
  
  randomSeed(millis());                    // Seed the random generator once Button A is pressed
}

