/*****************************************************************************
* Timer implementation.
*
* Copyright (c) 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "EmbeddedTypes.h"
#include "IrqControlLib.h"
#include "TS_Interface.h"
#include "TMR_Interface.h"
#include "Timer.h"
#include "FunctionLib.h"

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/


/* Number of elements in an array. */
#define NumberOfElements(array)   ((sizeof(array) / (sizeof(array[0]))))

/* Timer status. If none of these flags are on, the timer is not allocated.
* For allocated timers, exactly one of these flags will be set.
* mTmrStatusActive_c   Timer has been started and has not yet expired.
* mTmrStatusReady_c    TMR_StartTimer() has been called for this timer, but
*                      the timer task has not yet actually started it. The
*                      timer is considered to be active.
* mTmrStatusInactive_c Timer is allocated, but is not active.
*/
typedef uint8_t tmrStatus_t;
#define mTmrStatusFree_c        0
#define mTmrStatusActive_c      0x20
#define mTmrStatusReady_c       0x40
#define mTmrStatusInactive_c    0x80

#define mTimerStatusMask_c      ( mTmrStatusActive_c \
                                | mTmrStatusReady_c \
                                | mTmrStatusInactive_c)

/* If all bits in the status byte are zero, the timer is free. */
#define TMR_IsTimerAllocated(timerID)   (maTmrTimerStatusTable[(timerID)])
#define TMR_MarkTimerFree(timerID)       (maTmrTimerStatusTable[(timerID)] = mTmrStatusFree_c)

#define mTimerType_c            ( gTmrSingleShotTimer_c \
                                 | gTmrSetSecondTimer_c \
                                 | gTmrSetMinuteTimer_c \
                                 | gTmrIntervalTimer_c \
                                 | gTmrLowPowerTimer_c )
                              

/* TMR_Task() event flag. Only one event is needed. */
#define mTMR_Event_c    ( 1 << 0 )

/* One minute in milliseconds */
#define mOneMinute_c 60000
/* One second in milliseconds */
#define mOneSecond_c 1000

/* Count to maximum (0xffff - 2*4ms(in ticks)), to be sure that the currentTimeInTicks 
will never roll over previousTimeInTicks in the TMR_Task(); 
A task have to be executed at most in 4 ms  */
#define mMaxToCountDown_c (0x7FFF - TmrTicksFromMilliseconds(8))

/* 4ms in ticks (for different source clocks).
   It's assumed that a task is executed in 4 ms; */
/* Compensate for the real TMR clock (4.096ms) */
#define m4msTicks_c   ((TmrTicksFromMilliseconds(5) == 0) ? 1 : TmrTicksFromMilliseconds(5))

/* Detect the timer type */                                          
#define IsMinuteTimer(type)  (((type) & gTmrMinuteTimer_c) == gTmrMinuteTimer_c)
#define IsSecondTimer(type)  (((type) & gTmrSecondTimer_c) == gTmrSecondTimer_c)                                         
#define IsLowPowerTimer(type)  ((type) & gTmrLowPowerTimer_c)
#define IsLowPowerMinuteTimer(type)  (((type) & gTmrLowPowerMinuteTimer_c) == gTmrLowPowerMinuteTimer_c)
#define IsLowPowerSecondTimer(type)  (((type) & gTmrLowPowerSecondTimer_c) == gTmrLowPowerSecondTimer_c)
#define IsLowPowerSingleShotMillisTimer(type)  (((type) & gTmrLowPowerSingleShotMillisTimer_c) \
                                                  == gTmrLowPowerSingleShotMillisTimer_c)
#define IsLowPowerIntervalMillisTimer(type)  (((type) & gTmrLowPowerIntervalMillisTimer_c) \
                                                  == gTmrLowPowerIntervalMillisTimer_c)

