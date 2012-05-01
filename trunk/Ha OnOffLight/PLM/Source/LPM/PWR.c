/************************************************************************************
* Function to handle the different power down states.
*
*
* (c) Copyright 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifdef PROCESSOR_MC1323X
 #error "*** The current power modes implementation is not supported on MC1323X. Use PWR_MC1323x.c instead"
#endif
/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/
#include "PortConfig.h" 
#include "PWR_Configuration.h"   
#include "PWR_Interface.h"
#include "PWRLib.h"
#include "AppAspInterface.h"
#include "MsgSystem.h"
#include "IrqControlLib.h"
#include "Mc1319xDrv.h"
#include "Mc1319xReg.h"
#include "TMR_Interface.h"
#include "nv_data.h"
#include "powermanager.h"


extern uint8_t gPowerSaveMode;
extern void Phy_InitOnStartup(uint16_t phyRfCalibration, bool_t useDualAntenna);

#ifdef PROCESSOR_QE128
  extern void __near  LowLevelReadClockAsync(zbClock24_t *pRetClk);
#else
  extern void         LowLevelReadClockAsync(zbClock24_t *pRetClk);
#endif

extern void WAIT_6_NOPS( void);
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

#if (gTMR_EnableLowPowerTimers_d) 
/* How many millisecond need to count down to sync. low power timers */
#define MillisToSyncLpmTimers(time) ( (((uint32_t)cPWR_RTITicks - (uint32_t)PWRLib_RTIClockCheck()) \
                                            * (uint32_t)cRTI_OneTickInMillis) + (uint32_t)(time) )
#endif                                               
/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/
static uint32_t mPWR_DozeDurationMs = cPWR_DozeDurationMs;
static bool_t   mPWR_DozeDurationSymbols;


#if (cPWR_DeepSleepMode==31)
  uint8_t PWR_RemainingLoops = 1;
#endif /* if (cPWR_DeepSleepMode==31)  */

#if (gTMR_EnableLowPowerTimers_d) 
static uint32_t notCountedTimeBeforeSleep;
#endif
/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/
uint8_t mLPMFlag = gAllowDeviceToSleep_c;
/* but recommended to be regarded as private */

zbClock24_t lastLPMExitRadioTime;
/************************************************************************************
* Set the NVM write operation semaphore.
*
* This function will set the NVM write operation semaphore so that the idle task wont
* do a NVM write operation unless this is released by calling 
* NVM_ClearCriticalNVMSection.
*
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/

void PWR_AllowDeviceToSleep(void)
{
  uint8_t   ccr;
  IrqControlLib_BackupIrqStatus(ccr);/*To protect from re entrant*/
  IrqControlLib_DisableAllIrqs();
  if( mLPMFlag != 0 ){    
    mLPMFlag--;
  }
  IrqControlLib_RestoreIrqStatus(ccr);
}

/************************************************************************************
* Release the NVM write operation semaphore.
*
* This function will release the NVM write operation semaphore so that the idle task 
* can do a NVM write operation.
*  
* Interface assumptions:
*   None
*   
* Return value:
*   None
*
************************************************************************************/
 
void PWR_DisallowDeviceToSleep(void)
{
  uint8_t   ccr;
  IrqControlLib_BackupIrqStatus(ccr);/*To protect from re entrant*/
  IrqControlLib_DisableAllIrqs();
  mLPMFlag++;
  IrqControlLib_RestoreIrqStatus(ccr);
}

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

bool_t PWR_CheckIfDeviceCanGoToSleep( void ) 
{
  uint8_t   ccr;
  bool_t    returnValue;
  IrqControlLib_BackupIrqStatus(ccr);/*To protect from re entrant*/
  IrqControlLib_DisableAllIrqs();
  returnValue = mLPMFlag == 0 ? TRUE : FALSE;
  IrqControlLib_RestoreIrqStatus(ccr);
  return returnValue; 
}

