#include <Servo.h>
 
// CONSTANTS - keys for parsing messages via serial
#define GRAB 'G'
#define SWING 'S'
#define ROTATE 'R'
#define ANALOG_GRAB 'A'
 
// CONSTANTS - message length, servo count, and debug
#define MSG_LEN 16
#define SERVOS 3
 
// Vars that hold reference to each servo object
Servo one;
Servo two;
Servo three;
 
boolean debug = false;
 
// Arrays to hold information about state and mapping for each servo
char ctrlIds [SERVOS] = {GRAB,ROTATE,SWING};
int curPos [SERVOS] = {0,0,0};
int newPos [SERVOS] = {0,0,0};
int lowerMap [SERVOS] = {0,0,0};
int upperMap [SERVOS] = {1023,1023,1023};
int lowerConstrainServo [SERVOS] = {20, 0, 20}; // lowerConstrainServo for ACTUAL motor position
int upperConstrainServo [SERVOS] = {200, 140, 100}; // upperConstrainServo for ACTUAL motor position
boolean servoActive[SERVOS] = {false, false, false};
 
// Miscellaneous variables
boolean analogGrab = false; // when analogGrab is set to true then the claw will accept precise controls (0-1023)
int stepDelay = 30; // holds the internal between servo steps in millis
 
// Variables associated to message handling
int messageLength = 0; // stores current array position when reading new data
char message [MSG_LEN] = {'\0','\0','\0','\0','\0',
'\0','\0','\0','\0','\0',
'\0','\0','\0','\0','\0',
'\0'};
void setup() {
Serial.begin(9600);
 
// initialize each servo by passing the pin number to which it is attached
one.attach(9); // grab
two.attach(10); // rotate
three.attach(11); // swing
}
 
void loop() {
handleSerial();
 
// loop through each of the servos to check if their position needs adjustment
for (int k = 0; k < SERVOS; k++) {
 
// if a new servo position has been received or servo has not yet reached final position
// then move the servo
if (curPos[k] != newPos[k]) {
if (servoActive[k] == false) servoActive[k] = true;
 
// move position upwards if new position is greater than current position
if (curPos[k] < newPos[k]) {
curPos[k] += 1;
}
 
// move position downwards if new position is lower than current position
else if (curPos[k] > newPos[k]) {
curPos[k] -= 1;
}
 
updatePos(k, curPos[k]); // update the motors current position
delay(stepDelay);
}
 
// otherwise, set servo state to false
else {
if (servoActive[k] == true) {
servoActive[k] = false;
 
// print message to serial if servo reached final location
if (debug) {
Serial.print("OK: "); Serial.print(ctrlIds[k]);
Serial.print(" to ");
Serial.println(map(curPos[k], lowerConstrainServo[k], upperConstrainServo[k], lowerMap[k], upperMap[k]));
}
}
}
}
}
 
// update the position of the appropriate motor
void updatePos(int motor, int pos) {
if (motor == 0) one.write(pos);
else if (motor == 1) two.write(pos);
else if (motor == 2) three.write(pos);
 
if (debug) { Serial.print(F("[updatePos] motor: ")); Serial.print(ctrlIds[motor]); }
if (debug) { Serial.print(F(" pos: ")); Serial.println(pos); }
}
 
// check serial port and read data as appropriate
void handleSerial() {
while (Serial.available()) {
// get the new byte:
char inChar = (char)Serial.read();
 
// if the incoming character is a newline then parse message
if (inChar == '\n' || messageLength >= MSG_LEN) {
if (debug) { Serial.print(F("\n[handleSerial] ")); Serial.println(message); }
parseMessage();
}
 
else if (inChar == 'D') {
debug = true;
if (debug) { Serial.println(F("\n[handleSerial] debug ON")); }
else { Serial.println(F("\n[handleSerial] debug OFF")); }
}
 
// add it to the message
else {
message[messageLength] = inChar;
messageLength++;
// if (debug) Serial.print(inChar);
}
}
}
 
void parseMessage() {
for (int i = 0; i < messageLength; i++) {
if (message[i] == ANALOG_GRAB) {
analogGrab = true;
break;
}
}
for (int i = 0; i < messageLength; i++) {
// variable used to catch instances where variable 'i' has been overly incremented
boolean loaded = false;
 
for (int j = 0; j < SERVOS; j++) {
 
if (message[i] == ctrlIds[j] && i < messageLength) {
if (debug) {
Serial.print(ctrlIds[j]);
Serial.println(F("[parseMessage] found ID, reading numbers: "));
}
 
// increment index to point to first number in the message
i++;
 
// if index is incremented beyond message size the break out of the loop
if (i >= messageLength) break;
 
// hold location/index of first number in main message array
int first_num = i;
// create temp array to convert ascii numbers
char convert_val [5] = {'\0','\0','\0','\0','\0'};
while ((message[i] >= '0' && message[i] <= '9') && (i - first_num < 4)) {
if (debug) { Serial.print("... "); Serial.println(message[i]); }
convert_val[(i - first_num)] = message[i]; // transfer number to convert array
i++; // move to next element
loaded = true; // set loaded flag to true
 
// if we reached the end of the main message then process number
if (i >= messageLength) break;
}
 
newPos[j] = atoi(convert_val); // convert ascii number to integer
if (debug) {
Serial.print(ctrlIds[j]); Serial.print(": pre-mapping to newPos: ");
Serial.println(newPos[j]);
}
 
// constrain number to appropriate range
newPos[j] = constrain(newPos[j], lowerMap[j], upperMap[j]);
 
// if boolean grab message receive then process command
if (!analogGrab && ctrlIds[j] == GRAB) {
newPos[j] = constrain(newPos[j], lowerMap[j], 1);
newPos[j] = map(newPos[j], lowerMap[j], 1, lowerConstrainServo[j], upperConstrainServo[j]);
}
 
// otherwise map position variable to appropriate range
else {
newPos[j] = map(newPos[j], lowerMap[j], upperMap[j], lowerConstrainServo[j], upperConstrainServo[j]);
}
 
if (debug) {
Serial.print(ctrlIds[j]);
Serial.print(": post-mapping to newPos: ");
Serial.print(newPos[j]);
Serial.print(" from curPos: ");
Serial.println(curPos[j]); }
}
}
if (loaded) i -= 1;
}
resetMessage();
}
 
// reset message
void resetMessage() {
for (int i = 0; i < MSG_LEN; i ++) message[i] = '\0';
messageLength = 0;
}