/* Transform ticks to milliseconds
   Is assumed that 1 tick has at least 1ms...
*/
#define TmrMillisecondsFromTicks(ticks) ((uint32_t)((uint16_t)mOneSecond_c/TmrTicksForOneSecond()) * (uint32_t)(ticks))

 
/*****************************************************************************
******************************************************************************
* Private type definitions
******************************************************************************
*****************************************************************************/

/* Used to read 16 bit hardware registers. */
typedef union read16bitReg_tag {
  uint16_t wordAccess;
  uint8_t  byteAccess[2];
} read16bitReg_t;

/* One entry in the main timer table.
* intervalInTicks      The timer's original duration, in ticks. Used to reset
*                      intervnal timers.
* countDown            When a timer is started, this is set to the duration.
*                      The timer task decrements this value. When it reaches
*                      zero, the timer has expired.
*/
typedef struct tmrTimerTableEntry_tag {
  tmrTimerTicks_t intervalInTicks;
  tmrTimerTicks_t countDown;
  pfTmrCallBack_t pfCallBack;
} tmrTimerTableEntry_t;

/* The status and type are bitfields, to save RAM. This costs some code */
/* space, though. */
typedef uint8_t tmrTimerStatus_t;

/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/

static tmrStatus_t TMR_GetTimerStatus(tmrTimerID_t timerID);
static void TMR_SetTimerStatus(tmrTimerID_t timerID, tmrStatus_t status);
static tmrTimerType_t TMR_GetTimerType(tmrTimerID_t timerID);
static void TMR_SetTimerType(tmrTimerID_t timerID, tmrTimerType_t type);

/*****************************************************************************
******************************************************************************
* Private memory declarations
******************************************************************************
*****************************************************************************/

/* Main timer table. All allocated timers are stored here. A timer's ID */
/* is it's index in this table. */
static tmrTimerTableEntry_t maTmrTimerTable[gTmrTotalTimers_c];

/* Making the single-byte-per-timer status table a separate array saves */
/* a bit of code space. If an entry is == 0, the timer is not allocated. */
static tmrStatus_t maTmrTimerStatusTable[gTmrTotalTimers_c];

/* Number of Active timers (without low power capability)
   the MCU can not enter low power if numberOfActiveTimers!=0 */
static uint8_t numberOfActiveTimers = 0;
/* Number of low power active timer 
   The MCU can enter in low power if more low power timers are active */
static uint8_t numberOfLowPowerActiveTimers = 0;

#define IncrementActiveTimerNumber(type)  ((type & gTmrLowPowerTimer_c) \
                                          ?(++numberOfLowPowerActiveTimers) \
                                          :(++numberOfActiveTimers) )                                   
#define DecrementActiveTimerNumber(type)  ((type & gTmrLowPowerTimer_c) \
                                          ?(--numberOfLowPowerActiveTimers) \
                                          :(--numberOfActiveTimers) ) 

/* Used to access 16 bit hardware registers. */
static read16bitReg_t mTmrRead16bitReg;

/* No other code should ever post an event to the timer task. */
static tsTaskID_t gTimerTaskID;

/* The previous time in ticks when the counter register was read */
static uint16_t previousTimeInTicks = 0;
/* Flag if the hardware timer is running or not*/
static bool_t timerHardwareIsRunning = FALSE;

/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/* Initialize the timer module. */
void TMR_Init
(
void
)
{
  /* Configure a TPM channel: enable interrupts; set output compare mode. */
  gTPMxCnSC_c = (gTPMxCnSC_IE_c | gTPMxCnSC_MSA_c);
  
  gTimerTaskID = TS_CreateTask(gTsTimerTaskPriority_c, TMR_Task);
}                                       /* TMR_Init() */

/****************************************************************************/

/* Allocate a timer.
* Returns the timer id, or gTmrInvalidTimerID_c if the timer table is full.
*/
tmrTimerID_t TMR_AllocateTimer
(
void
)
{
  index_t i;
  
  for (i = 0; i < NumberOfElements(maTmrTimerTable); ++i) {
    if (!TMR_IsTimerAllocated(i)) {
      TMR_SetTimerStatus(i, mTmrStatusInactive_c);
      return i;
    }
  }
  
  /* Out of timers. Assert. */
#if gTMR_Debug_d
  BeeAppAssert(gAssertCode_NoTimers_c);
#endif
  return gTmrInvalidTimerID_c;
}                                       /* TMR_AllocateTimer() */


