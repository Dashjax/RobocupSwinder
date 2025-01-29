#include <DRV8825.h>

const int coil_motor_step = 24, coil_motor_dir = 25;
const int feed_motor_step = 38, feed_motor_dir = 39;
const int up_btn = 35, down_btn = 36, left_btn = 38, right_btn = 39, mid_btn = 37;

DRV8825 coil_motor;

void setup() {
  Serial.begin(9600);
  Serial.println("Hello World");
  // put your setup code here, to run once:
  //coil_motor.begin(coil_motor_dir, coil_motor_step);

  pinMode(coil_motor_step, OUTPUT);
  pinMode(coil_motor_dir, OUTPUT);
  pinMode(feed_motor_step, OUTPUT);
  pinMode(feed_motor_dir, OUTPUT);
  //coil_motor.setDirection(1);
  digitalWrite(coil_motor_dir, LOW);
  digitalWrite(feed_motor_dir, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 100; i++) {
    spinf();
  }

}

void spinc() {
  for (int i = 0; i < 200; i++) {
    digitalWrite(coil_motor_step, HIGH);
    delay(1);
    digitalWrite(coil_motor_step, LOW);
    delay(1);
  }
}

void spinf() {
  for (int i = 0; i < 200; i++) {
    digitalWrite(feed_motor_step, HIGH);
    delay(1);
    digitalWrite(feed_motor_step, LOW);
    delay(1);
  }
}