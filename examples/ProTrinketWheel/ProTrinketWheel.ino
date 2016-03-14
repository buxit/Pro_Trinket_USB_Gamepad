/*
 Joystick Gamepad Control
 
 Controls a PC gamepad from a joystick on an Adafruit Pro Trinket.
 Uses the joystick pushbutton to click the left gamepad button
 
 Hardware:
 * 2-axis joystick connected to pins A0 and A1 with pushbutton on D10
 * Pushbutton enable/disable the gamepad entirely on D9 (optional)
 
 The gamepad movement is always relative. 
 
 The sketch assumes that the joystick resting values are around the 
 middle of the range, but that they vary within a threshold.
 
 WARNING:  When you use the new gamepad, the Arduino takes
 over your gamepad!  Make sure you have control before you use the project.
 This sketch includes a pushbutton to toggle the gamepad on and off.
 
Based on software on arduino.cc by Tom Igoe placed in the public domain

Version 1.0  Initial version for Adafruit Pro Trinket by Mike Barela
 
 */
#include <ProTrinketGamepad.h>  // include gamepad library for Pro Trinket (3V or 5V)

const int axisWheel = 2;
const int axisPedals = 3;
const int axisHatX = 6;
const int axisHatY = 7;

const int responseDelay = 5;        // response delay of the gamepad, in ms

uint8_t btns[] = { 8, 6, 5, 4, 9, 10, 11, 12, 3, 1, 0 };

void setup() {

  for(char i=0; i < sizeof(btns); i++)
  {
    pinMode(btns[i], INPUT_PULLUP);
  }

  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

  TrinketGamepad.begin();                 // initialize the gamepad library
}

uint16_t buttonMap;

void loop() {
  // read and scale the joystick readings
  int xReading = readAxis(axisWheel, 0);
  int yReading = readAxis(axisPedals, 90);
  int rxReading = readAxis(axisHatX, 0);
  int ryReading = readAxis(axisHatY, 0);

  for(char i=0; i < sizeof(btns); i++)
  {
    char b = digitalRead(btns[i]);  // read the gamepad left button (push joystick)

    bitWrite(buttonMap, i, !b);
  }

  TrinketGamepad.move(xReading, yReading, rxReading, ryReading, buttonMap);

  delay(responseDelay);  // wait between gamepad readings
}

// Reads a joystick axis
// analog input range to a range from 0 to <range>
int readAxis(int thisAxis, int offset) { 
  int reading = analogRead(thisAxis);  // read the analog input

  reading += offset;
  // map the reading from the analog input range to the output range
  reading = map(reading, 0, 1023, -32768, 32767);

  return reading;
}