/*****************************************************************************/

/* Return true if there are no active timers (without low power capability); 
   return FALSE otherwise. */
bool_t TMR_AreAllTimersOff
(
void
)
{
  return !numberOfActiveTimers;
}                                       /* TMR_AreAllTimersOff() */

/****************************************************************************/

/* Free any timer. Harmless if the timer is already free. */
void TMR_FreeTimer
(
tmrTimerID_t timerID
)
{
  TMR_StopTimer(timerID);
  TMR_MarkTimerFree(timerID);
}                                       /* TMR_FreeTimer() */

/****************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
/* Timer ISR. */
INTERRUPT_KEYWORD void TMR_InterruptHandler(void) {
  __asm {                               /* The HCS08 does not save H on interrupt. */
    PSHH
  }
    
  /* Clearing the overflow flag requires reading it and then writing it. */
  if ( gTPMxCnSC_c & gTPMxCnSC_F_c ) {
    gTPMxCnSC_c &= ~gTPMxCnSC_F_c;
    TS_SendEvent(gTimerTaskID, mTMR_Event_c);
  }
  
  __asm {
    PULH
  }
}                                       /* TMR_InterruptHandler() */
#pragma CODE_SEG DEFAULT
/*****************************************************************************/


/* Return TRUE if the timer is active. */
bool_t TMR_IsTimerActive
(
tmrTimerID_t timerID
)
{
  return TMR_GetTimerStatus(timerID) == mTmrStatusActive_c;
}                                       /* TMR_IsTimerActive() */

/****************************************************************************/

/****************************************************************************
*
* Start a timer (BeeStack or application). When the timer goes off, call
* the callback function in non-interrupt context.
*
*****************************************************************************/
void TMR_StartTimer
(
tmrTimerID_t timerID,                       /* IN: */
tmrTimerType_t timerType,                   /* IN: */
tmrTimeInMilliseconds_t timeInMilliseconds, /* IN: */
void (*pfTimerCallBack)(tmrTimerID_t)       /* IN: */
)
{
  tmrTimerTicks_t intervalInTicks;
  tmrTimerTableEntry_t *currentTimer = &maTmrTimerTable[timerID];
  
  /* Stopping an already stopped timer is harmless. */
  TMR_StopTimer(timerID);
  /* if it's a minute/second timer .intervalInTicks will count down the minutes/seconds
     and .countDown is initialized with ticks for one minute/second */
  
#if gTMR_EnableMinutesSecondsTimers_d
  if (IsMinuteTimer(timerType)) 
  {
     currentTimer->intervalInTicks = (uint16_t)timeInMilliseconds;
     currentTimer->countDown = TmrTicksForOneMinute();
  } 
  else 
  {  
    if (IsSecondTimer(timerType)) 
    {
      currentTimer->intervalInTicks = (uint16_t)timeInMilliseconds;
      currentTimer->countDown = TmrTicksForOneSecond();
    } 
      else 
#endif     /* gTMR_EnableMinutesSecondsTimers_d */
       { 
       /* here, the timer is single shot or interval in milliseconds*/
        intervalInTicks = TmrTicksFromMilliseconds(timeInMilliseconds);
        if (!intervalInTicks) 
           intervalInTicks = 1;
      
        currentTimer->intervalInTicks = intervalInTicks;
        currentTimer->countDown = intervalInTicks;
       }
#if gTMR_EnableMinutesSecondsTimers_d       
  }/* end else... if (timerType == gTmrMinuteTimer_c)... */
#endif  /* gTMR_EnableMinutesSecondsTimers_d */
  
  
  TMR_SetTimerType(timerID, timerType);
  
  currentTimer->pfCallBack = pfTimerCallBack;
  
  /* Enable timer, the timer task will do the rest of the work. */
  TMR_EnableTimer(timerID);
}                                       /* TMR_StartTimer() */

