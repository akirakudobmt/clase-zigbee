/*****************************************************************************
* Accelerometer declarations.
*
* (c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _ACCELEROMETER_H_
#define _ACCELEROMETER_H_


#include "ACC_Interface.h"


/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/


#define ACC_SLAVE_ADDRESS               0x4C

#define ACC_XOUT_REG                    (0x00)
#define ACC_YOUT_REG                    (0x01)
#define ACC_ZOUT_REG                    (0x02)
#define ACC_TILT_REG                    (0x03)
#define ACC_SAMPLE_RATE_STATUS_REG      (0x04)               
#define ACC_SLEEP_COUNT_REG             (0x05)
#define ACC_INT_SETUP_REG               (0x06)
#define ACC_MODE_REG                    (0x07)
#define ACC_SR_REG                      (0x08)
#define TAP_DETECTION_REG               (0x09)
#define TAP_DEBOUNCE_COUNT_REG          (0x0A)

/* Mode reg setup */
#define gModeSetupActive_c              (1<<0)
#define gModeSetupAutoWake_c            (1<<3)
#define gModeSetupAutoSleep_c           (1<<4)
#define gModeSetupIpp_c                 (1<<6)    
#define gModeSetupIah_c                 (1<<7)

/* Sample rate register pos */
#define gSrAwsrPos_c (3)
#define gSrFiltPos_c (5)

/* Tap/pulse detection register bit pos */
#define gXdaPos_c (5)
#define gYdaPos_c (6)
#define gZdaPos_c (7)

/* SR clear mask */
#define gSrAmsrMask_c (0xF8)
#define gSrAwsrMask_c (0xE7)
#define gSrFiltMask_c (0x1F)

/* Tilt status events mask */
#define gTilt_ShakeMask_c  (1<<7)
#define gTilt_AlertMask_c  (1<<6)
#define gTilt_TapMask_c    (1<<5)
#define gTilt_PoLaMask_c   ((1<<4) | (1<<3) | (1<<2))
#define gTilt_BaFroMask_c  ((1<<0) | (1<<1))

/* INTSU mask */
#define gIntsu_ShakeOnX_c (1<<7)
#define gIntsu_ShakeOnY_c (1<<6)
#define gIntsu_ShakeOnZ_c (1<<5)
#define gIntsu_GInt_c     (1<<4)
#define gIntsu_AutoSleepInt_c      (1<<3)
#define gIntsu_TapDetetectInt_c    (1<<2)
#define gIntsu_PLInt_c    (1<<1)
#define gIntsu_FBInt_c    (1<<0)

/* Tilt status position */
#define gTilt_PoLaPos_c (2)

/* IRQSC bit def */
#define gIrqscDetectionMode_c     (1<<0)
#define gIrqscIntEnable_c         (1<<1)
#define gIrqscAck_c               (1<<2)
#define gIrqscFlag_c              (1<<3)
#define gIrqscPinEnable_c         (1<<4)
#define gIrqscEdgeSelect_c        (1<<5)
#define gIrqscPullDeviceDisable_c (1<<6)

/* IRQ pin */
#define gIRQ_Pin_c            (PTADD)
#define ACC_IRQ_SetPinInput() (gIRQ_Pin_c |= (1<<3)) 


/* Possible substatates of the accelerometer states */

/* Substates in Set Power mode state */
#define gAccSubstate_WaitToReadPowerModeReg_c  0x00
#define gAccSubstate_WaitToWritePowerModeReg_c 0x01

/* Substates in Set Samples state */
#define gAccSubstate_WaitToReadSampleReg_c     0x00
#define gAccSubstate_WaitToWriteSampleReg_c    0x01


/* ACC task events */
#define gAccEvent_AppServiceRequest_c       (1<<0)
#define gAccEvent_AccInterrupt_c            (1<<1)
#define gAccEvent_IICOperationSuccess_c     (1<<2)
#define gAccEvent_IICOperationFailed_c      (1<<3)

#define gAccEvent_MaskEventsForIdleState_c  (gAccEvent_AppServiceRequest_c | gAccEvent_AccInterrupt_c)

/*****************************************************************************
******************************************************************************
* Private type declarations
******************************************************************************
*****************************************************************************/

/* Data struct sent to ACC */
typedef struct dataToAcc_tag 
{
  uint8_t regAddress;
  uint8_t data;
} dataToAcc_t;

/* Data struct received from ACC */
typedef struct dataFromACC_tag {
  uint8_t xOutReg;
  uint8_t yOutReg;
  uint8_t zOutReg;
  uint8_t tiltStatusReg;
  uint8_t samplingRateStatustReg;
  uint8_t sleepCountReg;
  uint8_t interruptSetupReg;
  uint8_t modeReg;
  uint8_t mSR_Reg;//auto-wake/sleep and portrait/landscape samples per seconds
                  //and debounce filter  
  uint8_t tapDetectionReg;
  uint8_t tapDebounceCount;  
} dataFromAcc_t;


/* ACC task states */
typedef enum
{
  gAccState_Idle_c,
  gAccState_SetEvents_c,
  gAccState_SetPowerMode_c,
  gAccState_SetSamplesPerSecond_c,
  gAccState_SetTapPulseDetection_c,
  gAccState_SetTapPulseDebounceCounter_c,
  gAccState_GetEvents_c,
} accState_t;


typedef enum
{
  gAppToAccReqType_SetEvents_c = 0,
  gAppToAccReqType_SetPowerMode_c,
  gAppToAccReqType_SetSamplesPerSecond_c,
  gAppToAccReqType_SetTapPulseDetection_c,
  gAppToAccReqType_SetTapPulseDebounceCounter_c
}appToAccReqType_t;


/* Information of the accelerometer state machine */
typedef struct 
{
  accState_t        state;
  uint8_t           substate;
  bool_t            bAppRequestInProcess;
  appToAccReqType_t appToAccReqType;
  event_t           eventsForIdleState;
}accStateMachine_t;

/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/

#endif /* _ACCELEROMETER_H_ */