/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
bool_t PWR_Stop3AndOff(void) {
#if (cPWR_UsePowerDownMode)
  bool_t res;
#ifdef PROCESSOR_QE128
  ICSC1 |=  0x04; /* Set IREFS */   
  while(!(ICSSC & 0x10));   /* Wait for the FLL to lock on the internal clock source */
#endif         
  res = PWRLib_RadioHibernateReq();
  if (res == TRUE) {
    PWRLib_ClockSetup( SelfClk_2MHz);
    IrqControlLib_DisableMC1319xIrq(); 
    PWRLib_SetMCUIOForPowerSavingMode();
    PWRLib_RadioOffReq();                 /* To Reset RADIO. Has to be after PWRLib_SetMCUIOForPowerSavingMode */
    PWRLib_MCUStop3();
  }
  return res;
#else
  return FALSE;
#endif  /* #if (cPWR_UsePowerDownMode) */
}  /* PWR_Stop3AndOff(void) =================================================== */


/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
bool_t PWR_AllOff(void) {
#if (cPWR_UsePowerDownMode)
  bool_t res;
#ifdef  PROCESSOR_QE128
  ICSC1 |=  0x04; /* Set IREFS */   
  while(!(ICSSC & 0x10));   /* Wait for the FLL to lock on the internal clock source */
#endif    
  res = PWRLib_RadioHibernateReq();
  if ( res == TRUE) {
    IrqControlLib_DisableMC1319xIrq(); 
    PWRLib_SetMCUIOForPowerSavingMode();
    PWRLib_RadioOffReq();             /* To Reset RADIO. Has to be after PWRLib_SetMCUIOForPowerSavingMode */
    PWRLib_MCUStop1();
  }
  return res;
#else
  return FALSE;
#endif  /* #if (cPWR_UsePowerDownMode)  */
}  /* PWR_AllOff(void) ========================================================*/


/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
void PWR_RunAgain(void) {
  #if (cPWR_UsePowerDownMode)
    PWRLib_RadioOnReq();
    PWRLib_ResetMCUIOAfterPowerSavingMode();
       /* AbelRegisterSetup(); */
       /*   InitializePhy();   */
      PWRLib_ClockSetup( Normal_16MHz);
      /* We must enable interrupt before initializing Abel, to get the Reset irq.(wakeupisr()), */
      IrqControlLib_EnableMC1319xIrq();
      /* Init 802.15.4 MAC/PHY stack  */
    Phy_InitOnStartup(gHardwareParameters.Abel_HF_Calibration,gHardwareParameters.useDualAntenna);                      
     /* InitializeMac(FALSE);  */		/*   Don't init PIB */
    (void) PWRLib_RadioWakeReq();
  #endif  /* #if (cPWR_UsePowerDownMode) */
}  /* PWR_RunAgain(void) ======================================================*/


/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
bool_t PWR_DeepSleepAllowed(void) {
  #if ( cPWR_UsePowerDownMode)
    #if ( cPWR_DeepSleepMode<30)                               /* Normal running mode*/
      if (PWRLib_GetCurrentZigbeeStackPowerState == StackPS_DeepSleep) {
          if ( PWRLib_GetMacStateReq() == gAspMacStateIdle_c) {
            return TRUE;
          } else {
            return FALSE;
          }
      } else {
        return FALSE;
      }
    #else                                                /* Debug mode */
      return TRUE;
    #endif
  #else                                                      /* Debug mode */
    return TRUE;
  #endif  /* #if (cPWR_UsePowerDownMode)*/
}  /* PWR_DeepSleepAllowed ==================================================*/


/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
bool_t  PWR_SleepAllowed(void) {
  
  #if (cPWR_UsePowerDownMode)
    #if (cPWR_DeepSleepMode<30)                               /* Normal running mode*/
      if (( PWRLib_GetCurrentZigbeeStackPowerState == StackPS_Sleep) ||  \
          ( PWRLib_GetCurrentZigbeeStackPowerState == StackPS_DeepSleep)) {
/*        if ( PWRLib_GetMacStateReq()==gAspMacStateBusy_c) {        This can't be used */

          if (( PWRLib_GetMacStateReq()==gAspMacStateNotEmpty_c) || \
              ( PWRLib_GetMacStateReq()==gAspMacStateIdle_c)) {
            return TRUE;
          } else {
            return FALSE;
          }
      } else {
        return FALSE;
      }
    #else                                                      /* Debug mode */
      return TRUE;
    #endif
  #else                                                      /* Debug mode */
    return TRUE;
  #endif  /* #if (cPWR_UsePowerDownMode) else */
}  /* PWR_SleepAllowed ======================================================*/