/* Start a low power timer. When the timer goes off, call the callback function */
/* in non-interrupt context. If the timer is running when this function */
/* is called, it will be stopped and restarted. */
/* Start the timer with the following timer types:
   gTmrLowPowerMinuteTimer_c
   gTmrLowPowerSecondTimer_c
   gTmrLowPowerSingleShotMillisTimer_c
   gTmrLowPowerIntervalMillisTimer_c
*/
/* The MCU can enter in low power if there are only active low power timers */
void TMR_StartLowPowerTimer
(
tmrTimerID_t timerId,
tmrTimerType_t timerType,
uint32_t timeIn,
void (*pfTmrCallBack)(tmrTimerID_t)
) 
{
#if(gTMR_EnableLowPowerTimers_d)
  TMR_StartTimer(timerId, timerType | gTmrLowPowerTimer_c, timeIn, pfTmrCallBack);
#else
(void)timerId;
(void)timerType;
(void)timeIn;
(void)pfTmrCallBack;
#endif
}

#if gTMR_EnableMinutesSecondsTimers_d
/* Start a minute timer. */
/* Note: this is a single shot timer. There are no interval minute timers. */
void TMR_StartMinuteTimer
(
tmrTimerID_t timerId, 
tmrTimeInMinutes_t timeInMinutes, 
void (*pfTmrCallBack)(tmrTimerID_t)
)
{
 TMR_StartTimer(timerId, gTmrMinuteTimer_c, timeInMinutes, pfTmrCallBack);
}
#endif /* gTMR_EnableMinutesSecondsTimers_d */


#if gTMR_EnableMinutesSecondsTimers_d  
/* Start a second timer. */
/* Note: this is a single shot timer. There are no interval second timers. */
void TMR_StartSecondTimer
(
tmrTimerID_t timerId, 
tmrTimeInSeconds_t timeInSeconds, 
void (*pfTmrCallBack)(tmrTimerID_t)
) 
{
 TMR_StartTimer(timerId, gTmrSecondTimer_c, timeInSeconds, pfTmrCallBack);
}
#endif /* gTMR_EnableMinutesSecondsTimers_d */
 
/****************************************************************************/

void TMR_StartIntervalTimer
(
tmrTimerID_t timerID,
tmrTimeInMilliseconds_t timeInMilliseconds,
void (*pfTimerCallBack)(tmrTimerID_t)
)
{
  TMR_StartTimer(timerID, gTmrIntervalTimer_c, timeInMilliseconds, pfTimerCallBack);
}

void TMR_StartSingleShotTimer
(
tmrTimerID_t timerID,
tmrTimeInMilliseconds_t timeInMilliseconds,
void (*pfTimerCallBack)(tmrTimerID_t)
)
{
  TMR_StartTimer(timerID, gTmrSingleShotTimer_c, timeInMilliseconds, pfTimerCallBack);
}

/****************************************************************************/

/* Stop a timer. Does not free the timer; does not call the timer's callback
* function.
*
* Harmless if the timer is already inactive.
*/
void TMR_StopTimer
(
tmrTimerID_t timerID
)
{
  tmrStatus_t status;
  uint8_t ccr;
  
  IrqControlLib_BackupIrqStatus(ccr);
  IrqControlLib_DisableAllIrqs();
  status = TMR_GetTimerStatus(timerID);
  if (   (status == mTmrStatusActive_c) ||
         (status == mTmrStatusReady_c)
     ) 
  {
        TMR_SetTimerStatus(timerID, mTmrStatusInactive_c);
        DecrementActiveTimerNumber(TMR_GetTimerType(timerID));
        /* if no sw active timers are enabled, */
        /* call the TMR_Task() to countdown the ticks and stop the hw timer*/
    		if (!numberOfActiveTimers && !numberOfLowPowerActiveTimers) 
    		   TS_SendEvent(gTimerTaskID, mTMR_Event_c);

  }
  IrqControlLib_RestoreIrqStatus(ccr);    
}  
/*****************************************************************************/

