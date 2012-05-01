/************************************************************************************
* Source file for accelerometer driver.
*
* (c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "TMR_Interface.h"
#include "EmbeddedTypes.h"
#include "PortConfig.h"
#include "Accelerometer.h"
#include "ACC_Interface.h"
#include "IIC.h"
#include "IIC_Interface.h"
#include "TS_Interface.h"
#include "FunctionLib.h"

#ifndef gRF4CEIncluded_d
#define gRF4CEIncluded_d 0
#endif

#if gRF4CEIncluded_d
  #include "NwkGlobals.h"
#endif


#if (gAccelerometerSupported_d == TRUE)

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Private type definitions
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Public memory definitions
******************************************************************************
*****************************************************************************/


/*****************************************************************************
******************************************************************************
* Private memory definitions
******************************************************************************
*****************************************************************************/
/* Pointer to the acc callback registered by the application */
static ACCFunction_t mpfKeyFunction;

/* Variable storing the ID of the acc task */
static tsTaskID_t    mAccTaskID_c;

/* Variable storing information that should be sent to acc over I2C
This is kept globally, as it should not be altered while the I2C is
performing the write operation, on its own task */
static dataToAcc_t   mDataToAcc;

/* Variable storing information that is read from acc over I2C
This is kept globally, as the I2C is performing the read operation 
on its own task */
static dataFromAcc_t mDataFromAcc;

/* Extra variable that is needed in the SetSamplesPerSecond state */
static uint8_t       mDataToAccMask;

/* State machine of the accelerometer */
static accStateMachine_t mAccStateMachine =
{                                                                
  gAccState_Idle_c, /*  accState_t        state;                  */
  0x00,             /*  uint8_t           substate;               */
  FALSE,            /*  bool_t            bAppRequestInProcess;   */
  0x00,             /*  appToAccReqType_t appToAccReqType;        */
  0x00              /*  event_t           eventsForIdleState;     */
};



/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/
/* Handlers for the states of the accelerometer */
static void ACC_HandleStateIdle                 (event_t events);
static void ACC_HandleStateSetEvents            (event_t events);
static void ACC_HandleStateSetPowerMode         (event_t events);
static void ACC_HandleStateSetSamplesPerSecond  (event_t events);
static void ACC_HandleStateSetTapDetection      (event_t events);
static void ACC_HandleStateSetTapDebounceCounter(event_t events);
static void ACC_HandleStateGetEvents            (event_t events);

/* Other private services */
static bool_t ACC_InitAppRequest(void);
static void   ACC_SendCnfEventToApp(accEventType_t accEventType, bool_t status);
static void   ACC_SendIndEventToApp(accEvent_t accEventType);
static void   ACC_IICOperationComplete(uint8_t status);

/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/*****************************************************************************
* AccTask
*
* Accelerometer task implements various states for accelerometer functionality
******************************************************************************/

void AccTask(event_t events) 
{ 
  /*  As only one event is processed at a time, make sure we don't loose the others, in case they
  come in the same time */
  mAccStateMachine.eventsForIdleState |= (events & gAccEvent_MaskEventsForIdleState_c); 

   
  switch(mAccStateMachine.state)
  {
    case gAccState_Idle_c:
      ACC_HandleStateIdle(events);
      break;
      
    case gAccState_GetEvents_c:
      ACC_HandleStateGetEvents(events);
      break;
      
    case gAccState_SetEvents_c:
      ACC_HandleStateSetEvents(events);
      break;
      
    case gAccState_SetPowerMode_c:
      ACC_HandleStateSetPowerMode(events);
      break;
    
    case gAccState_SetSamplesPerSecond_c:
      ACC_HandleStateSetSamplesPerSecond(events);
      break;
      
    case gAccState_SetTapPulseDetection_c:
      ACC_HandleStateSetTapDetection(events);
      break;
    
    case gAccState_SetTapPulseDebounceCounter_c:
      ACC_HandleStateSetTapDebounceCounter(events);  
      break;
  }

  /* If there still are events for idle state, keep the task running */ 
  if(mAccStateMachine.eventsForIdleState) 
  {
    TS_SendEvent(mAccTaskID_c, mAccStateMachine.eventsForIdleState); 
  }
}
#endif /* gAccelerometerSupported_d */ 


