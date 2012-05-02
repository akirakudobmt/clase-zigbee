/************************************************************************************
* This module contains the ICG module defines etc.
*
* Note! Support for selecting external crystal and external clock source from Abel 2.0
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _ICG_H
#define _ICG_H

#include "EmbeddedTypes.h"

#ifndef PROCESSOR_MC1323X
#include "Mc1319xDrv.h"
#include "Mc1319xReg.h"
#endif /* PROCESSOR_MC1323X */

#include "IoConfig.h"
#include "NV_Data.h"

// **************************************************************************
// debug defines

//#define ENABLE_LEDS // Enables status on LEDS
//#define ENABLE_DEBUG_MEASURING // Enables bus frequency measuring loop after setup

// **************************************************************************
// User configuration defines

// Select one type of external clock source.
//#define USE_EXT_CLK // Use external crystal
#define USE_ABEL_CLKO // Use Abel CLKO output - select Abel CLKO frequency below

// Uses the values from NV RAM else  use default values
#define USE_VALUES_FROM_NV_RAM


#ifdef USE_ABEL_CLKO
// If Abel version 2.0/2.1 CLKO clock is selected - select one Abel CLKO output frequency.
// Note! All values will give a CPU frequency = 16 MHz and a bus frequency = 8 MHz
// However, 32,78 KHz and 16,39 KHz will give 8,39 MHz as bus frequency. This has impact on
// the UART baud rate and SPI clock.

	//#define USE_16MHZ_CLKO // DO NOT SELECT THIS - WILL NOT WORK (MAX. 10 MHz)
	//#define USE_8MHZ_CLKO
	//#define USE_4MHZ_CLKO
	//#define USE_2MHZ_CLKO
	//#define USE_1MHZ_CLKO // REQUIRES ADDITIONAL NOPS
	#define USE_62_5KHZ_CLKO
	//#define USE_32_78KHZ_CLKO
	//#define USE_16_39KHZ_CLKO // REQUIRES ADDITIONAL NOPS

// If Abel version 2.0/2.1 CLKO clock is selected - select one Abel CLKO output frequency.
// Note! All values will give a CPU frequency = 32 MHz and a bus frequency = 16 MHz
// However, 32,78 KHz and 16,39 KHz will give 16,78 MHz as bus frequency. This has impact on
// the UART baud rate and SPI clock.
    //#define USE_32_78KHZ_CLKO_AT_16_78HMZ
    //#define USE_32_78KHZ_CLKO_AT_14_24HMZ

#endif USE_ABEL_CLKO

// **************************************************************************
// Abel CLKO defines

// Abel CLKO register valuedefines
#define ABEL_16MHZ		0x0000
#define ABEL_8MHZ		0x0001
#define ABEL_4MHZ		0x0002
#define ABEL_2MHZ		0x0003
#define ABEL_1MHZ		0x0004
#define ABEL_62_50KHZ	0x0005
#define ABEL_32_78KHZ	0x0006
#define ABEL_16_39KHZ	0x0007

// **************************************************************************

// Note! The selected clock is the CPU frequency. Bus clock is half of CPU.
// I.e. CPU = 16 MHz => 8 MHz bus clock

// Set ICG module to run @ 8 MHz as bus clock using Abel clock 16 MHz clock out
#define ABEL_CLOKO_16MHZ_TO_8MHZ_ICG ICGC2 = 0x02; \
									 ICGC1 = 0x58;

// Set ICG module to run @ 8 MHz as bus clock using Abel clock 8 MHz clock out
#define ABEL_CLOKO_8MHZ_TO_8MHZ_ICG	 ICGC2 = 0x01; \
									 ICGC1 = 0x58;

// Set ICG module to run @ 8 MHz as bus clock using Abel clock 4 MHz clock out
#define ABEL_CLOKO_4MHZ_TO_8MHZ_ICG	 ICGC2 = 0x00; \
									 ICGC1 = 0x58;

// Set ICG module to run @ 8 MHz as bus clock using Abel clock 2 MHz clock out
#define ABEL_CLOKO_2MHZ_TO_8MHZ_ICG	 ICGC2 = 0x20; \
									 ICGC1 = 0x58;

// Set ICG module to run @ 8 MHz as bus clock using Abel clock 1 MHz clock out
#define ABEL_CLOKO_1MHZ_TO_8MHZ_ICG	 ICGC2 = 0x60; \
									 ICGC1 = 0x58;

// Set ICG module to run @ 8 MHz as bus clock using Abel clock 62,5 KHz clock out
#define ABEL_CLOKO_62_5_TO_8MHZ_ICG  ICGC2 = 0x00; \
									 ICGC1 = 0x18;