/*****************************************************************************
* Timer task. Called by the kernel when the timer ISR posts a timer event.
******************************************************************************/
void TMR_Task
(
event_t events
)
{
  pfTmrCallBack_t pfCallBack;
  uint16_t currentTimeInTicks;
  uint16_t nextInterruptTime;
  tmrTimerStatus_t status;
  uint16_t ticksSinceLastHere;
  index_t timerID;
  uint16_t ticksdiff;   
  uint8_t ccr;
  tmrTimerTableEntry_t *currentTimer;
#if gTMR_EnableMinutesSecondsTimers_d 
  uint8_t stopIt = FALSE; 
#endif /* gTMR_EnableMinutesSecondsTimers_d */
  tmrTimerType_t timerType; 
  (void)events;
  
  IrqControlLib_BackupIrqStatus(ccr);
  IrqControlLib_DisableAllIrqs();  
  TpmReadCNTRegister(currentTimeInTicks);
  
  #ifdef PROCESSOR_MC1323X  
   TpmReadCNTRegister(currentTimeInTicks);
  #endif
  IrqControlLib_RestoreIrqStatus(ccr);
  
  /* calculate difference between current and previous. This scheme works up to 32K of difference */
  ticksSinceLastHere = (uint16_t)((int16_t)currentTimeInTicks - (int16_t)previousTimeInTicks);
  
  /* remember for next time */
  previousTimeInTicks = currentTimeInTicks;
 
    /* Find the shortest active timer. */
  nextInterruptTime = mMaxToCountDown_c;
  
  for (timerID = 0; timerID < NumberOfElements(maTmrTimerTable); ++timerID) 
  {
    /* Pointer to the current timer. Optimize the code size */
    currentTimer = &maTmrTimerTable[timerID];
    
    IrqControlLib_BackupIrqStatus(ccr);
    IrqControlLib_DisableAllIrqs();
    status = TMR_GetTimerStatus(timerID);

    if (status == mTmrStatusReady_c) 
    {
      /* If TMR_StartTimer() has been called for this timer, start it's count */
      /* down as of now. */
      TMR_SetTimerStatus(timerID, mTmrStatusActive_c);            
      IrqControlLib_RestoreIrqStatus(ccr);
    }
    else
    {          
      IrqControlLib_RestoreIrqStatus(ccr);        
      
      /* Process the active timers. This does not includes the Ready timers started in this loop */
      if (status == mTmrStatusActive_c) 
      {      
        /* This timer is active. Decrement it's countdown. If the countdown */
        /* reaches zero, the timer is expired. */
        if (currentTimer->countDown > ticksSinceLastHere) 
        {
         currentTimer->countDown -= ticksSinceLastHere;
        }
        else
        {       
         bool_t callbackCall = TRUE;
         /* Timer countDown expired */
         
         /* Get timer type to detect it*/
         timerType = TMR_GetTimerType(timerID); 
                
         #if gTMR_EnableMinutesSecondsTimers_d   
         /* check if a second/minute expired, load the next one  */
         if (IsMinuteTimer(timerType) || IsSecondTimer(timerType)) 
         {
           /* Check if the minute/second timer expired */
           if(currentTimer->intervalInTicks > 1) 
           {
             --currentTimer->intervalInTicks; 
             /* Load with next minute/second to count down */
             if(IsMinuteTimer(timerType))
             {            
               currentTimer->countDown = TmrTicksForOneMinute();
             }
             else
             {            
               /* else if it's a second timer */
               currentTimer->countDown = TmrTicksForOneSecond();
             }

            callbackCall = FALSE;
           } 
           else 
           {          
             /* Minute/second Timer expired... stop it */
             TMR_StopTimer(timerID);
           }
         } 
         else 
         #endif /* gTMR_EnableMinutesSecondsTimers_d */
         {
           /* If this is an interval millisec. timer, restart it */
           if (timerType & gTmrIntervalTimer_c)
           {          
            currentTimer->countDown = currentTimer->intervalInTicks;
           }
           else
           {
            TMR_StopTimer(timerID);
           }
         }
         
         if(callbackCall)
         {        
           /* This timer has expired. */
           pfCallBack = currentTimer->pfCallBack;
      
           /* Call callback if it is not NULL. This is done after the timer got updated,
            * in case the timer gets stopped or restarted in the callback
            */
            if (pfCallBack) 
            {
             pfCallBack(timerID);
            }
         }
         
        }
      }
      
    } // end else if (status == mTmrStatusReady_c)     
    
    /* Timer is still active (not stopped)? 
     * This test includes the Ready timers started in this loop
     */
     if (TMR_GetTimerStatus(timerID) == mTmrStatusActive_c) 
     {           
        if (nextInterruptTime > currentTimer->countDown) 
        {
          nextInterruptTime = currentTimer->countDown;
        }
     }

  } // end for
  
  
  IrqControlLib_BackupIrqStatus(ccr);
  IrqControlLib_DisableAllIrqs();
  
  /* Update the compare register */
  nextInterruptTime += currentTimeInTicks;
  
  #ifndef PROCESSOR_QE128
   gTPMxCnVL_c = TPMxCnVLvalue(nextInterruptTime);
   gTPMxCnVH_c = TPMxCnVHvalue(nextInterruptTime);
  #endif
  
  /* Check to be sure that the timer was not programmed in the past for different source clocks.
   * The interrupts are now disabled.
   */      	 
  TpmReadCNTRegister(ticksdiff);  
  
  #ifdef PROCESSOR_QE128
   /* Always program minimum 2 ticks ahead on QE128 */ 
   if(((int16_t)nextInterruptTime - (int16_t)ticksdiff) < 2 )
   {
     nextInterruptTime = ticksdiff + 2;
   }
   gTPMxCnVL_c = TPMxCnVLvalue(nextInterruptTime);
   gTPMxCnVH_c = TPMxCnVHvalue(nextInterruptTime);
 
  #else
    
   if((((int16_t)ticksdiff - (int16_t)nextInterruptTime) >= 0) && (!(gTPMxCnSC_c & gTPMxCnSC_F_c)))
   {  
     /* Timer counter passed the comparator value. The Compare Flag was not set */
     /* The compare register was programmed in the past. The next interrupt time has already passed */
     TS_SendEvent(gTimerTaskID, mTMR_Event_c);
   }
   
  #endif 
  
  IrqControlLib_RestoreIrqStatus(ccr);   

  if (!numberOfActiveTimers && !numberOfLowPowerActiveTimers) {
    TpmStopTimerHardware();
    timerHardwareIsRunning = FALSE;
  } else if (!timerHardwareIsRunning) {
    TpmStartTimerHardware();
    timerHardwareIsRunning = TRUE;
  }
}                                       /* TMR_Task() */
/*****************************************************************************/