/*****************************************************************************
* ACC_Init
*
* Initiliaze accelerometer module
******************************************************************************/

bool_t ACC_Init(ACCFunction_t pfCallBackAdr)
{
#if (gAccelerometerSupported_d == TRUE)
  uint8_t retStatus = FALSE;
  
  /* Check if callback is not null */
  if(pfCallBackAdr)
  {
    /* Make ACC interrupt pin input  */
    ACC_IRQ_SetPinInput(); 
    /* where to send the data and events */
    mpfKeyFunction = pfCallBackAdr;
    /* Create acceleromter task */
    mAccTaskID_c = TS_CreateTask(gTsAccTaskPriority_c, AccTask);
    if (mAccTaskID_c != gTsInvalidTaskID_c)
    {
      retStatus = TRUE;
      /* Intialize interrupt pin */
      IRQSC = gIrqscIntEnable_c | gIrqscPinEnable_c | gIrqscEdgeSelect_c;
    }
  }
  return retStatus;
#else
  (void)pfCallBackAdr;
  return TRUE;
#endif /* gAccelerometerSupported_d */
}

/*****************************************************************************
* ACC_Uninit
* 
* Uninit the accelerometer module, destroy task
*****************************************************************************/
void ACC_Uninit(void)
{
#if (gAccelerometerSupported_d == TRUE)
   
  /* Destroy the ACC task if there is a valid ID */
  if(gTsInvalidTaskID_c != mAccTaskID_c)
  {
    TS_DestroyTask(mAccTaskID_c);  
  }
  
#endif /* gAccelerometerSupported_d */
}

/*****************************************************************************
* ACC_SetEvents
*
* Set events function -- used to setup the interrupt register
******************************************************************************/

bool_t ACC_SetEvents(uint16_t accMaskEvents)
{
#if (gAccelerometerSupported_d == TRUE)  
  if(ACC_InitAppRequest() == FALSE)
    return FALSE;

  /* Store information needed for when the acc task will get the chance to run
  and to start processing this request */
  mAccStateMachine.appToAccReqType = gAppToAccReqType_SetEvents_c;
  
  mDataToAcc.regAddress = ACC_INT_SETUP_REG;
  mDataToAcc.data       = (uint8_t)(accMaskEvents >> 1);
#else
  (void)accMaskEvents;
#endif /* gAccelerometerSupported_d */
  
  return TRUE;
}

                         
/*****************************************************************************
* ACC_SetPowerMode
*
* Set power mode
******************************************************************************/

bool_t ACC_SetPowerMode(gAccPowerMode_t accPowerMode)
{
#if (gAccelerometerSupported_d == TRUE) 
  if(ACC_InitAppRequest() == FALSE)
    return FALSE;
  
  /* Store information needed for when the acc task will get the chance to run
  and to start processing this request */
  mAccStateMachine.appToAccReqType = gAppToAccReqType_SetPowerMode_c;

  mDataToAcc.regAddress = ACC_MODE_REG;        
  mDataToAcc.data       = accPowerMode;
#else
  (void)accPowerMode;
#endif /* gAccelerometerSupported_d */
   
  return TRUE;
}



/*****************************************************************************
* ACC_SetSamplesPerSecond
*
* Set samples per seconds
******************************************************************************/

