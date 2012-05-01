/************************************************************************************
* Function to handle/set the different power down states.
*
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
***********************************************************************************/
#ifndef _PWR_H_
#define _PWR_H_

/************************************************************************************
*************************************************************************************
* Includes and external declarations
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
#include "PWRLib.h"

extern void AbelRegisterSetup(void);

/************************************************************************************
*************************************************************************************
* Public Macros
*************************************************************************************
************************************************************************************/
#define gAllowDeviceToSleep_c 0

/************************************************************************************
*************************************************************************************
* Public types
*************************************************************************************
************************************************************************************/
//---------------------------------------------------------------------------//
/*--- Parameter type for PWR_CheckForAndEnterNewPowerState(..) call       ---*/
typedef enum {PWR_Run = 77, PWR_Sleep, PWR_DeepSleep, PWR_Reset, PWR_OFF} PWR_CheckForAndEnterNewPowerState_t;

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/******************************************************************************
* Description : Executes a illegal instruction to force a MCU reset.
* Assumptions : None
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
extern void PWRLib_Reset(void);

/*****************************************************************************
* This function is used to  set the global variable which 
* permits(on SET)/restricts(On RESET)the device to enter low power state. 
*
*
* Interface assumptions:
* None
* 
* The routine limitations.
* None
*
* Return value:
* None
*
* Effects on global data.
* It sets the  mLPMFlag
*
* Source of algorithm used.
* None
*
*****************************************************************************/ 
void PWR_AllowDeviceToSleep(void);

/*****************************************************************************
* This function is used to  reset the global variable which 
* permits(on SET)/restricts(On RESET)the device to enter low power state. 
*
*
* Interface assumptions:
* None
* 
* The routine limitations.
* None
*
* Return value:
* None
*
* Effects on global data.
* It resets the  mLPMFlag
*
* Source of algorithm used.
* None
*
*****************************************************************************/ 
void PWR_DisallowDeviceToSleep(void);

/***********************************************************************************
* This function is used to  Check  the Permission flag to go to 
* low power mode
*
* Interface assumptions:
* None
* 
* The routine limitations.
* None
*
* Return value:
*   TRUE  : If the device is allowed to go to the LPM else FALSE
*
* Effects on global data.
* None
*
* Source of algorithm used.
* None
*
*****************************************************************************/ 

bool_t PWR_CheckIfDeviceCanGoToSleep( void );

/******************************************************************************
* Description : Will use internal MCU clock, set RADIO in hibernate and then
*               reset it. The MCU IO's are programmed for low power and then
*               MCU are set in STOP3 mode.
* Assumptions : None
* Input       : None
* Output      : FALSE: Hibernate not allowed
*               TRUE : All ok
* Errors      : Not handled
******************************************************************************/
bool_t PWR_Stop3AndOff(void);
/******************************************************************************
* Description : Will use internal MCU clock, set RADIO in hibernate and then
*               reset it. The MCU are set in STOP1 mode. Only exit if Reset.
* Assumptions : None
* Input       : None
* Output      : FALSE: Hibernate not allowed
*               TRUE : All ok
* Errors      : Not handled
******************************************************************************/
bool_t PWR_AllOff(void);

/******************************************************************************
* Description : Will restore normal IO, startup RADIO, init MAC/RADIO without
*               destroying PIB setup and then synchronize to RADIO clock.
*               Calls MAC to resync.
* Assumptions : Only to be called after a prior call to PWR_Stop3AndOff()
* Input       : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWR_RunAgain(void);

/******************************************************************************
* Description : Will check if DeepSleep are allowed by MAC and Stack.
*               DeepSleep are where RADIO is in reset and MCU are in STOP3
* Assumptions : None
* Input       : None
* Output      : TRUE  : Both MAC and Stack allows DeepSleep
*               FALSE : One or more are busy
* Errors      : Not handled
******************************************************************************/
bool_t PWR_DeepSleepAllowed(void);

/******************************************************************************
* Description : Will check if Sleep are allowed by MAC and Stack.
*               Sleep are when system will wake on an interrupt. Typically the
*               RADIO is set in DOZE mode and MCU in WAIT mode.
* Assumptions : None
* Input       : None
* Output      : TRUE  : Success - MAC and Stack both allows sleep
*               FALSE : One or more are busy
* Errors      : Not handled
******************************************************************************/
bool_t  PWR_SleepAllowed(void);

