// ===== SERVO CONTROL FUNCTIONS =====
void extend(int selection, bool updateDisplayDuringExtend, unsigned long* toneStartTime, unsigned long toneDuration) {
  int extend_target = 0;

  if (selection == 1) {
    extend_target = 90;
  } else if (selection == 2) {
    extend_target = 25;
  } else if (selection == 3) {
    extend_target = 120;
  } else if (selection == 4) {
    extend_target = 97;
  } else if (selection == 5) {
    extend_target = 90;
  }

  servo_retract.attach(11);
  
  // Single-phase constant speed movement with optional display updates
  for (pos = currentServoPos; pos >= extend_target; pos -= 1) {
    servo_retract.write(pos);
    delay(5);
    
    // Update display with tone countdown if requested
    if (updateDisplayDuringExtend && toneStartTime != nullptr) {
      float remainingTime = (toneDuration - (millis() - *toneStartTime)) / 1000.0;
      if (remainingTime > 0) {
        UpdateDisplay(false, remainingTime, false, true, false);
      }
    }
  }
  
  currentServoPos = extend_target;  // Update tracked position
  servo_retract.detach();
}

void retract() {
  servo_retract.attach(11);
  
  // Retract means going UP in angle (away from mouse)
  for (pos = currentServoPos; pos <= 120; pos += 2) {
    servo_retract.write(pos);
    delay(5);
  }
  
  currentServoPos = 120;  // Update tracked position
  servo_retract.detach();
  UpdateDisplay(false, 0.0, false, false, false);
}