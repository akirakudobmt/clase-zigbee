/************************************************************************************
* This is the source file for the temperature sensor LM61B from the SRB board.
*
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
******************************************************************************/
#include "IoConfig.h"
#include "LM61B.h"  
#include "AppToPlatformConfig.h"
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

/* 
   ATD functionality - on, 
   result register data is right justified,
   10-bit resolution selected,
   result register data is unsigned.
*/
#define ATDC_CFG 0xC0

/* 
   disable interrupt, 
   single conversion mode, 
   channel number 6
*/
#define ATDSC_CFG 0x06

/* 
   mask for Conversion Complete Flag
*/
#define MASK_CCF 0x80

/* 
   the exception value for the read temperature when the target is not SRB
*/
#define EXCEPTIONVALUE 125

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
static uint16_t ReadRawTemperature( void );

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/*None*/

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************/
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
#if gTargetMC13213SRB_d
void ADC_Init(void)
{
    /* Initialize the ADC port */
	ATDPE = TempSensorAdcPort_c;        /* enable desired ADC channel */
	ATDC  = ATDC_PRESC | ATDC_CFG_c;		/* set factor to 16 */                           
}
#else
void ADC_Init(void)
{
}
#endif /* gTargetMC13213SRB_d */

#if gTargetMC13213SRB_d
static uint16_t ReadRawTemperature( void )
{
   /* configure and start converting */
   ATDSC = ATDSC_CFG_c;	
	/* wait until conversion is finished */
	while((ATDSC & Mask_CCF_c) == 0){}
	/* read the conversion result */
	return( (uint16_t)(ATDRL) | (uint16_t)(((uint16_t)((ATDRH)&0x03))<<8));
}
#endif
/******************************************************************************
* This function computes the temperature read from the LM61B sensor.
*
* Interface assumptions:
*
*
* Return value: [-25..85] Celsius Degrees or 125 when the defult target is different of MC13213 SRB
* None
*
******************************************************************************/
#if gTargetMC13213SRB_d
int8_t ReadTemperature( void )
{

uint16_t milliVolts;
uint16_t AdcValue;

AdcValue = ReadRawTemperature();

/* 1LSB = (VREFH - VREFL) / 2^10 */
/* 1LSB = 3.2mV/bit */

  milliVolts = (AdcValue) * 3;
/* Convert milliVolts to Celsius */
#if TemperatureCompensation_c == 0
 return((int8_t)((milliVolts - TempSensormilliVoltOffset_c)/milliVoltPerDegree_c));
#else
return((int8_t)((milliVolts - TempSensormilliVoltOffset_c)/milliVoltPerDegree_c) + TemperatureCompensation_c);
#endif  
}
#else
int8_t ReadTemperature( void )
{
   return Exceptionvalue_c;
}

#endif /* gTargetMC13213SRB_d */