bool_t ACC_SetSamplesPerSecond(gAccSamplesPerSecondSetup_t accSamplesPerSecond)
{
#if (gAccelerometerSupported_d == TRUE)
  if(ACC_InitAppRequest() == FALSE)
    return FALSE;
  
  /* Store information needed for when the acc task will get the chance to run
  and to start processing this request */
  mAccStateMachine.appToAccReqType = gAppToAccReqType_SetSamplesPerSecond_c;

  mDataToAcc.regAddress = ACC_SR_REG;
  switch(accSamplesPerSecond.sampleRegisterSelect)
  {    
    case gAccSampleRegisterAM_c:
      mDataToAcc.data = ~gSrAmsrMask_c  & (accSamplesPerSecond.sampleRegister.activeAutoSleepSampleRate);       
      mDataToAccMask  = gSrAmsrMask_c;       
      break;
    
    case gAccSampleRegisterAW_c:
      mDataToAcc.data = ~gSrAwsrMask_c  & (accSamplesPerSecond.sampleRegister.autoWakeSampleRate << gSrAwsrPos_c);             
      mDataToAccMask  = gSrAwsrMask_c;       
      break;
    
    case gAccSampleRegisterTilt_c:            
      mDataToAcc.data = ~gSrFiltMask_c  & (accSamplesPerSecond.sampleRegister.tiltSamplesBeforeGen << gSrFiltPos_c);                       
      mDataToAccMask  = gSrFiltMask_c;       
      break;
  }
#else
  (void)accSamplesPerSecond;
#endif /* gAccelerometerSupported_d */
  
  return TRUE;
}



/*****************************************************************************
* ACC_SetTapPulseDetection
*
* Setup the tap/pulse dection
******************************************************************************/

bool_t ACC_SetTapPulseDetection(gAccTapSetup_t accTapSetup)
{
#if (gAccelerometerSupported_d == TRUE)
  if(ACC_InitAppRequest() == FALSE)
    return FALSE;
  
  /* Store information needed for when the acc task will get the chance to run
  and to start processing this request */
  mAccStateMachine.appToAccReqType = gAppToAccReqType_SetTapPulseDetection_c;

  mDataToAcc.regAddress = TAP_DETECTION_REG;                             
  mDataToAcc.data       = (uint8_t)( accTapSetup.tapDetectThresholdCount | (accTapSetup.xTapDetectEnable << gXdaPos_c) | (accTapSetup.yTapDetectEnable << gYdaPos_c) | (accTapSetup.zTapDetectEnable << gZdaPos_c) );
#else
  (void)accTapSetup;
#endif /* gAccelerometerSupported_d */
  
  return TRUE;
}


/*****************************************************************************
* ACC_SetTapPulseDebounceCounter
*
* Setup the tap/pulse debounce counter
******************************************************************************/

bool_t ACC_SetTapPulseDebounceCounter(uint8_t accTapDebounceCounter)
{
#if (gAccelerometerSupported_d == TRUE)
  if(ACC_InitAppRequest() == FALSE)
    return FALSE;
  
  /* Store information needed for when the acc task will get the chance to run
  and to start processing this request */
  mAccStateMachine.appToAccReqType = gAppToAccReqType_SetTapPulseDebounceCounter_c;

  mDataToAcc.regAddress = TAP_DEBOUNCE_COUNT_REG;                             
  mDataToAcc.data       = accTapDebounceCounter;
#else
  (void)accTapDebounceCounter;
#endif /* gAccelerometerSupported_d */
  
  return TRUE;
}

/*****************************************************************************
* ACC_Isr
*
* Interrupt service routine for external IRQ pin
******************************************************************************/

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

INTERRUPT_KEYWORD void ACC_Isr(void)
{
#if (gAccelerometerSupported_d == TRUE)
  /* Check the IRQ interrupt bit */    
  if(IRQSC & gIrqscFlag_c)
  {
    /* clear IRQ flag */
    IRQSC |= gIrqscAck_c;
    /* Send event to ACC  */
    TS_SendEvent(mAccTaskID_c, gAccEvent_AccInterrupt_c);    
  }
#endif /* gAccelerometerSupported_d */
}