/* Enable or disable the timer tmrID
   If enable = TRUE timer is active
   Else timer is inactive 
*/
void TMR_EnableTimer(tmrTimerID_t tmrID)
{    
  uint8_t ccr;
  
  IrqControlLib_BackupIrqStatus(ccr);
  IrqControlLib_DisableAllIrqs();
  if (TMR_GetTimerStatus(tmrID) == mTmrStatusInactive_c)
  {      
    IncrementActiveTimerNumber(TMR_GetTimerType(tmrID));    
    TMR_SetTimerStatus(tmrID, mTmrStatusReady_c);
    TS_SendEvent(gTimerTaskID, mTMR_Event_c);
  }  
  IrqControlLib_RestoreIrqStatus(ccr);
}
/*****************************************************************************/

/*
This function is called by Low Power module; Also this function stops the 
harware timer.
Return: time in millisecond that wasn't counted before entering in sleep  
*/
#if (gTMR_EnableLowPowerTimers_d)
uint32_t TMR_NotCountedMillisTimeBeforeSleep(void)
{
  uint16_t currentTimeInTicks;
  
  if (!numberOfLowPowerActiveTimers)
    return 0;
  TpmReadCNTRegister(currentTimeInTicks);
  TpmStopTimerHardware();
  timerHardwareIsRunning = FALSE; 
     
  return  TmrMillisecondsFromTicks((currentTimeInTicks - previousTimeInTicks));

} 
#else
uint32_t TMR_NotCountedMillisTimeBeforeSleep(void)
{
 return 0;
}
#endif /* #if (gTMR_EnableLowPowerTimers_d) */