/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
PWRLib_WakeupReason_t  PWR_HandleDeepSleep( zbClock24_t  DozeDuration) {
  PWRLib_WakeupReason_t  Res;
  Res.AllBits = 0;
  (void) DozeDuration;
  #if (cPWR_UsePowerDownMode)
  /* Insure that Key Wakeup interrupts are enabled!! Note insert next line of code if you
     always need the keyboard irq enabled while entering sleep.
     KBISC |= cKBI1SC; */
		  /*---------------------------------------------------------------------------*/
    #if (cPWR_DeepSleepMode==0)
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
      Res.AllBits = 0xff | (uint8_t) DozeDuration;  // Last part to avoid unused warning
      /* No code */

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==1)
      #if (cPWR_KBIWakeupEnable==1)
        if (PWR_Stop3AndOff() != FALSE) {
          Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
          PWR_RunAgain();
        }
      #else
        #error "*** ERROR: cPWR_KBIWakeupEnable has to be set to 1"
      #endif

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==2)
      #if (cPWR_KBIWakeupEnable==0)
        PWRLib_RTIClockStart( cPWR_RTITickTime, DozeDuration);
        if (PWR_Stop3AndOff() != FALSE) {
          while ( PWRLib_RTIClockCheck() > 0) {
            PWRLib_MCUStop3();
          }
          PWRLib_RTIClockStop();
          #if (gTMR_EnableLowPowerTimers_d) 
          /* Sync. the low power timers */
          TMR_SyncLpmTimers(MillisToSyncLpmTimers(notCountedTimeBeforeSleep));
          #endif /* #if (gTMR_EnableLowPowerTimers_d)  */
          Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
          Res.Bits.DeepSleepTimeout = 1;
          PWR_RunAgain();
          cPWR_DeepSleepWakeupStackProc;          /* User function called only on timeout */
        }
      #else
        #error "*** ERROR: cPWR_KBIWakeupEnable has to be set to 0"
      #endif

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==3)
      #if (cPWR_KBIWakeupEnable)
      
        PWRLib_MCU_WakeupReason.Bits.FromKBI = FALSE; // Clear any prior KBI wakeup flag
      
        PWRLib_RTIClockStart( cPWR_RTITickTime, DozeDuration);
        if (PWR_Stop3AndOff() != FALSE) {
          while ((PWRLib_MCU_WakeupReason.Bits.FromKBI == 0 ) && ( PWRLib_RTIClockCheck() > 0)) {
            (void) PWRLib_MCUStop3();
            
          }
          PWRLib_RTIClockStop();
          #if (gTMR_EnableLowPowerTimers_d) 
          /* Sync. the low power timers */
          TMR_SyncLpmTimers(MillisToSyncLpmTimers(notCountedTimeBeforeSleep));
          #endif /* #if (gTMR_EnableLowPowerTimers_d)  */
          Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
          PWR_RunAgain();
          if ( PWRLib_RTIClockCheck() == 0) {
            Res.Bits.DeepSleepTimeout = 1;
            cPWR_DeepSleepWakeupStackProc;                            /* User function called only on timeout */
          }
        }
      #else
        #error "*** ERROR: cPWR_KBIWakeupEnable has to be set to 1"
      #endif

   //---------------------------------------------------------------------------
 /* Mode 4: Radio in hibernate, MCU in STOP3, RTI int clock 1024 mS wakeup, KBI enabled */ 
    #elif (cPWR_DeepSleepMode==4) 
  
        PWRLib_MCU_WakeupReason.Bits.FromKBI = FALSE; // Clear any prior KBI wakeup flag
        
        #ifdef PROCESSOR_HCS08
         PWRLib_ICG_Mode(SCM, LOW_CLOCK); // Put MCU into Self-Clocked Mode (SCM)
        #endif
        #ifdef PROCESSOR_QE128
         ICSC1 |=  0x04; /* Set IREFS */   
         while(!(ICSSC & 0x10));   /* Wait for the FLL to lock on the internal clock source */
        #endif
        
        if (PWRLib_RadioHibernateReq() != FALSE)
        {
          PWRLib_SetMCUIOForPowerSavingMode();
          PWRLib_RTIClockStart( cPWR_RTITickTime, DozeDuration);
          while (( PWRLib_MCU_WakeupReason.Bits.FromKBI == 0) && ( PWRLib_RTIClockCheck() > 0))
          {
            (void) PWRLib_MCUStop3();
          }
          PWRLib_RTIClockStop();
          /* Sync. the low power timers */
          #if (gTMR_EnableLowPowerTimers_d) 
          TMR_SyncLpmTimers(MillisToSyncLpmTimers(notCountedTimeBeforeSleep));
          #endif /*#if (gTMR_EnableLowPowerTimers_d) */
          Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
          
          // Wake radio
          MC1319xDrv_AttEnable();                                            /* Set att. pin high so we do not waste any power..*/
          WAIT_6_NOPS();
          WAIT_6_NOPS();
          MC1319xDrv_AttDisable();                                            /* Set att. pin high so we do not waste any power..*/
          // Wait for attn irq to arrive...
          while ( PowerManager_GetPowerMode() != gSeqPowerModeAwake_c){}
          /* At this point the MCU Clock has been restored by the WakeUp ISR (triggered by ATTN) */ 
          mRADIO_SetStatus( RADIO_Idle);
          PWRLib_ResetMCUIOAfterPowerSavingMode();
          
          if ( PWRLib_RTIClockCheck() == 0)
          {
            Res.Bits.DeepSleepTimeout = 1;
            cPWR_DeepSleepWakeupStackProc; // User function called only on timeout
          }
        }
        else  // Didn't make it into hibernate; restore clock
	        PWRLib_ClockSetup( Normal_16MHz); 
              
     //---------------------------------------------------------------------------
 /* Mode 5: Radio in acoma/doze, clockout enabled, MCU in STOP3, RTI on ext clock 1024 mS wakeup */
 /*         KBI enabled */
 /* NOTE: must select RTI on external clock via cPWR_RTIFromExternalClock in PWR_Config.h, else int clock */
 /* NOTE: MCU's internal RTI clock will NOT run in DEBUG (BDM) mode; therefore select ext clock for this */
 /* NOTE: Radio clockout will be changed to 32.786 kHz to make RTI timeout same as with internal MCU clock */
 
    #elif (cPWR_DeepSleepMode==5)
    
        PWRLib_MCU_WakeupReason.Bits.FromKBI = FALSE; // Clear any prior KBI wakeup flag
        
        #ifdef PROCESSOR_HCS08 
         PWRLib_ICG_Mode(SCM, LOW_CLOCK); // Put MCU into Self-Clocked Mode (SCM)
        #endif 
        #ifdef PROCESSOR_QE128 
         ICSC1 |=  0x04; /* Set IREFS */   
         while(!(ICSSC & 0x10));   /* Wait for the FLL to lock on the internal clock source */
        #endif
        
        /* On the QE128 processor the RTI division on the external clock source is 2 times bigger than on HCS08 */
        #ifdef PROCESSOR_HCS08 
         MC1319xDrv_WriteSpi(ABEL_regA, RADIO_CLKO_32_78KHZ); // Change RTI clock source to give same timeout as internal clock
        #endif 
        #ifdef PROCESSOR_QE128 
         MC1319xDrv_WriteSpi(ABEL_regA, RADIO_CLKO_62_50KHZ); // Change RTI clock source to give same timeout as internal clock
        #endif
        
        if (PWRLib_RadioAcomaDozeReq(TRUE) != FALSE) 	// Put radio into acoma/doze, clockout enabled
        {	
            PWRLib_SetMCUIOForPowerSavingMode();
            PWRLib_RTIClockStart( cPWR_RTITickTime, DozeDuration);  // Setup and start the RTI
            while (( PWRLib_MCU_WakeupReason.Bits.FromKBI == 0) && ( PWRLib_RTIClockCheck() > 0))
            {
              (void) PWRLib_MCUStop3(); // STOP3 loop
            }
          
          PWRLib_RTIClockStop();
          /* Sync. the low power timers */
          #if (gTMR_EnableLowPowerTimers_d) 
          TMR_SyncLpmTimers(MillisToSyncLpmTimers(notCountedTimeBeforeSleep));
          #endif /* #if (gTMR_EnableLowPowerTimers_d)  */
         Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
         
         // Make attn pulse instead of calling AbelWakeUp() in MAC/PHY
         // This is done because we have clk0 running (TRUE on the ..Dozereq function call)
         // If this is changed to FALSE, call AbelWakeUp() instead.
         MC1319xDrv_AttEnable();                                            /* Set att. pin high so we do not waste any power..*/
         WAIT_6_NOPS();
         WAIT_6_NOPS();
         MC1319xDrv_AttDisable();                                            /* Set att. pin high so we do not waste any power..*/
         // Wait for attn irq to arrive...
         while ( PowerManager_GetPowerMode() != gSeqPowerModeAwake_c){}
         /* At this point the MCU Clock has been restored by the WakeUp ISR (triggered by ATTN) */ 
         mRADIO_SetStatus( RADIO_Idle);
          PWRLib_ResetMCUIOAfterPowerSavingMode(); 
         if ( PWRLib_RTIClockCheck() == 0)
          {
            Res.Bits.DeepSleepTimeout = 1;
            cPWR_DeepSleepWakeupStackProc; // User function called only on timeout
          }
        } // PWRLib_RadioAcomaDozeReq
        else
	         PWRLib_ClockSetup( Normal_16MHz); 

    /*---------------------------------------------------------------------------*/
    /* Mode 6:   Radio in doze mode, supplying 62.5KHz to MCU, MCU in STOP3 */
    /*           Radio wakeup after  cPWR_DozeDurationMs */
    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==6)
      
      {
              
       #if (gTMR_EnableLowPowerTimers_d)  
        zbClock24_t currentClock; // only used for low power timers sync
       #endif

       if(!mPWR_DozeDurationSymbols)
       {               
        /* Calculate doze time based on the Radio Event Time frequency - 62.5 KHz*/
        /* The DozeDuration value is computed by the preprocessor.*/
        DozeDuration = (zbClock24_t)(((zbClock24_t)mPWR_DozeDurationMs * 625) / 10) & (zbClock24_t)0xFFFFFF;
       }
       else 
       {
        DozeDuration = mPWR_DozeDurationMs;
       }
       
       
       PWRLib_MCU_WakeupReason.Bits.FromKBI = FALSE; // Clear any prior KBI wakeup flag
       
       #if (gTMR_EnableLowPowerTimers_d)  
        // Get current clock from ABEL. Only needed for low power timers sync
        LowLevelReadClockAsync(&currentClock);
       #endif 
       
       #ifdef PROCESSOR_HCS08        
          PWRLib_ICG_Mode(SCM, LOW_CLOCK); // Put MCU into Self-Clocked Mode (SCM)
       #endif   
       #ifdef PROCESSOR_QE128        
          ICSC1 |=  0x04; /* Set IREFS */   
          while(!(ICSSC & 0x10));   /* Wait for the FLL to lock on the internal clock source */
       #endif
  
       /* Request Radio Doze with clock output */      
       if(Asp_DozeReq(&DozeDuration, TRUE) == 0) 
       {
       
        mRADIO_SetStatus(RADIO_Doze);
        
        PWRLib_SetMCUIOForPowerSavingMode();
        
        (void)PWRLib_MCUStop3(); // STOP3
        
      
        Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
        
        if(PowerManager_GetPowerMode() != gSeqPowerModeAwake_c) 
        {        
         // Attn pulse in case MCU wakes before doze expiration time */
         MC1319xDrv_AttEnable();                                            /* Set att. pin high so we do not waste any power.*/
         WAIT_6_NOPS();
         WAIT_6_NOPS();
         MC1319xDrv_AttDisable();                                           /* Set att. pin high so we do not waste any power.*/
         // Wait for attn irq to arrive...
         while ( PowerManager_GetPowerMode() != gSeqPowerModeAwake_c){}
        }
        else 
        {
         Res.Bits.DeepSleepTimeout = 1;     /* Sleep timeout ran out */
         Res.Bits.FromTimer = 1;            /* Wakeup by radio timer */
        }                         
        
        /* At this point the MCU Clock has been restored by the WakeUp ISR. 
         * This was triggered either by the doze finish event or by ATTN.
         */
        
        // Get current clock from ABEL
        LowLevelReadClockAsync(&lastLPMExitRadioTime);
        /* Sync. the low power timers */
        #if (gTMR_EnableLowPowerTimers_d)           
          DozeDuration = (zbClock24_t)((lastLPMExitRadioTime - currentClock) & 0xFFFFFF);
          // Transform doze time in ms
          DozeDuration = (zbClock24_t)(((uint32_t)DozeDuration * 10) / 625);                
             
          TMR_SyncLpmTimers(DozeDuration + notCountedTimeBeforeSleep);
        #endif /* #if (gTMR_EnableLowPowerTimers_d)  */
        
        mRADIO_SetStatus(RADIO_Idle);
        PWRLib_ResetMCUIOAfterPowerSavingMode(); 
        
        if(Res.Bits.DeepSleepTimeout == 1) 
        {
          cPWR_DeepSleepWakeupStackProc; // User function called only on timeout
        }
       
       }
       else
       {
         // Didn't make it into doze mode; restore clock 
         PWRLib_ClockSetup( Normal_16MHz);
       }
       
      }
    #elif (cPWR_DeepSleepMode==30)
        #error "*** ERROR: cPWR_DeepSleepMode == 30 not allowed"

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==31)
      #if (cPWR_KBIWakeupEnable)
        if ( PWR_RemainingLoops > 0) {
          PWR_RemainingLoops--;
          PWRLib_RTIClockStart( cSRTISC_Int1024ms, DozeDuration);
          if (PWR_Stop3AndOff() != FALSE) {
            while (( PWRLib_MCU_WakeupReason.Bits.FromKBI == 0) && ( PWRLib_RTIClockCheck() > 0)) {
              (void) PWRLib_MCUStop3();
            }
            PWRLib_RTIClockStop();
            /* Sync. the low power timers */
            #if (gTMR_EnableLowPowerTimers_d) 
            TMR_SyncLpmTimers(MillisToSyncLpmTimers(notCountedTimeBeforeSleep));
            #endif /* #if (gTMR_EnableLowPowerTimers_d)  */
            Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
            PWR_RunAgain();
            if ( PWRLib_RTIClockCheck() == 0) {
              Res.Bits.DeepSleepTimeout = 1;
              cPWR_DeepSleepWakeupStackProc;                            /* User function called only on timeout*/
            }
          }
        }
      #else
        #error "*** ERROR: cPWR_KBIWakeupEnable has to be set to 1"
      #endif

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==33)
      PWRLib_SetMCUIOForPowerSavingMode();
      PWRLib_MCUWait();
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==34)
      PWRLib_SetMCUIOForPowerSavingMode();
      PWRLib_MCUStop3();
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==35)
      PWRLib_SetMCUIOForPowerSavingMode();
      PWRLib_MCUStop2();
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==36)
      PWRLib_SetMCUIOForPowerSavingMode();
      PWRLib_MCUStop1();
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==37)
    /*IrqControlLib_DisableAllIrqs(); */
      (void) PWRLib_RadioDozeReq(0x0ffff0, FALSE);
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==38)
    /*IrqControlLib_DisableAllIrqs(); */
      (void) PWRLib_RadioAcomaDozeReq( FALSE);
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==39)
   /* IrqControlLib_DisableAllIrqs(); */
      PWRLib_RadioHibernateReq();
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;

   /*---------------------------------------------------------------------------*/
    #elif (cPWR_DeepSleepMode==40)
   /* IrqControlLib_DisableAllIrqs(); */
      PWRLib_RadioHibernateReq();
   /* IrqControlLib_DisableAllIrqs(); */
      PWRLib_RadioOffReq();
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
   /*---------------------------------------------------------------------------*/
    #else
      #error "*** ERROR: Not a valid cPWR_DeepSleepMode chosen"
    #endif
    PWRLib_MCU_WakeupReason.AllBits = 0;
    return Res;
  #else  /* #if (cPWR_UsePowerDownMode) else */
    Res.AllBits = 0xff | (uint8_t) DozeDuration;			/* Last part to avoid unused warning*/
    PWRLib_MCU_WakeupReason.AllBits = 0;
    return Res;          /*(PWRLib_WakeupReason_t) DozeDuration;*/
  #endif  /* #if (cPWR_UsePowerDownMode) end*/
}  /* PWR_HandleDeepSleep =====================================================*/


