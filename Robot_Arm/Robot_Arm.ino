/******************************************************************
 * author: Balázs Simon
 * e-mail: simon.balazs.1992@gmail.com
 * 
 * This code controls a robot arm. An Arduino 101 is used in this project. 
 * If you plan to use this code on another Arduino you might need to use Serial
 * instead of Serial1
 * More info:
 * https://www.hackster.io/Abysmal/robot-arm-controlled-through-ethernet-5ce9ce
 *
 * Modified by Richard Day:
 * Added wrist joint, etc
 ******************************************************************/

#include <Servo.h>
 
// Used pins
#define GRIPPER_SERVO 9
#define UPPER_JOINT_SERVO 6
#define LOWER_JOINT_SERVO 5
#define BASE_ROTATOR_JOINT_SERVO 3
#define WRIST_JOINT_SERVO 7

// Gripper states
#define GRIPPER_OPEN 0
#define GRIPPER_CLOSED_SOFT 1
#define GRIPPER_CLOSED_MEDIUM 2
#define GRIPPER_CLOSED_HARD 3

// Gripper configuration, specific to your robot
#define GRIPPER_OPEN_ANGLE 150            // The angle of the servo in different positions
#define GRIPPER_CLOSED_SOFT_ANGLE 75      // Closing with low force (less stressful on the servo)
#define GRIPPER_CLOSED_MEDIUM_ANGLE 65    // Closing with medium force (slightly stressful on the servo)
#define GRIPPER_CLOSED_HARD_ANGLE 0       // Closing with high force (quite stressful on the servo, it will also heat up)

// Limits, specific to your robot
#define SERVO_ANGLE_UPPER_LIMIT 170
#define SERVO_ANGLE_LOWER_LIMIT 0
#define SERVO_ANGLE_RESTING 90
#define MIN_MOVEMENT_DELAY 2

// Commands
#define MOVE_GRIPPER 'G'
#define MOVE_ARM 'A'

Servo gripper, upperJoint, lowerJoint, baseRotator, wristJoint;

unsigned int movementDelay = 5;

int gripperState;
int targetUpperJointAngle;
int targetLowerJointAngle;
int targetBaseRotatorAngle;
int targetWristJointAngle;

float currentUpperJointAngle, currentLowerJointAngle, currentBaseRotatorAngle, currentWristJointAngle;
float upperJointStep, lowerJointStep, baseRotatorStep, wristJointStep;

unsigned long lastStepTime = 0;

void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  gripper.attach(GRIPPER_SERVO);
  upperJoint.attach(UPPER_JOINT_SERVO);
  lowerJoint.attach(LOWER_JOINT_SERVO);
  baseRotator.attach(BASE_ROTATOR_JOINT_SERVO);
  wristJoint.attach(WRIST_JOINT_SERVO);

  // Setting default position on starting. Specific for your robot, so you 
  // might need to adjust it to yours
  gripperState = GRIPPER_CLOSED_SOFT;
  targetUpperJointAngle = currentUpperJointAngle = 140;
  targetLowerJointAngle = currentLowerJointAngle = 45;
  targetBaseRotatorAngle = currentBaseRotatorAngle = 85;
  targetWristJointAngle = currentWristJointAngle = 90;

  moveGripper(gripperState);
  moveTo(targetUpperJointAngle, targetLowerJointAngle, targetBaseRotatorAngle, targetWristJointAngle);
}

