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

#include <Arduino.h>
#include "UnoTC4.h"

float averageTemperature ( float *);

/*
 * The task definition for taskCmdHander() which handles 
 * Artisan TC4 commands from native UART port and  
 * displays BT temperature on 7-segment LEDs if installed
 */
void taskCmdHandle(int id_) {

    if ( Serial.available() ) {
		String msg = Serial.readStringUntil('\n');

#if PRINT_ARTISAN_WHOLE_MESSAGE
		SerialDebug.println(msg);                 			// print whole message
#endif

		if (msg.indexOf("READ")==0) {	            		// READ command
/*
	The READ command requests current temperature readings on all active channels. 
	Response from TC4 device is ambient temperature followed by a comma separated list of temperatures in current active units
	The logical channel order is : ambient,chan1,chan2,chan3,chan4
*/	
			Serial.print("0.00,");			      			// ambient temperature
#ifdef ENVIRONMENT_TEMPERATURE_EXIST    
      		if (unit_C)  
				Serial.print(ET_CurTemp);	      			// channel 1 : Environment Temperature (ET) with degree Celsius
      		else 
      			Serial.print(ET_CurTemp * 9.0/5.0 + 32);	// channel 1 : Environment Temperature (ET) with degree Farenheit
#else
      		if (unit_C)
			 	Serial.print(BT_AvgTemp);			        // channel 1 : Environment Temperature (ET); no ET sensor, so uses BT instead	
      		else
        		Serial.print(BT_AvgTemp * 9.0/5.0 + 32);    // channel 1 : Environment Temperature (ET); no ET sensor, so uses BT instead	
#endif      				 
    		Serial.print(",");		
      		if (unit_C)		
			  	Serial.print(BT_AvgTemp);	            	// channel 2 : Bean Temperature (BT) with degree Celsius
      		else 
        		Serial.print(BT_AvgTemp * 9.0/5.0 + 32);	// channel 2 : Bean Temperature (BT) with degree Farenheit 
    		Serial.println(",0.00,0.00");	          		// channel 3,4 : A vaule of zero indicates the channel is inactive
			
// The READ command be sent from Artisan every 3 seconds (set by sample rate), unmark below code carefully
//			SerialDebug.println("Artisan \"READ\"");						
    	} else if (msg.indexOf("UNITS;")== 0) {	  			// UNIT command 
      		if (msg.substring(6,7)=="F") {   
			  	unit_C = false;
        		Serial.println("#OK Farenheit");
				SerialDebug.println("Artisan \"Farenheit\"");
      		}
      		else if (msg.substring(6,7)=="C") {  
        		unit_C = true;
        		Serial.println("#OK Celsius");
			  	SerialDebug.println("Artisan \"Celsius\"");
      		}
    	} else if (msg.indexOf("CHAN;")== 0) {    			// CHAN command
      		Serial.print("#OK");
	    	SerialDebug.println("Artisan \"CHAN\"");
    	} else if (msg.indexOf("FILT;")== 0) {    			// FILT command
      		Serial.print("#OK");
			SerialDebug.println("Artisan \"FILT\"");
		} else {
		  	SerialDebug.println("Artisan Unhandle command");
			SerialDebug.println(msg);
		}
  	}

#if SEGMENT_DISPLAY_EXIST
    // Must run repeatedly; don't use blocking code (ex: delay()) in the loop() function or this won't work right
    sevseg.refreshDisplay(); 
#endif  
}

/*
 * The task definition for taskThermo() which will be 
 * executed by HeliOS every MAX6675_READING_INTERVEL (750ms) to read MAX6675 thermocouple
 */
void taskThermo(int id_) {

    // read BT from MAX6675 thermal couple
    BT_CurTemp = BT_Thermocouple.readTempC();

	if ( isReady ) {
    	// means, first round of temperature array is done, can do averaging and filter out operation  
	    BT_AvgTemp = averageTemperature( &BT_TempArray[0] );

        // filter out abnormal temperature-up only, bypass temperature-down
	    // because "CHARGE" period, the temperature-down may large than 10 degree
	    if ( BT_CurTemp < (BT_AvgTemp + ABNORMAL_TEMPERATURE_DEGREE) ) {
	    	// temperature is in-arrange, store it
	    	BT_TempArray[BT_ArrayIndex] = BT_CurTemp;
	     }
	     else {
	     	// set abnormal flag
	    	abnormalValue = true;
	    	// print ? with temperature value in newline
            SerialDebug.println(" ");
            SerialDebug.print(" ?");
            SerialDebug.println(BT_CurTemp);
	   	}
	}
	else {
	   	// just read current temperature
	   	BT_TempArray[BT_ArrayIndex] = BT_CurTemp;
	}

    if ( !abnormalValue ) {
    	// Normal temperature will into this loop
#if PRINT_TEAMPERATURE_EACH_READING
        // print MAX6675 reading value on serial monitor
	   	if ( BT_ArrayIndex == 0 ) {
			SerialDebug.println(" ");
	   		SerialDebug.print("Temperature: ");
	  	}

        SerialDebug.print(" ");			
	   	SerialDebug.print(BT_CurTemp);
#endif
        BT_ArrayIndex++;
        if ( BT_ArrayIndex >= TEMPERATURE_ARRAY_LENGTH ) {
        	BT_ArrayIndex = 0;

            if ( !isReady ) {
                isReady = true;
            }
#if PRINT_TEAMPERATURE_EACH_READING          
            SerialDebug.print(" ");	
            SerialDebug.print("Average: ");
            SerialDebug.print(BT_AvgTemp);
#endif     
#if ENVIRONMENT_TEMPERATURE_EXIST
            // The ET is reference temperature, don't need averaging
            // just read ET from MAX6675 thermal couple every 3 seconds
            ET_CurTemp = ET_Thermocouple.readTempC();
#if PRINT_TEAMPERATURE_EACH_READING           
            SerialDebug.print(" ");	
            SerialDebug.print("ET: ");
            SerialDebug.print(ET_CurTemp);
#endif   
#endif
        } 
#if SEGMENT_DISPLAY_EXIST
        sevseg.setNumber(BT_AvgTemp);   
#endif             
    }
    else {
        // After bypass abnormal value, reset flag here
	  	  abnormalValue = false;
    }
}

// A function to average temperature array which values are from MAX6675 temperature reading
float averageTemperature ( float *pTemp ) {

	float avg = *pTemp;

#if 0 
    // Below code works on ESP32 and ESP8266 but get OVF error on UNO,
    // I think this is because UNO is 8-bit microcontroller
    pTemp++;
    for (int i=0; i<=(TEMPERATURE_ARRAY_LENGTH-1); i++) {
		avg = avg + *pTemp;
      	avg =  avg / 2;
      	pTemp++;
}
#else
    pTemp++;
	avg = avg + *pTemp;
    avg =  avg / 2;
    pTemp++;
	avg = avg + *pTemp;
    avg =  avg / 2;
    pTemp++;
	avg = avg + *pTemp;
    avg =  avg / 2;
#endif  

	return avg;
}