/*****************************************************************************/
      
/* This function is called by the Low Power Module */
/* each time the MCU wakes up */ 
/* The function make an approximate sync. the active low power timers. */   

#if (gTMR_EnableLowPowerTimers_d)                            
void TMR_SyncLpmTimers(tmrTimeInMilliseconds_t sleepDurationMillisec) 
{
#if gTMR_EnableMinutesSecondsTimers_d
 uint16_t spentSeconds = 0;
 uint16_t spentMinutes = 0;
 uint16_t restTicksForMinutes = 0;
 uint16_t restTicksForSeconds = 0;
#endif /* gTMR_EnableMinutesSecondsTimers_d */ 
 uint32_t durationMillis = sleepDurationMillisec;
 index_t  timerID;
 tmrTimerType_t timerType;
 tmrTimerTableEntry_t *currentTimer;
 
 /* Check if there are low power active timer */
 if (!numberOfLowPowerActiveTimers)
    return;
 
#if gTMR_EnableMinutesSecondsTimers_d           
 /* Calc the units spent in sleep mode - minutes and seconds */
 while(durationMillis >= mOneMinute_c) 
 {
   durationMillis -= mOneMinute_c;
   spentMinutes++;
   if(spentSeconds < ((uint16_t)0xFFFE-60)) 
   {    
    spentSeconds+=60;
   }
      
 }
 /* durationMillis now indicates the rest of milliseconds spent after spentMinutes */
 restTicksForMinutes = TmrTicksFromMilliseconds(durationMillis);
   
 while(durationMillis >= mOneSecond_c) 
 {
   durationMillis -= mOneSecond_c;
   if(spentSeconds < (uint16_t)0xFFFE) 
   {    
    spentSeconds++;
   }
 }
 /* durationMillis now indicates the rest of milliseconds spent after spentSeconds */
 restTicksForSeconds = TmrTicksFromMilliseconds(durationMillis);
#endif /* gTMR_EnableMinutesSecondsTimers_d */

/* For each timer, detect the timer type and count down the spent duration in sleep */  
for (timerID = 0; timerID < NumberOfElements(maTmrTimerTable); ++timerID) 
{

  /* Detect the timer type and count down the spent duration in sleep */
  timerType = TMR_GetTimerType(timerID);
  
  currentTimer = &maTmrTimerTable[timerID];
  
  /* Sync. only the low power timers that are active */
  if ( (TMR_GetTimerStatus(timerID) == mTmrStatusActive_c)
        && (IsLowPowerTimer(timerType)) ) 
  {
#if gTMR_EnableMinutesSecondsTimers_d

         if (IsLowPowerSecondTimer(timerType) || IsLowPowerMinuteTimer(timerType)) 
         {
           bool_t timerExpired = FALSE;
           
           uint16_t spentUnits = spentSeconds; /* seconds or minutes depending on the timer type */
           uint16_t millisecInUnit = mOneSecond_c;          
           uint16_t restTicks = restTicksForSeconds;
           
           if(IsLowPowerMinuteTimer(timerType)) 
           {
            spentUnits = spentMinutes;
            millisecInUnit = mOneMinute_c;
            restTicks = restTicksForMinutes;
           }           
                                 
           /* Timer expired while MCU was in sleep mode??? */
           /* SleepUnits + SleepMilliseconds <> RemainingUnits + RemainingMiliseconds */
           
           /* currentTimer->intervalInTicks is decremented only after the countdown has expired */
           
           if(currentTimer->intervalInTicks < (spentUnits + 1)) 
           {
              /* More units spent in sleep mode than remaining timer units */
              timerExpired = TRUE;
           } 
           else            
           {
             
             if( currentTimer->intervalInTicks  == (spentUnits + 1))
             {            
                /* Same units in sleep mode as remaing timer units */
                if((uint32_t)currentTimer->countDown <= restTicks) 
                {                                
                  /* Remaining milliseconds are grater than the remaining timer ticks */
                  timerExpired = TRUE;
                }
                else 
                {
                 currentTimer->intervalInTicks = 0;
                 currentTimer->countDown -= restTicks;
                }              
             } 
             else 
             {
               /* Less units spent in sleep mode than remaining timer units */
               currentTimer->intervalInTicks -= spentUnits;
               
               
               if(currentTimer->countDown >=  restTicks) 
               {                
                currentTimer->countDown -= restTicks;
               } 
               else 
               {
                 /* Remaining milliseconds are grater than remaining timer ticks */
                 /* Decrement the number of units and recalculate the timer ticks */
                 currentTimer->intervalInTicks--;
                 currentTimer->countDown += (uint16_t)(TmrTicksFromMilliseconds(millisecInUnit) - restTicks);
               }
               
             }                           
           }
           
           if(timerExpired) 
           {
              currentTimer->intervalInTicks = 0;
              currentTimer->countDown = 0;
           }
         }
         /* here the timer is a millisecond one (single shot or interval)*/
         else 
#endif  /* gTMR_EnableMinutesSecondsTimers_d */
         {
           /* how many millisecond counts the timer */
           durationMillis = TmrMillisecondsFromTicks(currentTimer->countDown);
           /* Timer expired when MCU was in sleep mode??? */
           if(durationMillis <= sleepDurationMillisec) 
           {
            currentTimer->countDown = 0;
           } 
           else 
           {
             /* calculate remaining tick to count after wake up */
             durationMillis -= sleepDurationMillisec;
             currentTimer->countDown = TmrTicksFromMilliseconds(durationMillis);
           }

           
         } /* end if (IsLowPowerSecondTimer(timerType) || IsLowPowerMinuteTimer(timerType)) */
        
   }

}/* end for (timerID = 0;.... */

TpmStartTimerHardware();
TpmReadCNTRegister(previousTimeInTicks);

TS_SendEvent(gTimerTaskID, mTMR_Event_c);
}
#else
void TMR_SyncLpmTimers(tmrTimeInMilliseconds_t sleepDurationMillisec) 
{
(void)sleepDurationMillisec;
}
#endif /* #if (gTMR_EnableLowPowerTimers_d) */
/*****************************************************************************
******************************************************************************
* Private functions
******************************************************************************
*****************************************************************************/

static tmrStatus_t TMR_GetTimerStatus
(
tmrTimerID_t timerID
)
{
  return maTmrTimerStatusTable[timerID] & mTimerStatusMask_c;
}

static void TMR_SetTimerStatus
(
tmrTimerID_t timerID, 
tmrStatus_t status
)
{
  maTmrTimerStatusTable[timerID] = (maTmrTimerStatusTable[timerID] & ~mTimerStatusMask_c) | status;
}

/*****************************************************************************/

static tmrTimerType_t TMR_GetTimerType
(
tmrTimerID_t timerID
)
{
  return maTmrTimerStatusTable[timerID] & mTimerType_c;
}

static void TMR_SetTimerType
(
tmrTimerID_t timerID, 
tmrTimerType_t type
)
{
  maTmrTimerStatusTable[timerID] = (maTmrTimerStatusTable[timerID] & ~mTimerType_c) | type;
}

