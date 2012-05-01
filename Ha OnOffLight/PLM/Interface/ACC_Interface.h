/************************************************************************************
* This header file is for Accelerometer Driver Interface.
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _ACC_INTERFACE_H_
#define _ACC_INTERFACE_H_

#include "IrqControlLib.h"
#include "AppToPlatformConfig.h"


#ifndef gAccelerometerSupported_d
#define gAccelerometerSupported_d       FALSE
#endif

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* Events enabled for ACC component */
#define	gAccEvent_ShakeOnXDetection_c  		  (1<<8)
#define	gAccEvent_ShakeOnYDetection_c  		  (1<<7)
#define	gAccEvent_ShakeOnZDetection_c  		  (1<<6)
#define	gAccEvent_RealTimeMotion_c 			    (1<<5)
#define	gAccEvent_AutoSleep_c  			        (1<<4)
#define	gAccEvent_TapDetection_c  			    (1<<3)
#define	gAccEvent_LeftRightUpDownDetection_c  	(1<<2)
#define	gAccEvent_FrontBackDetection_c  		    (1<<1)

/* Accelerometer Task priority */
#ifndef gTsAccTaskPriority_c        
  #define gTsAccTaskPriority_c      (0x9) 
#endif

/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// Events sent from ACC component
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

typedef enum
{
  gAccFront_c = 0x01,
  gAccBack_c  = 0x02
}accFrontBack_t;


typedef enum
{
  gAccLeft_c  = 0x01,
  gAccRight_c = 0x02,
  gAccDown_c  = 0x05,   
  gAccUp_c    = 0x06
}accUpDownLeftRight_t;


typedef enum
{
  gAccEventType_SetPowerModeCnf_c,
  gAccEventType_SetEventsCnf_c,
  gAccEventType_SetSamplesPerSecondCnf_c,
  gAccEventType_SetTapDetectionCnf_c,
  gAccEventType_SetTapDebounceCounterCnf_c,
  /* ------------------------------- */
  gAccEventType_ShakeInd_c,
  gAccEventType_TapInd_c,
  gAccEventType_RealTimeMotionInd_c,
  gAccEventType_FrontBackInd_c,
  gAccEventType_LeftRightUpDownInd_c,    
}accEventType_t; 


/* Set power confirm status */
typedef struct
{
  uint8_t status;
} accSetPowerModeCnf_t;

/* Events setup confirm status */
typedef struct 
{
  bool_t status;
} accSetEventsCnf_t;

/* Sample rate confirm status */
typedef struct 
{  
  uint8_t status;
} accSetSamplesPerSecondCnf_t;

/* Set tap detection confirm status */
typedef struct
{
  uint8_t status;
} accSetTapDectionCnf_t;

/* Set tap debounce counter */
typedef struct
{
  uint8_t status;
} accSetTapDebounceCounCnf_t;

/* Values of accelerometer axes */
typedef struct
{
  uint8_t X;
  uint8_t Y;
  uint8_t Z;
} accRealTimeMotionInd_t;

/* direction indication */
typedef struct
{
  accFrontBack_t direction;
} accFrontBackInd_t;

typedef struct
{
  accUpDownLeftRight_t direction;
} accLeftRightUpDownInd_t;

/* Event and status sent from ACC component */
typedef struct
{
  accEventType_t accEventType;
  union
  {
    accSetPowerModeCnf_t        accSetPowerModeCnf;
    accSetEventsCnf_t           accSetEventsCnf;
    accSetSamplesPerSecondCnf_t accSetSamplesPerSecondCnf;
    accSetTapDectionCnf_t       accSetTapDetectionCnf;
    accSetTapDebounceCounCnf_t  accSetTapDebounceCountCnf;
    /* ----------------------------------------------- */
    accRealTimeMotionInd_t      accRealTimeMotionInd;
    accFrontBackInd_t           accFrontBackInd;
    accLeftRightUpDownInd_t     accLeftRightUpDownInd;
  }eventBody;
} accEvent_t;


/* Callback function prototype */
typedef void (*ACCFunction_t) (accEvent_t event);

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// Accelerometer functionality
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

/* ACC mode */
typedef enum
{
  gAccPowerMode_Standby_c     = 0x00,
	gAccPowerMode_Active_c      = 0x01	
} gAccPowerMode_t;	