void loop() {
  if (Serial.available() > 0) {
    // The first character means the received command type.
    char command = Serial.read();

    if (command == MOVE_GRIPPER) {
      updateGripperState();
      moveGripper(gripperState);
      delay(250);
    }
    else if (command == MOVE_ARM) {
      updateArmMovement();
    }
  }

  if (millis() - lastStepTime >= movementDelay) {
    if (targetUpperJointAngle != round(currentUpperJointAngle)) currentUpperJointAngle -= upperJointStep;
    if (targetLowerJointAngle != round(currentLowerJointAngle)) currentLowerJointAngle -= lowerJointStep;
    if (targetBaseRotatorAngle != round(currentBaseRotatorAngle)) currentBaseRotatorAngle -= baseRotatorStep;
    if (targetWristJointAngle != round(currentWristJointAngle)) currentWristJointAngle -= wristJointStep;

    moveTo(currentUpperJointAngle, currentLowerJointAngle, currentBaseRotatorAngle, currentWristJointAngle);

    printPosition();
    lastStepTime = millis();
  }
}

void updateGripperState() {
  if (Serial.available() > 0)
    gripperState = Serial.parseInt();
  else
    return;

  clearSerial();
}

void updateArmMovement() {
  int mDelay;
  if (Serial.available() > 0)
    mDelay = Serial.parseInt();
  else
    return;

  int upperJointAngle;
  if (Serial.available() > 0)
    upperJointAngle = Serial.parseInt();
  else
    return;

  int lowerJointAngle;
  if (Serial.available() > 0)
    lowerJointAngle = Serial.parseInt();
  else
    return;

  int baseRotatorAngle;
  if (Serial.available() > 0)
    baseRotatorAngle = Serial.parseInt();
  else
    return;

  int wristJointAngle;
  if (Serial.available() > 0)
    wristJointAngle = Serial.parseInt();
  else
    return;
  clearSerial();

  // This calculation is used to slow down a movement of the robot arm segments.
  // mDelay is used as speed. Instantious movement would be too fast and it could
  // cause hardware issues.
  movementDelay = mDelay >= MIN_MOVEMENT_DELAY ? mDelay : MIN_MOVEMENT_DELAY;
  targetUpperJointAngle = upperJointAngle;
  targetLowerJointAngle = lowerJointAngle;
  targetBaseRotatorAngle = baseRotatorAngle;
  targetWristJointAngle = wristJointAngle;
  
  upperJointStep = (currentUpperJointAngle - targetUpperJointAngle) / 100;
  lowerJointStep = (currentLowerJointAngle - targetLowerJointAngle) / 100;
  baseRotatorStep = (currentBaseRotatorAngle - targetBaseRotatorAngle) / 100;
  wristJointStep = (currentWristJointAngle - targetWristJointAngle) / 100;
}

void clearSerial() {
  // Clearing the serial buffer. New line and/or carriage return characters
  // are expected to be in the buffer.
  while (Serial.available() > 0)
    Serial.read();
}

void moveGripper(int state) {
  switch (state) {
    case GRIPPER_OPEN:
      gripper.write(GRIPPER_OPEN_ANGLE);
      break;
    case GRIPPER_CLOSED_SOFT:
      gripper.write(GRIPPER_CLOSED_SOFT_ANGLE);
      break;
    case GRIPPER_CLOSED_MEDIUM:
      gripper.write(GRIPPER_CLOSED_MEDIUM_ANGLE);
      break;
    case GRIPPER_CLOSED_HARD:
      gripper.write(GRIPPER_CLOSED_HARD_ANGLE);
      break;
  }
}

void moveTo(int upperJointAngle, int lowerJointAngle, int baseRotatorAngle, int wristJointAngle) {
  // Moving the robot arm segments with updating the servos
  upperJoint.write(upperJointAngle);
  lowerJoint.write(lowerJointAngle);
  baseRotator.write(baseRotatorAngle);
  wristJoint.write(wristJointAngle);
}

void printPosition() {
  // Sending back feedback
/*  
  Serial1.print(movementDelay);
  Serial1.print(" ");
  Serial1.print(gripperState);
  Serial1.print(" ");
  Serial1.print(round(currentUpperJointAngle));
  Serial1.print(" ");
  Serial1.print(round(currentLowerJointAngle));
  Serial1.print(" ");
  Serial1.println(round(currentBaseRotatorAngle));
*/
  Serial.println("ok");
}
