/*****************************************************************************
* Timer module private declarations.
*
* (c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _TIMER_H
#define _TIMER_H

#include "TMR_Interface.h"
#include "EmbeddedTypes.h"
#include "IoConfig.h"

/*****************************************************************************
******************************************************************************
* Private type declarations
******************************************************************************
*****************************************************************************/

/* Time in clock ticks. */
typedef uint16_t tmrTimerTicks_t;

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

/* Hardware related declarations. */

/* Declare which TPM this code uses, and which channel in the TPM. */

#ifndef gTmrTpmNumber_d
#define gTmrTpmNumber_d 1
#endif

/* TPM registers. */

#if gTmrTpmNumber_d == 1
#define gTPMxSC_c   TPM1SC              /* Status and control register. */

#define gTPMxCNTH_c TPM1CNTH            /* Counter, high byte. */
#define gTPMxCNTL_c TPM1CNTL            /* Counter, low  byte. */

#define gTPMxMODH_c TPM1MODH            /* Modulo register, high byte. */
#define gTPMxMODL_c TPM1MODL            /* Modulo register, low byte. */

/* TPMx channel registers. */
#define gTPMxCnSC_c TPM1C0SC            /* Status and control register. */

#define gTPMxCnVH_c TPM1C0VH            /* Counter compare value, high byte. */
#define gTPMxCnVL_c TPM1C0VL            /* Counter compare value, low byte. */
#endif

#if gTmrTpmNumber_d == 2
#define gTPMxSC_c   TPM2SC              /* Status and control register. */

#define gTPMxCNTH_c TPM2CNTH            /* Counter, high byte. */
#define gTPMxCNTL_c TPM2CNTL            /* Counter, low  byte. */

#define gTPMxMODH_c TPM2MODH            /* Modulo register, high byte. */
#define gTPMxMODL_c TPM2MODL            /* Modulo register, low byte. */

/* TPMx channel registers. */
#define gTPMxCnSC_c TPM2C0SC            /* Status and control register. */

#define gTPMxCnVH_c TPM2C0VH            /* Counter compare value, high byte. */
#define gTPMxCnVL_c TPM2C0VL            /* Counter compare value, low byte. */
#endif

#if defined   (PROCESSOR_QE128)  
  #define gTmrSourceClkHz_c (31250>>1)         /* Define the source clock for the timer 
                                                  (hz) = ICSFFCLK/2 for QE128*/
  #define gTmrSourceClkForMilli_c (314>>1)     /* The source clk is 31.25Khz*/
                                               /* Set it (31.4Khz/2) for a more accurate millisec. timer */ 
#else         /* HCS08 & MC1323x */
  #define gTmrSourceClkHz_c 31250              /* Define the source clock for the timer (hz) HCS08 */
  #define gTmrSourceClkForMilli_c 313          /* The source clk is 31.25Khz*/
                                               /* Set it to be 31.3Khz for a more accurate millisec. timer */
#endif

#define gTPMxSC_Prescale1_c     0x00    /* For prescale Value of 1   */
#define gTPMxSC_Prescale2_c     0x01    /* For prescale Value of 2   */
#define gTPMxSC_Prescale4_c     0x02    /* For prescale Value of 4   */
#define gTPMxSC_Prescale8_c     0x03    /* For prescale Value of 8   */
#define gTPMxSC_Prescale16_c    0x04    /* For prescale Value of 16  */
#define gTPMxSC_Prescale32_c    0x05    /* For prescale Value of 32  */
#define gTPMxSC_Prescale64_c    0x06    /* For prescale Value of 64  */
#define gTPMxSC_Prescale128_c   0x07    /* For prescale Value of 128 */

/*
The resolution of the timer is between 
4ms(max. time a task to be executed) - 0xffff * (1tick in ms).  
*/
/* Changing the prescaler the resolution will increase or decrease */
#ifndef PROCESSOR_QE128
/* HCS08 prescaler configured to obtain 4ms/tick */
#define gTPMxSC_PrescaleCount_c gTPMxSC_Prescale128_c
#else
/* QE128 prescaler configured to obtain 2ms/tick (ensure that the compare register is loaded from the latch) */
#define gTPMxSC_PrescaleCount_c gTPMxSC_Prescale32_c
#endif /* PROCESSOR_QE128 */

