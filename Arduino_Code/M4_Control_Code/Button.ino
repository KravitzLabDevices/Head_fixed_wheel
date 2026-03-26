// void checkbuttons() {
//   while (digitalRead(6) == LOW) {  // Check if Button B is pressed
//     offset++;
//     UpdateDisplay(false, 0, false); // Update display with no timing info

//     int position = 0 + offset;
//     servo_rotate.attach(10);  // Attach servo to pin

//     if (servo_rotate.read() > position) {
//       for (int pos = servo_rotate.read(); pos >= position; pos -= 1) {
//         servo_rotate.write(pos);  // Move servo to new position
//         delay(20);                // Wait for servo to move
//       }
//     } else {
//       for (int pos = servo_rotate.read(); pos <= position; pos += 1) {
//         servo_rotate.write(pos);  // Move servo to new position
//         delay(20);                // Wait for servo to move
//       }
//     }
//     servo_rotate.detach();  // Detach servo from pin
//     timer = millis();
//     delay(200);  // Debounce delay
//   }

//   while (digitalRead(5) == LOW) {  // Check if Button C is pressed
//     offset--;
//     UpdateDisplay(false, 0, false); // Update display with no timing info

//     int position = 0 + offset;
//     servo_rotate.attach(10);  // Attach servo to pin

//     if (servo_rotate.read() > position) {
//       for (int pos = servo_rotate.read(); pos >= position; pos -= 1) {
//         servo_rotate.write(pos);  // Move servo to new position
//         delay(20);                // Wait for servo to move
//       }
//     } else {
//       for (int pos = servo_rotate.read(); pos <= position; pos += 1) {
//         servo_rotate.write(pos);  // Move servo to new position
//         delay(20);                // Wait for servo to move
//       }
//     }
//     servo_rotate.detach();  // Detach servo from pin
//     timer = millis();
//     delay(200);  // Debounce delay
//   }
// }