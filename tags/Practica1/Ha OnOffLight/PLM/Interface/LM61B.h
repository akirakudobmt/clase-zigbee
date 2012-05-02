/*****************************************************************************
* This is the header file for the temperature sensor LM61B from the SRB board.
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/


#ifndef _LM61B_H_
#define _LM61B_H_

#include "EmbeddedTypes.h"
#include "IoConfig.h"


/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* The ADC prescaler is set in function of the maximum bus frequency(16MHz) */
#define gAdcPresc2Mhz_d 0x03
#define gAdcPresc1Mhz_d 0x07
#define gAdcPresc500Khz_d 0x0F

/* select the ADC prescaler */
#define ATDC_PRESC gAdcPresc1Mhz_d

/* 
   ATD functionality - on, 
   result register data is right justified,
   10-bit resolution selected,
   result register data is unsigned.
*/
#define ATDC_CFG_c 0xC0

/* 
   disable interrupt, 
   single conversion mode, 
   channel number 6
*/
#define ATDSC_CFG_c 0x06

/* 
   mask for Conversion Complete Flag
*/
#define Mask_CCF_c 0x80

/*Temp Sensor ADC port - AD 6*/
#define TempSensorAdcPort_c 0x40

/* 
   the exception value for the read temperature when the target is not SRB
*/
#define Exceptionvalue_c 125

/*
 no.of degrees in Celsius to add/substract to compenstate for self heating etc.
 note type is int8_t!
*/
#define TemperatureCompensation_c -5 

/*LM61B ZERO degrees celsius offset in milliVolt */
#define TempSensormilliVoltOffset_c 600

/* LM61B milliVolt per 1 degree celius */
#define milliVoltPerDegree_c 10


/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/
/******************************************************************************
* This function initialize the ADC
*
* Interface assumptions:
*
*
* Return value:
* None
*
******************************************************************************/
void ADC_Init( void );
/******************************************************************************
* This function computes the temperature read from the LM61B sensor.
*
* Interface assumptions:
*
*
* Return value: [-25..85] Celsius Degrees
* None
*
******************************************************************************/
int8_t ReadTemperature( void );

#endif /*_LM61B_H_ */
