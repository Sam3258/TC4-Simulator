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
#include "TC4.h"
#include <WiFi.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Define two tasks 
extern void TaskIndicator( void *pvParameters );
extern void TaskThermalMeter( void *pvParameters );

extern float    BT_AvgTemp;
bool 		    unit_C = true;          // indicate temperature unit from Artisan requested 

BluetoothSerial BTSerial;

void Bluetooth_Callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

	switch (event) {
        case ESP_SPP_INIT_EVT:
			Serial.println("SPP is inited");
			break;
        case ESP_SPP_START_EVT:
            Serial.println("SPP server started");
			break;
		case ESP_SPP_SRV_OPEN_EVT:
			Serial.println("Client Connected");
			break;
		case ESP_SPP_CLOSE_EVT:
			Serial.println("Client disconnected");
			break;
        case ESP_SPP_DATA_IND_EVT:
            Serial.println("SPP connection received data");
			break;       
		default:
			Serial.print("Unhandle Event: ");
			Serial.println(event);
			break;
	}
}

void setup() {
  
    // This project does not use WiFi, So turn it off.
    WiFi.mode(WIFI_OFF);

    // Initialize serial communication at 115200 bits per second:
    Serial.begin(115200);
    while (!Serial) {
        ; // wait for serial port ready
    }
    Serial.printf("\nTC4 Simulator v0.02 - Using ESP32 and MAX6675 !\n");
  
    // Initial Bluetooth Serial Port Profile (SPP)
    BTSerial.register_callback(Bluetooth_Callback);
	// Setup bluetooth device name as
	if (!BTSerial.begin("TC4 Simulator")) {
		Serial.println("An error occurred during initialize");
	}
	else {
	    Serial.println("ESP32BTSerial is ready for pairing");
        // Use FIXED pin-code for Legacy Pairing
		char pinCode[5];
		memset(pinCode, 0, sizeof(pinCode));
		pinCode[0] = '1';
  		pinCode[1] = '2';
  		pinCode[2] = '3';
 		pinCode[3] = '4';
		BTSerial.setPin(pinCode); 
	}

    /*---------- Task Definition ---------------------*/
    // Setup tasks to run independently.
    xTaskCreatePinnedToCore (
        TaskIndicator
    ,   "IndicatorTask" // 4 digitals 7-Segment LED task to indicate thermal temperature
    ,   2048            // This stack size can be checked & adjusted by reading the Stack Highwater
    ,   NULL
    ,   1               // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,   NULL 
    ,   tskNO_AFFINITY  // Running Core decided by FreeRTOS
    );

  xTaskCreatePinnedToCore (
        TaskThermalMeter
    ,   "ThermalMeter"  // MAX6675 thermal task to read Bean-Temperature (BT)
    ,   1024            // Stack size
    ,   NULL
    ,   2               // Priority
    ,   NULL 
    ,   tskNO_AFFINITY  // Running Core decided by FreeRTOS
    );
}

void loop()
{
    // This is main task which is created by Arduino to handle Artisan TC4 Commands

    if (BTSerial.available())
    {
        String msg = BTSerial.readStringUntil('\n');

#if PRINT_ARTISAN_WHOLE_MESSAGE
		Serial.println(msg);                 // print whole message
#endif
/*
	The READ command requests current temperature readings on all active channels. 
	Response from TC4 is ambient temperature followed by a comma separated list of temperatures in current active channels.
	The logical channel order is : ambient,chan1,chan2,chan3,chan4
*/	
 /* READ command */ 
		if (msg.indexOf("READ")==0) {	                // READ command
			BTSerial.print("0.00,");			        // ambient temperature
#if ENVIRONMENT_TEMPERATURE_EXIST    
            if (unit_C)  
		        BTSerial.print(ET_CurTemp);			        // channel 1 : Environment Temperature (ET) with degree Celsius
            else 
      	        BTSerial.print(ET_CurTemp * 9.0/5.0 + 32);	// channel 1 : Environment Temperature (ET) with degree Farenheit
#else
            if (unit_C)
			    BTSerial.print("0.00");			            // channel 1 : Environment Temperature (ET); no ET sensor, so uses BT instead	
            else
                BTSerial.print("0.00");	                    // channel 1 : Environment Temperature (ET); no ET sensor, so uses BT instead	
#endif      				 
    	    BTSerial.print(",");		
            if (unit_C)		
			    BTSerial.print(BT_AvgTemp);	                // channel 2 : Bean Temperature (BT) with degree Celsius
            else 
                BTSerial.print(BT_AvgTemp * 9.0/5.0 + 32);  // channel 2 : Bean Temperature (BT) with degree Farenheit 
    	    BTSerial.println(",0.00,0.00");	                // channel 3,4 : A vaule of zero indicates the channel is inactive
        }
 /* UNIT command */ 
        else if (msg.indexOf("UNITS;")== 0) {	 
            if (msg.substring(6,7)=="F") {   
			    unit_C = false;
                BTSerial.println("#OK Farenheit");
				Serial.println("Artisan \"Farenheit\"");
            }
            else if (msg.substring(6,7)=="C") {  
                unit_C = true;
                BTSerial.println("#OK Celsius");
			    Serial.println("Artisan \"Celsius\"");
            }
        }
/* CHAN command */        
        else if (msg.indexOf("CHAN;")== 0) {    
            BTSerial.print("#OK");
	        Serial.println("Artisan \"CHAN\"");
        }
/* FILT command */        
        else if (msg.indexOf("FILT;")== 0) {   
            BTSerial.print("#OK");
			Serial.println("Artisan \"FILT\"");
		}
/* Unhandle command */           
        else {
		    Serial.println("Artisan Unhandle command");
			Serial.println(msg);
		}
   }
   //vTaskDelay( 250 );
}