/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
PWRLib_WakeupReason_t  PWR_HandleSleep( zbClock24_t DozeDuration) {
  PWRLib_WakeupReason_t   Res;
  bool_t                  ReturnValue;
  uint8_t                 i =10;           /* ~10us ATTN pulse @ 62.5KHz */

  Res.AllBits = 0;
  #if (cPWR_UsePowerDownMode)
  	 /* check whether timer is enabled ?		*/
  if (TPM1SC != 0) {
  /* Insure that Key Wakeup interrupts are enabled!! Note insert next line of code if you
   always need the keyboard irq enabled while entering sleep.
   KBISC |= cKBI1SC;
  */ 
    /*---------------------------------------------------------------------------*/
    #if (cPWR_SleepMode==0)
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
      PWRLib_MCU_WakeupReason.AllBits = 0;
      ReturnValue = (bool_t) DozeDuration;     /* Dummy line to remove warning */
      return Res;

    /*---------------------------------------------------------------------------*/
    #elif (cPWR_SleepMode==1)
        if (DozeDuration == 0) {
        	ReturnValue = PWRLib_RadioAcomaDozeReq(TRUE);
        } else {
        	ReturnValue = PWRLib_RadioDozeReq( DozeDuration, TRUE);
        }
      Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
      if ( ReturnValue) {
        PWRLib_MCUWait();
        Res.AllBits = PWRLib_MCU_WakeupReason.AllBits;
        Res.Bits.SleepTimeout = 1;
        /* Make attn pulse instead of calling AbelWakeUp() 
         This is done because we have clk0 running (true on the ..Dozereq)
         If this is changed to FALSE, call AbelWakeUp instead. */
        MC1319xDrv_AttEnable();
        while(i) { i--;}
        MC1319xDrv_AttDisable();
        /* Wait for attn irq to arrive... */
        while ( gPowerSaveMode != 0){
        }
      }
      PWRLib_MCU_WakeupReason.AllBits = 0;
      return Res;
    /*---------------------------------------------------------------------------*/
    #else
      #error "*** ERROR: Not a valid cPWR_SleepMode chosen"
    #endif
  } else {
    
    return Res;
  }
  #else  /* #if (cPWR_UsePowerDownMode) else */
    Res.AllBits = 0xff | (uint8_t) DozeDuration | ReturnValue;  // Last part to avoid unused warning
    return Res;
  #endif  /* #if (cPWR_UsePowerDownMode) end */
}  /* PWR_HandleSleep =========================================================*/


