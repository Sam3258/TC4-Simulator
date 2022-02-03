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
#include "U8g2lib.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

extern float    BT_AvgTemp;
char 			printBuf[64];			// a string buffer for OLED 

// Create object for SSD1306
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   

#define INDICATOR_INTERVEL      1000    // Task re-entry intervel (ms)

void TaskIndicator(void *pvParameters)  
{
    /***
    Artisan Command Handler
    The definition of Artisan Command Handler is to
    (1) response Artisan TC4 commands which from native UART port
        or Bluetooth Serial Port Profile (SPP)  
    (2) Displays Beam Temperature (BT) on 4 * 7-segment LEDs if installed
    ***/
    
    /* Variable Definition */
    (void) pvParameters;
    TickType_t xLastWakeTime;
    const TickType_t xIntervel = INDICATOR_INTERVEL / portTICK_PERIOD_MS;
    
	// Start SSD1315 OLED Display
	u8g2.begin();
    // Show-up message on display
	u8g2.clearBuffer();                   		    // clear display the internal memory
	u8g2.setFont(u8g2_font_ncenB08_tr);   		    // choose font
    u8g2.drawStr(0, 16, "   Thermal Monitor v2");   // write string to buffer
    u8g2.sendBuffer();                    		    // transfer buffer string to display internal memory to show-up

    // Initial the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;) // A Task shall never return or exit.
    {
        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xIntervel);

	    memset(printBuf, 0, sizeof(printBuf));
   		snprintf(printBuf, sizeof(printBuf)-1, "%3.1f", BT_AvgTemp);

	    u8g2.clearBuffer();                   		    // clear display the internal memory
	    u8g2.setFont(u8g2_font_ncenB08_tr);   		    // choose font
        u8g2.drawStr(0, 16, "   Thermal Monitor v2");   // write string to buffer
	    u8g2.setFont(u8g2_font_ncenB24_tr);   		    // choose font
        if (BT_AvgTemp>=100)
            u8g2.drawStr(24, 56, printBuf);                 // write string to buffer
        else
            u8g2.drawStr(32, 56, printBuf);                 // write string to buffer
        u8g2.sendBuffer();                    		    // transfer buffer string to display internal memory to show-up
    }
}