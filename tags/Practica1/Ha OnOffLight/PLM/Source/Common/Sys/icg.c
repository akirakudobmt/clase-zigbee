/************************************************************************************
* This module contains code for the ICG (Internal Clock Generator) module.
* 
* Note! Support for selecting external crystal and external clock source from Abel 2.0
*
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#include "IrqControlLib.h"
#include "EmbeddedTypes.h"
#include "PowerManager.h"
#include "ICG.h"



// **************************************************************************

#pragma MESSAGE DISABLE C4200  // Warning C4200: Other segment than in previous declaration
#pragma DATA_SEG BOOTLOADER_VARIABLES

	// Global variables
	volatile bool_t init_mode = TRUE;

#pragma DATA_SEG DEFAULT

// **************************************************************************

#pragma DATA_SEG SLEEP_VARIABLE
	uint8_t gPowerSaveMode;
#pragma DATA_SEG DEFAULT

#ifdef PROCESSOR_HCS08
void ICG_Setup(void)
{
uint8_t loop_counter;

	SETUP_LED_PORT
	
	SET_LED1 // Show entry in function

	SET_LED2 // Cleared first time in loop
	CLEAR_LED3
	CLEAR_LED4

	if(gPowerSaveMode == 0)
	{
		// Not in power save mode

		// Loop until clock is locked
		for(;;)
		{
			TOGGLE_LED2

			if((ICGS1 & ICG_IRQ_PENDING) == ICG_IRQ_PENDING)
				ICGS1 |= ICG_IRQ_PENDING; // Clear FLL lost lock interrupt

			if(init_mode == FALSE)
			{
				TOGGLE_LED3
				
				// Is filter value drifting?
				if(ICGFLTH > ICGFLTH_VALUE_8MHZ)
				{
					TOGGLE_LED4
					
					SCM_8MHZ_DEFAULT_ICG_FILTER
					WAIT_FOR_STABLE_CLOCK
				}

				SELECT_SCM_MODE // DO NOT CALL THIS CODE UNDER INIT

				WAIT_FOR_STABLE_CLOCK
			}
			else
				init_mode = FALSE; // Only one time under init

			SETUP_ABEL_CLOCK

			WAIT_FOR_STABLE_CLOCK

			SETUP_ICG_MODULE

			WAIT_FOR_STABLE_CLOCK

			loop_counter = LOOP_COUNTER_VALUE;

			// Wait for clock to lock
      while(!ICG_IsFllLocked() && loop_counter-- > 0);

			// Check exit condition
			if(ICG_IsFllLocked())
				break; // Clock is locked - get out
		}
	}
	else
	{
		// In power save mode - set SCM clock.
		SCM_2MHZ_ICG_FILTER
	}
	
	MEASURING_BUS_CLOCK
	
	CLEAR_LED1 // Show exit of function
}
#endif /* PROCESSOR_HCS08 */

// **************************************************************************

#ifdef PROCESSOR_QE128
void ICG_SetupQE128(void) 
{
  uint16_t reg = 0;
  /* RST de-asserted. Wait for transceiver to become active (idle mode) */
  do 
  {
   /* Write Abel/Toro clock */
   MC1319xDrv_WriteSpiAsync(ABEL_regA, gHardwareParameters.Abel_Clock_Out_Setting);
   
   WAIT_FOR_STABLE_CLOCK
   
   /* Read Back Abel/Toro clock */
   MC1319xDrv_ReadSpiAsync(ABEL_regA, &reg);
  }while(reg != gHardwareParameters.Abel_Clock_Out_Setting);
  
  /* SPI Active. Abel/Toro clock set */
       
  ICSC2 = gHardwareParameters.NV_ICSC2 & 0xF7; /* Do not write the LP bit at this point */    
  ICSC1 = gHardwareParameters.NV_ICSC1; /* program the requested ICSC1 values */  
  ICSTRM = gHardwareParameters.NV_ICSTRM;
    
  ICSSC = gHardwareParameters.NV_ICSSC;
    
  /* wait for clock to settle */
  while(!ICG_IsFllLocked());
  
  if(gHardwareParameters.NV_ICSC2 & 0x08) 
  {
   ICSC2 |= 0x08; /* write the LP bit */
  }
  
}
#endif /* PROCESSOR_QE128 */
// **************************************************************************


void PowerManager_WaitForPowerUp(void)
{
 #if defined(PROCESSOR_QE128)
  ICG_SetupQE128();
 #elif defined(PROCESSOR_MC1323X)
 LVDTRIM = 0x19; // trim value seen on the scope
 /* SOMC1 register should be set by application */
 //SOMC1 = gHardwareParameters.SOMC1_value;
 SOMC2 = gHardwareParameters.Xtal_Trim;
 SCGC1 = gHardwareParameters.SCGC1_value;
 SCGC2 = gHardwareParameters.SCGC2_value;
 #else
	ICG_Setup(); // Reuse code in function to reduce code size
 #endif /* PROCESSOR_QE128 */
}

  
// **************************************************************************

#ifdef PROCESSOR_HCS08
INTERRUPT_KEYWORD void FLL_Lost_Lock_ISR(void)
{
	// Setup ICG module again to prevent that system hangs forever.
	
	ICGS1 |= 0x01; // Clear FLL lost lock interrupt

    // Ensure MCU clk is stable
  PowerManager_WaitForPowerUp();
}
#endif /* PROCESSOR_HCS08 */