/************************************************************************************
*************************************************************************************
* Very Public functions, recommended for general use
*************************************************************************************
************************************************************************************/

/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
PWRLib_WakeupReason_t PWR_CheckForAndEnterNewPowerState(PWR_CheckForAndEnterNewPowerState_t NewPowerState, zbClock24_t DozeDuration) {
  PWRLib_WakeupReason_t    ReturnValue;
  ReturnValue.AllBits = 0;

  #if (cPWR_UsePowerDownMode)
    mSETPIN_CHKPWRSTATE_MODE;

    if ( NewPowerState == PWR_Run) {
      /* ReturnValue = 0; */
    }
    else if ( NewPowerState == PWR_OFF) {
      while ( PWR_AllOff() == FALSE)  ;    /* Never returns */
    }
    else if ( NewPowerState == PWR_Reset) {
      PWRLib_Reset();                      /* Never returns	 */
    }
     
    else if (( NewPowerState == PWR_DeepSleep) && PWR_DeepSleepAllowed()) {
      ReturnValue = PWR_HandleDeepSleep( DozeDuration);
    } 
    else if (( NewPowerState == PWR_Sleep) && PWR_SleepAllowed()) {
      ReturnValue = PWR_HandleSleep( DozeDuration);
    }
    else {
      /* ReturnValue = FALSE; */
    }
    /*--- Come here after sleeping or ... */
    mMCU_SetStatus( MCU_Running);
    mRESETPIN_CHKPWRSTATE_MODE
    PWRLib_MCU_WakeupReason.AllBits = 0;                     /* Clear wakeup reason */
  #else
    ReturnValue.AllBits = 0xff | (NewPowerState > 0) | (DozeDuration > 0); 
                              /* To remove warning for variabels in functioncall */
  #endif  /* #if (cPWR_UsePowerDownMode) */

  return ReturnValue;
}   /* PWR_CheckForAndEnterNewPowerState ============================================*/