/* Bits in the TPMx timer status and control registers (TPMxSC). */
#define gTPMxSC_TOF_c           0x80    /* Timer overflow flag. (RO) */
#define gTPMxSC_TOIE_c          0x40    /* Timer overflow interrupt enable. (RW) */
#define gTPMxSC_CPWMS_c         0x20    /* Center-aliged PWM select. (RW) */
#define gTPMxSC_CLKSB_c         0x10    /* Clock source, high bit. (RW) */
#define gTPMxSC_CLKSA_c         0x08    /* Clock source, low bit. (RW) */
#define gTPMxSC_PS2_c           0x04    /* Prescale divisor select, high bit. (RW) */
#define gTPMxSC_PS1_c           0x02    /* Prescale divisor select, middle bit. (RW) */
#define gTPMxSC_PS0_c           0x01    /* Prescale divisor select, low bit. (RW) */

#define gTPMxSC_Stop_c          0x00
#ifndef PROCESSOR_MC1323X
  #define gTPMxSC_ClockSource_c    gTPMxSC_CLKSB_c /* Select the fixed system clock. */
#else
  #define gTPMxSC_ClockSource_c    (gTPMxSC_CLKSA_c | gTPMxSC_CLKSB_c) /* For MC1323x select the external clock (32MHz/1024 supplied by the SIM module) */  
#endif

/* Bits in the TPMx channel n status and control registers (TPMxCnSC). */
/* Bits 0 and 1 are not defined. */
#define gTPMxCnSC_F_c           0x80    /* Overflow flag. */
#define gTPMxCnSC_IE_c          0x40    /* Interrupt enable. */
#define gTPMxCnSC_MSB_c         0x20    /* Mode select bit B. */
#define gTPMxCnSC_MSA_c         0x10    /* Mode select bit A. */
#define gTPMxCnSC_ELSB_c        0x04    /* Edge/level select high bit. */
#define gTPMxCnSC_ELSA_c        0x02    /* Edge/level select low bit. */

/* HCS08 clock ticks for one second  */
#define TmrTicksForOneSecond() ((tmrTimerTicks_t)((tmrTimerTicks_t)gTmrSourceClkHz_c/(0x01<<gTPMxSC_PrescaleCount_c)))

/* HCS08 clock ticks for one minute */
#define TmrTicksForOneMinute()  ((tmrTimerTicks_t)(60*TmrTicksForOneSecond()))

#define gPrescaleCountForMilli_c (uint32_t)(((uint32_t)(0x01<<gTPMxSC_PrescaleCount_c)*10))
/* Convert milliseconds to HCS08 clock ticks. */
#define TmrTicksFromMilliseconds(milliseconds)  (tmrTimerTicks_t)( (0 == gTPMxSC_PrescaleCount_c) \
        ? ((uint32_t)milliseconds * (uint32_t)gTmrSourceClkForMilli_c)/10 \
        :(((uint32_t)milliseconds * (uint32_t)gTmrSourceClkForMilli_c) / gPrescaleCountForMilli_c) )


/* Read compare registers. */
#define TpmReadCnVRegister(variable) \
        mTmrRead16bitReg.byteAccess[ 0 ] = gTPMxCnVH_c; \
        mTmrRead16bitReg.byteAccess[ 1 ] = gTPMxCnVL_c; \
       (variable) = mTmrRead16bitReg.wordAccess;

/* Read free running counter. */
#define TpmReadCNTRegister(variable) \
        mTmrRead16bitReg.byteAccess[ 0 ] = gTPMxCNTH_c; \
        mTmrRead16bitReg.byteAccess[ 1 ] = gTPMxCNTL_c; \
       (variable) = mTmrRead16bitReg.wordAccess;

/* Enable the software timer registers.*/
#define TpmStartTimerHardware() \
        (gTPMxSC_c |= (gTPMxSC_ClockSource_c | gTPMxSC_PrescaleCount_c))

#define TpmStopTimerHardware()  (gTPMxSC_c = gTPMxSC_Stop_c)

/* Load TPMxC0V registers */
#define TPMxCnVLvalue(ticks)    (ticks & 0xFF)
#define TPMxCnVHvalue(ticks)    (ticks >> 8)

/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/

void TMR_LoadNextTimeOutCompareValue(void);

#endif