#pragma CODE_SEG DEFAULT
  
  
#if (gAccelerometerSupported_d == TRUE)  
/*****************************************************************************
******************************************************************************
* Private functions
******************************************************************************
*****************************************************************************/
  
  
/*****************************************************************************
* ACC_HandleStateIdle
*
* Handler of the 'Idle' state
******************************************************************************/

static void ACC_HandleStateIdle(event_t events)
{
  (void)events;
  
  /* There are 2 types of events that can appear while the acc component is in idle state
      1. Component receives a request from application to execute some action
      2. Component receives an interrupt from acc peripheral signaling the availability of some data
     The component will process one event at a time, as both of them involve I2C read and write 
  operations. The request from application has a higer priority as it may for example request 
  disabling some active events that are generated very fast (like realTimeMotion), but will not
  be able to do this if it is always served second */

  if(mAccStateMachine.eventsForIdleState & gAccEvent_AppServiceRequest_c)
  {
    /* Check what type of request has been received from app and act accordingly */
    switch(mAccStateMachine.appToAccReqType)
    {
      case gAppToAccReqType_SetEvents_c:
        if( IIC_Transmit_Master((uint8_t *)&mDataToAcc, sizeof(mDataToAcc), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == FALSE )  
        { 
          ACC_SendCnfEventToApp(gAccEventType_SetEventsCnf_c, FALSE);   
        }
        else
        {
          mAccStateMachine.state = gAccState_SetEvents_c;  
        }
        break;
      
      case gAppToAccReqType_SetPowerMode_c:
        /* Read acc mode register */  
        if(IIC_Receive_Master((uint8_t *)&mDataFromAcc, sizeof(dataFromAcc_t), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == FALSE )  
        {
          ACC_SendCnfEventToApp(gAccEventType_SetPowerModeCnf_c, FALSE);  
        }
        else
        {
          mAccStateMachine.state = gAccState_SetPowerMode_c;  
        }
        break;
        
      case gAppToAccReqType_SetSamplesPerSecond_c:
        if( IIC_Receive_Master((uint8_t *)&mDataFromAcc, sizeof(dataFromAcc_t), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == FALSE)
        {
          ACC_SendCnfEventToApp(gAccEventType_SetSamplesPerSecondCnf_c, FALSE);
        }
        else
        {
          mAccStateMachine.state = gAccState_SetSamplesPerSecond_c;   
        }
        break;

      case gAppToAccReqType_SetTapPulseDetection_c:
        if( IIC_Transmit_Master((uint8_t *)&mDataToAcc, sizeof(mDataToAcc), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == FALSE )  
        {
          ACC_SendCnfEventToApp(gAccEventType_SetTapDetectionCnf_c, FALSE);
        }    
        else
        {
          mAccStateMachine.state = gAccState_SetTapPulseDetection_c;    
        }
        break;
        
      case gAppToAccReqType_SetTapPulseDebounceCounter_c:
        if( IIC_Transmit_Master((uint8_t *)&mDataToAcc, sizeof(mDataToAcc), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == FALSE )  
        {
          ACC_SendCnfEventToApp(gAccEventType_SetTapDebounceCounterCnf_c, FALSE);
        }      
        else
        {
          mAccStateMachine.state = gAccState_SetTapPulseDebounceCounter_c;      
        }
        break;
    }
    /* Clear the event, as it has been processed */
    mAccStateMachine.eventsForIdleState &= ~gAccEvent_AppServiceRequest_c; 
  }
  else if(mAccStateMachine.eventsForIdleState & gAccEvent_AccInterrupt_c)
  {
    /* Try to read the ACC tilt register.
       If not successful, do not clear the event in order to retry reading the register next time the task is run
       This is very important, because as long as we don't successfully read the tilt register, no future interrupt will be triggered 
    by the accelerometer */
    if( IIC_Receive_Master((uint8_t *)&mDataFromAcc, sizeof(dataFromAcc_t), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == TRUE)
    { 
      mAccStateMachine.state = gAccState_GetEvents_c;
      /* Clear the event, as it has been processed */
      mAccStateMachine.eventsForIdleState &= ~gAccEvent_AccInterrupt_c; 
    }
  }
}




/*****************************************************************************
* ACC_HandleStateSetEvents
*
* Handler of the 'Set Events' state
******************************************************************************/

static void ACC_HandleStateSetEvents(event_t events)
{
  if(events & gAccEvent_IICOperationFailed_c)
  {
    ACC_SendCnfEventToApp(gAccEventType_SetEventsCnf_c, FALSE);     
  }
  else if(events & gAccEvent_IICOperationSuccess_c)
  {
    ACC_SendCnfEventToApp(gAccEventType_SetEventsCnf_c, TRUE);   
  }
}


/*****************************************************************************
* ACC_HandleStateSetPowerMode
*
* Handler of the 'Set Power Mode' state
******************************************************************************/

static void ACC_HandleStateSetPowerMode(event_t events)
{
  switch(mAccStateMachine.substate)
  {
    case gAccSubstate_WaitToReadPowerModeReg_c:
      if(events & gAccEvent_IICOperationFailed_c)
      {
        ACC_SendCnfEventToApp(gAccEventType_SetPowerModeCnf_c, FALSE);     
      }
      else if(events & gAccEvent_IICOperationSuccess_c)
      {
        /* Make logical OR between what we have received from application and what we have read from ACC */
        mDataToAcc.data |=  (mDataFromAcc.modeReg & (~gModeSetupActive_c)) | gModeSetupIpp_c | gModeSetupIah_c;
        if( IIC_Transmit_Master((uint8_t *)&mDataToAcc, sizeof(dataToAcc_t), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == FALSE )  
        {
          ACC_SendCnfEventToApp(gAccEventType_SetPowerModeCnf_c, FALSE);     
        }
        else
        {
          /* Advance SM */
          mAccStateMachine.substate = gAccSubstate_WaitToWritePowerModeReg_c;          
        }
      }
      break;

    case gAccSubstate_WaitToWritePowerModeReg_c:
      if(events & gAccEvent_IICOperationFailed_c)
      {
        ACC_SendCnfEventToApp(gAccEventType_SetPowerModeCnf_c, FALSE);     
      }
      else if(events & gAccEvent_IICOperationSuccess_c)
      {
        ACC_SendCnfEventToApp(gAccEventType_SetPowerModeCnf_c, TRUE);   
      }
      break;
  }
}



/*****************************************************************************
* ACC_HandleStateSetSamplesPerSecond
*
* Handler of the 'Set Samples Per Second' state
******************************************************************************/

static void ACC_HandleStateSetSamplesPerSecond(event_t events)
{
  switch(mAccStateMachine.substate)
  {
    case gAccSubstate_WaitToReadSampleReg_c:
      if(events & gAccEvent_IICOperationFailed_c)
      {
        ACC_SendCnfEventToApp(gAccEventType_SetSamplesPerSecondCnf_c, FALSE);     
      }
      else if(events & gAccEvent_IICOperationSuccess_c)
      {
        /* Write back the sample register */
        mDataToAcc.data = mDataFromAcc.mSR_Reg & mDataToAccMask;
        if( IIC_Transmit_Master((uint8_t *)&mDataToAcc, sizeof(dataToAcc_t), ACC_SLAVE_ADDRESS, ACC_IICOperationComplete) == FALSE )  
        {
          ACC_SendCnfEventToApp(gAccEventType_SetSamplesPerSecondCnf_c, FALSE); 
        }
        else
        {
          /* Advance SM */
          mAccStateMachine.substate = gAccSubstate_WaitToWriteSampleReg_c;       
        }        
      }
      break;

    case gAccSubstate_WaitToWriteSampleReg_c:
      if(events & gAccEvent_IICOperationFailed_c)
      {
        ACC_SendCnfEventToApp(gAccEventType_SetSamplesPerSecondCnf_c, FALSE);     
      }
      else if(events & gAccEvent_IICOperationSuccess_c)
      {
        ACC_SendCnfEventToApp(gAccEventType_SetSamplesPerSecondCnf_c, TRUE);   
      }
      break;
  }
}




/*****************************************************************************
* ACC_HandleStateSetTapPulseDetection
*
* Handler of the 'Set Tap Pulse Detection' state
******************************************************************************/

static void ACC_HandleStateSetTapDetection(event_t events)
{
  if(events & gAccEvent_IICOperationFailed_c)
  {
    ACC_SendCnfEventToApp(gAccEventType_SetTapDetectionCnf_c, FALSE);     
  }
  else if(events & gAccEvent_IICOperationSuccess_c)
  {
    ACC_SendCnfEventToApp(gAccEventType_SetTapDetectionCnf_c, TRUE);   
  }
}



/*****************************************************************************
* ACC_HandleStateSetTapDebounce
*
* Handler of the 'Set Tap Debounce ' state
******************************************************************************/

static void ACC_HandleStateSetTapDebounceCounter(event_t events)
{
  if(events & gAccEvent_IICOperationFailed_c)
  {
    ACC_SendCnfEventToApp(gAccEventType_SetTapDebounceCounterCnf_c, FALSE);     
  }
  else if(events & gAccEvent_IICOperationSuccess_c)
  {
    ACC_SendCnfEventToApp(gAccEventType_SetTapDebounceCounterCnf_c, TRUE);   
  }
}

  
  
/*****************************************************************************
* ACC_HandleStateGetEvents
*
* Handler of the 'Get Events' state. This state is responsible for reading
* data from accelerometer every time the accelerometer activates the IRQ
******************************************************************************/

static void ACC_HandleStateGetEvents(event_t events)
{
  accEvent_t accEvent;
  
  if(events & gAccEvent_IICOperationSuccess_c)      
  {
    /* Inspect the accelerometer interrupt source */
    if(mDataFromAcc.tiltStatusReg & gTilt_AlertMask_c)
    {
      /* Data read from ACC is not valid. Do not report any indication to the
      application. Go back to idle state */     
      mAccStateMachine.state     = gAccState_Idle_c; 
      mAccStateMachine.substate  = 0x00;
      
      return;          
    }      
                    
    if( (mDataFromAcc.tiltStatusReg & gTilt_ShakeMask_c) && (mDataFromAcc.interruptSetupReg & (gIntsu_ShakeOnX_c | gIntsu_ShakeOnY_c | gIntsu_ShakeOnZ_c)) )
    {
      /* Shake was detected */
      accEvent.accEventType = gAccEventType_ShakeInd_c;
      ACC_SendIndEventToApp(accEvent);               
    }
    if( (mDataFromAcc.tiltStatusReg & gTilt_TapMask_c) && (mDataFromAcc.interruptSetupReg & gIntsu_TapDetetectInt_c))
    {
      /* Tap was detected*/
      accEvent.accEventType = gAccEventType_TapInd_c;
      ACC_SendIndEventToApp(accEvent);                              
    }        
    if( (mDataFromAcc.tiltStatusReg & gTilt_PoLaMask_c) && (mDataFromAcc.interruptSetupReg & gIntsu_PLInt_c) )
    {
      /* Up, down, left or right condition is detected */
      accEvent.accEventType = gAccEventType_LeftRightUpDownInd_c;
      accEvent.eventBody.accLeftRightUpDownInd.direction = (accUpDownLeftRight_t)((mDataFromAcc.tiltStatusReg & gTilt_PoLaMask_c) >> gTilt_PoLaPos_c);
      ACC_SendIndEventToApp(accEvent);          
    }            
    if( (mDataFromAcc.tiltStatusReg & gTilt_BaFroMask_c) && (mDataFromAcc.interruptSetupReg & gIntsu_FBInt_c))        
    {
      /* Back, front condition is detected */
      accEvent.accEventType = gAccEventType_FrontBackInd_c;
      accEvent.eventBody.accFrontBackInd.direction = (accFrontBack_t)(mDataFromAcc.tiltStatusReg & gTilt_BaFroMask_c);
      ACC_SendIndEventToApp(accEvent);          
    }        
    /* Interupt on G */
    if( (mDataFromAcc.xOutReg & gTilt_AlertMask_c) || (mDataFromAcc.yOutReg & gTilt_AlertMask_c) || (mDataFromAcc.zOutReg & gTilt_AlertMask_c) )
    {
      /* Data read from ACC is not valid. Do not report any indication to the
      application. Go back to idle state */     
      mAccStateMachine.state     = gAccState_Idle_c; 
      mAccStateMachine.substate  = 0x00;
      return;
    }
    else
    {
      if(mDataFromAcc.interruptSetupReg & gIntsu_GInt_c)
      {            
        accEvent.accEventType = gAccEventType_RealTimeMotionInd_c;
        accEvent.eventBody.accRealTimeMotionInd.X = mDataFromAcc.xOutReg;          
        accEvent.eventBody.accRealTimeMotionInd.Y = mDataFromAcc.yOutReg;          
        accEvent.eventBody.accRealTimeMotionInd.Z = mDataFromAcc.zOutReg;          
        ACC_SendIndEventToApp(accEvent); 
      }
    }        
  }
  else
  {
    /* We need to read again the tilt register of the accelerometer. This is done on idle state */
    mAccStateMachine.state     = gAccState_Idle_c; 
    TS_SendEvent(mAccTaskID_c, gAccEvent_AccInterrupt_c);
  }
}
  

/*****************************************************************************
* ACC_InitAppRequest
*
* Tries to initialize a new request from the application
******************************************************************************/

static bool_t ACC_InitAppRequest(void)
{  
  /* If another app request is already processing, return fail status */
  if (mAccStateMachine.bAppRequestInProcess)
    return FALSE; 
  
  mAccStateMachine.bAppRequestInProcess = TRUE;
  
  /* Send event to acc task to inform it that a request from app is pending */
  TS_SendEvent(mAccTaskID_c, gAccEvent_AppServiceRequest_c); 
  
  return TRUE;
}
  
  
/*****************************************************************************
* ACC_IICOperationComplete
*
* Triggered every time a IIC read or write operation completes
******************************************************************************/

static void ACC_IICOperationComplete(uint8_t status)
{
  if(status == TRUE)
  {
    TS_SendEvent(mAccTaskID_c, gAccEvent_IICOperationSuccess_c);
  }
  else
  {
    TS_SendEvent(mAccTaskID_c, gAccEvent_IICOperationFailed_c);
  }        
}


/*****************************************************************************
* ACC_SendCnfEventToApp
*
* Sends the confirm event to the application
******************************************************************************/

static void ACC_SendCnfEventToApp(accEventType_t accEventType, bool_t status)
{
  accEvent_t accEvent;

  accEvent.accEventType                     = accEventType;
  accEvent.eventBody.accSetEventsCnf.status = status;
  /* App request executed */
  mAccStateMachine.bAppRequestInProcess = FALSE;
  /* Can switch back to idle state */
  mAccStateMachine.state     = gAccState_Idle_c; 
  mAccStateMachine.substate  = 0x00;
  /* Send confirm to application */ 
  mpfKeyFunction(accEvent);   
}


/*****************************************************************************
* ACC_SendIndToApp
*
* Sends the indication event to the application
******************************************************************************/

static void ACC_SendIndEventToApp(accEvent_t accEvent)
{
  /* Can switch back to idle state */
  mAccStateMachine.state     = gAccState_Idle_c; 
  mAccStateMachine.substate  = 0x00;
  /* Send confirm to application */ 
  mpfKeyFunction(accEvent);   
}

#endif /* gAccelerometerSupported_d */
