#include <Servo.h>

Servo servoSG90;

void setup() {
  servoSG90.attach(6);
}

void loop() {
  servoSG90.write(45);
  delay(1000);

  servoSG90.write(90);
  delay(1000);

  servoSG90.write(135);
  delay(1000);
}