/******************************************************************************
* Description : Will go into a DeepSleep mode. Depending on what was decided on
*               Compile time, other reactions can be chosen (For test)
* Assumptions : None
* Input       : DozeDuration    : The number of RTI ticks (1024msec) to DeepSleep
* Output      : Bit array containing the reason for wakeup - please see the
*               definition in PWRLib.h
* Errors      : Not handled
******************************************************************************/
PWRLib_WakeupReason_t  PWR_HandleDeepSleep( zbClock24_t  DozeDuration);

/******************************************************************************
* Description : Will go into a Sleep mode. Depending on what was decided on
*               Compile time, other reactions can be chosen (For test).
* Assumptions : None
* Input       : DozeDuration    : The time in Symbols to wait (16usec)
* Output      : Bit array containing the reason for wakeup - please see the
*               definition in PWRLib.h
* Errors      : Not handled 
******************************************************************************/
PWRLib_WakeupReason_t  PWR_HandleSleep( zbClock24_t DozeDuration);

/******************************************************************************
* Description : Will check for new power state to enter.
* Note        : Intended for use in main code.
* Input       : NewPowerState   : The wanted new power state. Can be one of the following:
*                 PWR_Run       : No change will continue to run. Does nothing.
*                 PWR_Sleep     : Will enter a RADIO Doze mode for spec. time.
*                 PWR_DeepSleep : Will Power down RADIO and Stop the MCU.
*                 PWR_Reset     : Force a reset of MCU.
*                 PWR_OFF       : Power down RADIO and MCU completely. Reset to wake.
*               DozeDuration    : The time to sleep. In symbols when PWR_Sleep.
*                                 In times 1024ms when PWR_DeepSleep.
* Output      : Bit array containing the reason for wakeup - please see the
*               definition in PWRLib.h
* Assumptions : None
* Errors      : Not handled
******************************************************************************/
PWRLib_WakeupReason_t  PWR_CheckForAndEnterNewPowerState(PWR_CheckForAndEnterNewPowerState_t NewPowerState, zbClock24_t DozeDuration);


/******************************************************************************
* Description : Detection of low voltage.
*               Will call PWRLib_LVD_CollectLevel to get level directly
*               when cPWR_LVD_Enable == 1. When cPWR_LVD_Enable == 2 it just reports
*               last sampled value. When cPWR_LVD_Enable == 0 or cPWR_LVD_Enable == 3, PWR_NODEPOWER_LEVEL_100
*               is always returned and no collection of level is done
* Assumptions : None
* NOTE        : See the PWRLib_LVD_CollectLevel function for better description
* Inputs      : None
* Output      : PWR_NODEPOWER_LEVEL_CRITICAL..PWR_NODEPOWER_LEVEL_100 : As
*               specified above
* Errors      : Not handled
******************************************************************************/
PWRLib_LVD_VoltageLevel_t PWRLib_LVD_ReportLevel(void);

/******************************************************************************
* Description : Will initialize the project specific registers and setup for
*               correct function of PWR_CheckForAndEnterNewPowerState() function.
* Assumptions : Intended for use in main code and to be modified for each new
*               project.
* Input       : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWR_CheckForAndEnterNewPowerState_Init(void);
/******************************************************************************
* Description : This function makes the decision between Light Sleep and 
                Deep Sleep and goes to the sleep mode.
                The function is protected from interrupt.
* Input       : None
* Output      : PWRLib_WakeupReason_t
* Errors      : Not handled
******************************************************************************/
PWRLib_WakeupReason_t PWR_EnterLowPower(void);

/******************************************************************************
* Description : This function is used to modify the default configured Doze Time 
*               in DeepSleep mode 6.
*             
* Input       : dozeTimeMs - new doze time duration in milliseconds
* Output      : none
* Errors      : Not handled
******************************************************************************/
void PWR_SetDozeTimeMs(uint32_t dozeTimeMs);

/******************************************************************************
* Description : This function is used to modify the default configured Doze Time 
*               in DeepSleep mode 6.
*             
* Input       : dozeTimeSym - new doze time duration in Mac symbols (16us)
* Output      : none
* Errors      : Not handled
******************************************************************************/
void PWR_SetDozeTimeSymbols(uint32_t dozeTimeSym);

/*****************************************************************************/
 #endif