/* Auto sleep active mode sample rate */
typedef enum
{
  gAccSamplesPerSecondAM_120_c    = 0x00,
  gAccSamplesPerSecondAM_64_c     = 0x01,
  gAccSamplesPerSecondAM_32_c     = 0x02,		
  gAccSamplesPerSecondAM_16_c     = 0x03,
  gAccSamplesPerSecondAM_8_c      = 0x04,	
  gAccSamplesPerSecondAM_4_c      = 0x05,
  gAccSamplesPerSecondAM_2_c      = 0x06,		
	gAccSamplesPerSecondAM_1_c      = 0x07	
} gAccSamplesPerSecondAM_t;	

/* Auto wake sample rate */
typedef enum
{
  gAccSamplesPerSecondAW_32_c     = 0x00,		
  gAccSamplesPerSecondAW_16_c     = 0x01,
  gAccSamplesPerSecondAW_8_c      = 0x02,	
	gAccSamplesPerSecondAW_1_c      = 0x03
} gAccSamplesPerSecondAW_t;	

/* Tilt No of samples */
typedef enum   
{
  gAccSamplesTiltEverySample_c  = 0x00,
  gAccSamplesTilt_2_c  = 0x01,  
  gAccSamplesTilt_3_c  = 0x02,  
  gAccSamplesTilt_4_c  = 0x03,  
  gAccSamplesTilt_5_c  = 0x04,  
  gAccSamplesTilt_6_c  = 0x05,  
  gAccSamplesTilt_7_c  = 0x06,  
  gAccSamplesTilt_8_c  = 0x07
} gAccSamplesTilt_t; 

/* sample register */
typedef enum 
{
  gAccSampleRegisterAM_c   = 0x00,
  gAccSampleRegisterAW_c   = 0x01,
  gAccSampleRegisterTilt_c = 0x02
} gAccSampleRegister_t;

typedef struct 
{
  gAccSampleRegister_t sampleRegisterSelect; 
  union {    
    gAccSamplesPerSecondAM_t activeAutoSleepSampleRate;  
    gAccSamplesPerSecondAW_t autoWakeSampleRate;
    gAccSamplesTilt_t        tiltSamplesBeforeGen;     
  } sampleRegister;
} gAccSamplesPerSecondSetup_t;

/* Tap setup */
typedef struct 
{  
  bool_t  xTapDetectEnable;
  bool_t  yTapDetectEnable;
  bool_t  zTapDetectEnable;
  uint8_t tapDetectThresholdCount;  
} gAccTapSetup_t;

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

/******************************************************************************
* This function initializes the accelerometer module
*
* Interface assumptions:
*
* Return value:
* TRUE or FALSE
*
******************************************************************************/

extern bool_t ACC_Init (ACCFunction_t pfCallBackAdr);

/*****************************************************************************
* This function un-initializes the accelerometer module
*
* Interface assumption:
*
* Return value: None
* 
*****************************************************************************/

extern void ACC_Uninit(void);

/******************************************************************************
* This function sets the power mode of the accelerometer module
*
* Interface assumptions:
*
* Return value:
* None
*
******************************************************************************/
extern bool_t ACC_SetPowerMode(gAccPowerMode_t accPowerMode);

/******************************************************************************
* This function sets what events will be generated from ACC module
*
* Interface assumptions:
*
* Return value:
* None
*
******************************************************************************/

extern bool_t ACC_SetEvents(uint16_t maskAccEvents);

/******************************************************************************
* This function sets the number of samples per secods
*
* Interface assumptions:
*
* Return value:
* None
*
******************************************************************************/
 
extern bool_t ACC_SetSamplesPerSecond(gAccSamplesPerSecondSetup_t accSamplesPerSecondSetup); 

/******************************************************************************
* This function sets the tap axes enable/disable and the no. of threshold counts
*
* Interface assumptions:
*
* Return value:
* None
*
******************************************************************************/

extern bool_t ACC_SetTapPulseDetection(gAccTapSetup_t accTapSetup);

/******************************************************************************
* This function sets the tap debounce conter
*
* Interface assumptions:
*
* Return value:
* None
*
******************************************************************************/

extern bool_t ACC_SetTapPulseDebounceCounter(uint8_t accTapDebounceCount);


/******************************************************************************
* This is the interrupt service routine for the external IRQ pin
*
* Interface assumptions:
*
* Return value:
* None
*
******************************************************************************/


/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

INTERRUPT_KEYWORD void ACC_Isr(void);

#pragma CODE_SEG DEFAULT

 
#endif /* _ACC_INTERFACE_H_ */