/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
PWRLib_LVD_VoltageLevel_t   PWRLib_LVD_ReportLevel(void) {
  PWRLib_LVD_VoltageLevel_t   Level;

  #if ((cPWR_LVD_Enable == 0) || (cPWR_LVD_Enable == 3))
    Level = PWR_NODEPOWER_LEVEL_100;
  #elif (cPWR_LVD_Enable==1)
    Level = PWRLib_LVD_CollectLevel();
  #elif (cPWR_LVD_Enable==2)
    Level = PWRLib_LVD_SavedLevel;
  #else
    #error "*** ERROR: Illegal value for cPWR_LVD_Enable"
  #endif /* #if (cPWR_LVD_Enable) */
  return Level;
}  /* PWRLib_LVD_ReportLevel ================================================*/


/*****************************************************************************/
/* Please see in PWR.h for description                                       */
void PWR_CheckForAndEnterNewPowerState_Init(void) {
#if (cPWR_UsePowerDownMode)

  PWRLib_Init();

#endif  /* #if (cPWR_UsePowerDownMode) */
}   /* PWR_CheckForAndEnterNewPowerState_Init ==================================*/

/********************************************************************************/
PWRLib_WakeupReason_t PWR_EnterLowPower(void)
{
		 
  uint8_t ccr;
  PWRLib_WakeupReason_t ReturnValue;  
  ReturnValue.AllBits = 0;
  
  
  if (PWRLib_LVD_ReportLevel() == PWR_NODEPOWER_LEVEL_CRITICAL) {
    /* Voltage <= 1.8V so enter power-off state - to disable false Tx'ing(void)*/
    ReturnValue = PWR_CheckForAndEnterNewPowerState( PWR_OFF, 0);
  }
  IrqControlLib_BackupIrqStatus(ccr);    
  IrqControlLib_DisableAllIrqs();
  if (TS_PendingEvents() == FALSE)
  {		 
    PWRLib_SetCurrentZigbeeStackPowerState(StackPS_DeepSleep);
    if (TMR_AreAllTimersOff())  /*No timer running*/
    {			
      /* if Low Power Capability is enabled*/
      #if (gTMR_EnableLowPowerTimers_d)   
       /* if more low power timers are running, stop the hardware timer
        and save the spend time in millisecond that wasn't counted.
        */
       notCountedTimeBeforeSleep = TMR_NotCountedMillisTimeBeforeSleep();
      #endif /* #if (gTMR_EnableLowPowerTimers_d)  */
    
      ReturnValue = PWR_CheckForAndEnterNewPowerState ( PWR_DeepSleep, cPWR_RTITicks);
    }else /*timers are running*/
    { 	 
     ReturnValue = PWR_CheckForAndEnterNewPowerState ( PWR_Sleep, 0);
    }
  } /* enable irq's if there is pending evens */
  IrqControlLib_RestoreIrqStatus(ccr);
  
  return ReturnValue;
}

/********************************************************************************/
/* Please see in PWR_Interface.h for description                                          */
void PWR_SetDozeTimeMs(uint32_t dozeTimeMs)
{
 if(dozeTimeMs == 0) 
 {
  return;
 }
 mPWR_DozeDurationMs = dozeTimeMs;
 mPWR_DozeDurationSymbols = FALSE;
}
/********************************************************************************/

/********************************************************************************/
/* Please see in PWR_Interface.h for description                                          */
void PWR_SetDozeTimeSymbols(uint32_t dozeTimeSym)
{
 if(dozeTimeSym == 0) 
 {
  return;
 }
 mPWR_DozeDurationMs = dozeTimeSym;
 mPWR_DozeDurationSymbols = TRUE; 
}
/********************************************************************************/
 
/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private Debug stuff
*******************************************************************************
******************************************************************************/
