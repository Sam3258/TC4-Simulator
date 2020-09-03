/*
 * TC4 Simulator for Artisan Coffee Roaster Application
 *
 * Released under MIT License
 *
 * Created by Sam Chen on 2020
 * Copyright (c) 2020 sam4sharing.com, All rights reserved.
 * 
 * Blog     : https://www.sam4sharing.com
 * YouTube	: https://www.youtube.com/channel/UCN7IzeZdi7kl1Egq_a9Tb4g
 */

/*
 * Include the standard HeliOS header for Arduino sketches. This header
 * includes the required HeliOS header files automatically.
 */

#include <HeliOS_Arduino.h>
#include "UnoTC4.h"

#define TC4_GLOBALS

extern void taskCmdHandle(int);
extern void taskThermo(int);

// Create objects for Beam Temperature (BT) and Environment Temperature (ET)
MAX6675 BT_Thermocouple(CS_BT_PIN);

// Variables for Environment Temperature (ET) if exist
#if ENVIRONMENT_TEMPERATURE_EXIST
MAX6675 ET_Thermocouple(CS_ET_PIN);
#endif

#if SEGMENT_DISPLAY_EXIST
// Create object for a seven segment controller
SevSeg sevseg; 
#endif

// Create Software Serial port for debugging purpose
SoftwareSerial SerialDebug(SOFT_RX, SOFT_TX);       // RX, TX : D7, D8

void setup() {

  // Declare and initialize an int to hold the task id
  int id = 0;

#if SEGMENT_DISPLAY_EXIST
  byte numDigits = 3;   
  byte digitPins[] = {6, 5, 4}; 
  byte segmentPins[] = {14, 15, 16, 17, 18, 19, 2, 3}; 

  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(100);
#endif

  // Serial ports initialize
  Serial.begin(115200);
  SerialDebug.begin(115200);
  // Try to print something on serial port
//Serial.println("On board UART2USB port is used to communicate with Artisan App");
  SerialDebug.println(" ");
  SerialDebug.println("TC4 Simulator with UNO, MAX6675 v0.02");

  // Call xHeliOSSetup() to initialize HeliOS and its data structures
  xHeliOSSetup();

  // Add the task taskCmdHander() to HeliOS taskList
  id = xTaskAdd("TASKCMDHANDLE", &taskCmdHandle);

  // Pass the task id of the task to set its state from stopped to running
  xTaskStart(id);

  // Add the task taskThermo() to HeliOS taskList
  id = xTaskAdd("TASKTHERMO", &taskThermo);
  
  // Call xTaskWait() to place taskThermo() into a wait state 
  xTaskWait(id);

  // Set the timer interval for taskThermo() to 750 microseconds
  xTaskSetTimer(id, MAX6675_READING_INTERVEL);
}

void loop() {

  /*
   * Momentarily pass control to HeliOS by calling the
   * xHeliOSLoop() function call. xHeliOSLoop() should be
   * the only code inside of the sketch's loop() function.
   */
  xHeliOSLoop();
}

