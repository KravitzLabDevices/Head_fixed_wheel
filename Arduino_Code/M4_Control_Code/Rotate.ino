/**
     """
  Move the servo motor to the specified spout position.

  Parameters:
  selection (int): the spout position to move the servo to, from 1 to 5.

  Returns:
  None

  """
*/

void spoutPosition(int selection) {
  //retract();
  int target = 0;
  if (selection == 1) {
    target = 21 + offset;
    analogWrite (A0, 51);
  }
  if (selection == 2) {
    target = 63 + offset;
    analogWrite (A0, 102);
  }
  if (selection == 3) {
    target = 103 + offset;
    analogWrite (A0, 153);
  }
  if (selection == 4) {
    target = 141 + offset;
    analogWrite (A0, 204);
  }
  if (selection == 5) {
    target = 170 + offset;
    analogWrite (A0, 253);
  }

  servo_rotate.attach(10);  // attach servo to pin
  if (servo_rotate.read() > target) {
    for (pos = servo_rotate.read(); pos >= target; pos -= 1) {
      servo_rotate.write(pos);  // move servo to new position
      delay(10);                // wait for servo to move
    }
  } else {
    for (pos = servo_rotate.read(); pos <= target; pos += 1) {
      servo_rotate.write(pos);  // move servo to new position
      delay(10);                // wait for servo to move
    }
  }
  servo_rotate.detach();  // detach servo from pin
  //extend(selection);
}
