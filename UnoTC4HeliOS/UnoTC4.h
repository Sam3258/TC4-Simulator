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

#ifndef __UNOTC4_H__
#define __UNOTC4_H__

#include <SoftwareSerial.h>
#include "MAX6675.h"
#include "SevSeg.h"

#ifdef  TC4_GLOBALS
#define TC4_EXT
#else
#define TC4_EXT  extern
#endif

#define PRINT_TEAMPERATURE_EACH_READING         1	    // Set, to print temperature vaule on serial debug port
#define PRINT_ARTISAN_WHOLE_MESSAGE		        0       // set, to print Artisan commands on serial debug port
#define ENVIRONMENT_TEMPERATURE_EXIST           1       // Set, if you installed ET thermal couple 
#define SEGMENT_DISPLAY_EXIST                   1       // Set, if you installed 7-Segment Display

#define SCK_PIN       13      // D13, PB5
#define SO_PIN 	      12      // D12, PB4
#define CS_BT_PIN 	  10      // D10, PB2
#define CS_ET_PIN     9       // D9,  PB1
#define SOFT_TX       8       // D8,  PB0
#define SOFT_RX       7       // D7,  PD7

/* 
the default sample rate of Artisan is 3 seconds, although the setting value can be modified by user.
I think this value is generated from lots of experimental, so uses 3 seconds as our program algorithm
thus, 1) Set polling intervel 750ms and 2) Array length is 4. that is, the reported temperature is
avaraged from 4 times MAX6675 temperature reading. (750ms x 4 = 3s)
*/
#define TEMPERATURE_ARRAY_LENGTH		        4	    // for averagging temperature purpose
#define MAX6675_READING_INTERVEL 			    750000  // read MAX6675 value every "INTERVEL" ms
TC4_EXT float       BT_TempArray[TEMPERATURE_ARRAY_LENGTH] = {0.0};	    // temperature array
TC4_EXT int			BT_ArrayIndex = 0;                  // A pointer of temperature array
TC4_EXT float		BT_CurTemp = 0.0;
TC4_EXT float		BT_AvgTemp = 0.0;
TC4_EXT bool		isReady = false;                    // flag for temperature array ready for reading
TC4_EXT bool 		unit_C = true;                      // indicate temperature unit from Artisan requested 

#define ABNORMAL_TEMPERATURE_DEGREE             10      // A arrange for abnormal temperature value
TC4_EXT bool		abnormalValue = false;              // indicate temperature value is unexpect or not

// Variables for Environment Temperature (ET) if exist
#if ENVIRONMENT_TEMPERATURE_EXIST
TC4_EXT float		ET_CurTemp = 0.0;
#endif

#endif /*__UNOTC4_H__*/