// Note! Below running @ 8,39 MHz
// Set ICG module to run @ 8,39 MHz as bus clock using Abel clock 32,78 KHz clock out
#define ABEL_CLOKO_32_78_TO_8MHZ_ICG ICGC2 = 0x20; \
									 ICGC1 = 0x18;

// Set ICG module to run @ 8,39 MHz as bus clock using Abel clock 16,39 KHz clock out
#define ABEL_CLOKO_16_39_TO_8MHZ_ICG ICGC2 = 0x60; \
									 ICGC1 = 0x18;

// Set ICG module to run @ 8 MHz as bus clock using external crystal
#define EXT_CRYSTAL_4MHZ_TO_8MHZ_ICG ICGC2 = 0x00; \
									 ICGC1 = 0x78;


// Set ICG module to run @ 16,78 MHz as bus clock using Abel clock 32,78 KHz clock out
#define ABEL_CLOKO_32_78_TO_16_78MHZ_ICG ICGC2 = 0x60; \
									     ICGC1 = 0x18;

// Set ICG module to run @ 14,24 MHz as bus clock using Abel clock 32,78 KHz clock out
#define ABEL_CLOKO_32_78_TO_14_24MHZ_ICG ICGC2 = 0x50; \
									     ICGC1 = 0x18;


// **************************************************************************

// SCM filter values for ~8 MHz bus clock
#define ICGFLTH_VALUE_8MHZ			0x02
#define ICGFLTL_VALUE_8MHZ			0x40

// SCM filter values for ~2 MHz bus clock (lowest possible value)
#define ICGFLTH_VALUE_2MHZ			0x00
#define ICGFLTL_VALUE_2MHZ			0x11 //0x10

// **************************************************************************

// Set ICG module to run @ 8 MHz as SCM bus clock. Adjust filter
// Note! LSB first
#define SCM_8MHZ_DEFAULT_ICG_FILTER	ICGFLTL = ICGFLTL_VALUE_8MHZ; \
									ICGFLTH = ICGFLTH_VALUE_8MHZ;

// Set ICG module to run @ 2 MHz as SCM bus clock. Adjust filter
// Note! LSB first
#define SCM_2MHZ_ICG_FILTER		ICGFLTL = ICGFLTL_VALUE_2MHZ; \
								ICGFLTH = ICGFLTH_VALUE_2MHZ;

// **************************************************************************

#define SELECT_SCM_MODE				ICGC1 = 0x40;

// **************************************************************************

#define LOOP_COUNTER_VALUE			100

// **************************************************************************
// Configuration dependend defines

#ifdef USE_ABEL_CLKO

#ifdef USE_16MHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_16MHZ_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_16MHZ
#endif USE_16MHZ_CLKO

#ifdef USE_8MHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_8MHZ_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_8MHZ
#endif USE_8MHZ_CLKO

#ifdef USE_4MHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_4MHZ_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_4MHZ
#endif USE_4MHZ_CLKO

#ifdef USE_2MHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_2MHZ_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_2MHZ
#endif USE_2MHZ_CLKO

#ifdef USE_1MHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_1MHZ_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_1MHZ
#endif USE_1MHZ_CLKO

#ifdef USE_62_5KHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_62_5_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_62_50KHZ
#endif USE_62_5KHZ_CLKO

#ifdef USE_32_78KHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_32_78_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_32_78KHZ
#endif USE_32_78KHZ_CLKO

#ifdef USE_16_39KHZ_CLKO
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_16_39_TO_8MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_16_39KHZ
#endif USE_16_39KHZ_CLKO


#ifdef USE_32_78KHZ_CLKO_AT_16_78HMZ
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_32_78_TO_16_78MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_32_78KHZ
#endif USE_32_78KHZ_CLKO_AT_16_78HMZ

#ifdef USE_32_78KHZ_CLKO_AT_14_24HMZ
	#define DEFAULT_SETUP_ICG_MODULE	ABEL_CLOKO_32_78_TO_14_24MHZ_ICG
	#define SELECT_ABEL_CLOCK			ABEL_32_78KHZ
#endif USE_32_78KHZ_CLKO_AT_14_24_HMZ


#define DEFAULT_SETUP_ABEL_CLOCK		MC1319xDrv_WriteClockoCtrl(0x3640 | SELECT_ABEL_CLOCK);
#define NV_SETUP_ABEL_CLOCK				MC1319xDrv_WriteClockoCtrl(gHardwareParameters.Abel_Clock_Out_Setting);

#endif USE_ABEL_CLKO




// **************************************************************************

#define NV_SETUP_ICG_MODULE	ICGC2 = gHardwareParameters.NV_ICGC2; \
							ICGC1 = gHardwareParameters.NV_ICGC1;

// **************************************************************************

#ifdef USE_EXT_CLK

	#define SETUP_ICG_MODULE		EXT_CRYSTAL_4MHZ_TO_8MHZ_ICG
	#define SETUP_ABEL_CLOCK 		// Empty

#endif USE_EXT_CLK

// **************************************************************************
// Macros
#define SETUP_ICG_MODULE    NV_SETUP_ICG_MODULE

#define SETUP_ABEL_CLOCK    NV_SETUP_ABEL_CLOCK
// **************************************************************************

// Some NOPS
#define SOME_NOPS	__asm nop; \
			 		__asm nop; \
			 		__asm nop; \
			 		__asm nop; \
			 		__asm nop; \
			 		__asm nop;

// These special low CLKO requires additional settling nops
#if defined USE_16_39KHZ_CLKO | defined USE_1MHZ_CLKO | defined USE_16MHZ_CLKO
	// A few NOPS in loop for clock to settle.
	#define WAIT_FOR_STABLE_CLOCK	{                           \
								   	uint8_t i;                  \
										for(i = 50; i > 0; i--) \
											SOME_NOPS           \
									}
#else
	// A few NOPS for clock to settle.
	#define WAIT_FOR_STABLE_CLOCK	SOME_NOPS
#endif
// **************************************************************************

#ifdef ENABLE_DEBUG_MEASURING

// This macro can be used to measure the bus frequency.
// The loop and toggling of the GPIO takes 22318 cycles.
// Connect a scope on PTC6 trigger falling edge. MUST BE UNUSED IN SYSTEM
// Number of cycles 22318 divided with measured time on scope _|-|_ = bus clock in Hz
#define MEASURING_BUS_CLOCK     PTCPE = 0x00;                   \
								PTCSE = 0x00;                   \
								PTCDD = 0x40;                   \
																\
								PTCD &= ~0x40;					\
						  		    	  					    \
															    \
  						        PTCD |= 0x40;					\
															    \
						        {								\
						        uint16_t i, j;					\
															    \
							      for(i = 10; i > 0; i--)		\
								    for(j = 100; j > 0; j--);	\
						        } 								\
															    \
  					            PTCD &= ~0x40;

#else
#define MEASURING_BUS_CLOCK // Empty function

#endif ENABLE_DEBUG_MEASURING

// **************************************************************************

#ifdef ENABLE_LEDS

// Setup port F 0,1,2 and 3 as output LED1, LED2, LED3 and LED4
#define SETUP_LED_PORT 		PTFPE = 0x00; \
							PTFSE = 0x00; \
							PTFDD = 0x0F;


// LED's on Axiom development board are inverted.
// I.e. port = "1" => LED off and port = "0" => LED on
#define SET_LED1			PTFD &= ~0x01;
#define SET_LED2		 	PTFD &= ~0x02;
#define SET_LED3			PTFD &= ~0x04;
#define SET_LED4			PTFD &= ~0x08;

#define CLEAR_LED1			PTFD |= 0x01;
#define CLEAR_LED2			PTFD |= 0x02;
#define CLEAR_LED3			PTFD |= 0x04;
#define CLEAR_LED4			PTFD |= 0x08;


#define TOGGLE_LED1			if((PTFD & 0x01) == 0x01) \
								SET_LED1              \
							else                      \
								CLEAR_LED1

#define TOGGLE_LED2			if((PTFD & 0x02) == 0x02) \
								SET_LED2              \
							else                      \
								CLEAR_LED2

#define TOGGLE_LED3			if((PTFD & 0x04) == 0x04) \
								SET_LED3              \
							else                      \
								CLEAR_LED3

#define TOGGLE_LED4			if((PTFD & 0x08) == 0x08) \
								SET_LED4              \
							else                      \
								CLEAR_LED4

#else ENABLE_LEDS

// Use empty defines

#define SETUP_LED_PORT

// LED's on Axiom development board are inverted.
// I.e. port = "1" => LED off and port = "0" => LED on
#define SET_LED1
#define SET_LED2
#define SET_LED3
#define SET_LED4

#define CLEAR_LED1
#define CLEAR_LED2
#define CLEAR_LED3
#define CLEAR_LED4

#define TOGGLE_LED1
#define TOGGLE_LED2
#define TOGGLE_LED3
#define TOGGLE_LED4

#endif ENABLE_LEDS

// **************************************************************************

// Some ICG module bit positions
#define ICG_IRQ_PENDING	0x01
#define ICG_FLL_LOCKED	0x08

// **************************************************************************
// Macros and Prototypes
void ICG_Setup(void);

#if defined(PROCESSOR_QE128)
 #define ICG_IsFllLocked() (ICSSC == gHardwareParameters.NV_ICSSC)
#elif  defined(PROCESSOR_MC1323X)
 #define ICG_IsFllLocked() TRUE
#else
// defined(PROCESSOR_HCS08)
 #define ICG_IsFllLocked() (ICGS1 & ICG_FLL_LOCKED)
#endif 


// **************************************************************************

#endif _ICG_H
