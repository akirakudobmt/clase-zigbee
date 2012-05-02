/******************************************************************************
* Functions to handle the different power states.
*
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*******************************************************************************/


/******************************************************************************
*******************************************************************************
* Includes
*******************************************************************************
*******************************************************************************/
#include "PublicConst.h"
#include "PortConfig.h"  
#include "AppAspInterface.h"
#include "MsgSystem.h"
#include "IoConfig.h"
#ifndef PROCESSOR_MC1323X
 #include "Mc1319xDrv.h"           /* For RADIO spi driver */
 #include "Mc1319xReg.h"           /* For RADIO registers */
#endif
#include "NV_Data.h"              /* For RADIO trim value */
//#include "hwdrv_hcs08.h"        /* For RADIO register init */
#include "PWR_Configuration.h" 
#include "PWRLib.h"
#include "IrqControlLib.h"
/******************************************************************************
*******************************************************************************
* external declarations
*******************************************************************************
*******************************************************************************/

extern void FLib_MemCpyReverseOrder
  (
  void *pDst, /* Destination buffer */
  void *pSrc, /* Source buffer  */
  uint8_t n   /* Byte count */
  );

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
*******************************************************************************/

#if (cPWR_UsePowerDownMode)
  /*--- Variables for saving the port settings  --- */
  #if (cPWR_SetupIOWhenInPD >= 1)
    static uint8_t            SavedPTAD ;
    static uint8_t            SavedPTBD ;
    static uint8_t            SavedPTCD ;
    static uint8_t            SavedPTDD ;
   #ifndef PROCESSOR_MC1323X  
    static uint8_t            SavedPTED ;
    static uint8_t            SavedPTFD ;
    static uint8_t            SavedPTGD ;
   #endif 
  #endif   /* (cPWR_SetupIOWhenInPD >= 1) */
  #if (cPWR_SetupIOWhenInPD == 1)
    static uint8_t            SavedPTADD;
    static uint8_t            SavedPTBDD;
    static uint8_t            SavedPTCDD;
    static uint8_t            SavedPTDDD;
   #ifndef PROCESSOR_MC1323X  
    static uint8_t            SavedPTEDD;
    static uint8_t            SavedPTFDD;
    static uint8_t            SavedPTGDD;
   #endif 
    static uint8_t            SavedPTAPE;
    static uint8_t            SavedPTBPE;
    static uint8_t            SavedPTCPE;
    static uint8_t            SavedPTDPE;
   #ifndef PROCESSOR_MC1323X  
    static uint8_t            SavedPTEPE;
    static uint8_t            SavedPTFPE;
    static uint8_t            SavedPTGPE;
   #endif  
  #endif   /* (cPWR_SetupIOWhenInPD == 1) */

  #if (cPWR_UseDebugOutputs)
    static uint8_t            SavedDebugPTBD;
    static uint8_t            SavedDebugPTCD;
    static uint8_t            SavedDebugPTDD;
  #endif  /* #if (cPWR_UseDebugOutputs) */


  /*--- Variables for saving clock mode ---*/
  static PWRLib_ICGMode_t         PWRLib_ICGMode;

  /*--- RTI variables ---*/
  static uint32_t                 PWRLib_RTIRemainingTicks;
  
  
#endif /* #if (cPWR_UsePowerDownMode) */

  /*--- For Reset function ---*/
  static const uint16_t     illegal_opcode = 0x9e62;                /* An HCS08 illegal instruction. Used to force an illegal */
                                                                    /* instruction reset */
  const uint16_t * const    illegal_opcode_ptr = &illegal_opcode;   /* A pointer to the address of illegal opcode */


/*--- For LVD function ---*/ 
#if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2))
  #if (cPWR_LVD_Enable == 2)
    uint16_t                   PWRLib_LVD_CollectCounter;
  #endif  /* #if (cPWR_LVD_Enable == 2) */
  uint16_t                     PWRLib_LVD_L2Counter;
  PWRLib_LVD_VoltageLevel_t    PWRLib_LVD_SavedLevel;
#endif /* #if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2)) */

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
*******************************************************************************/
  /*--- Chip status ---*/
  #if (cPWR_UseMCUStatus)
    PWRLib_MCUStatus_t            PWRLib_MCUStatus;
  #endif  /* #if (cPWR_UseMCUStatus) */

  #if (cPWR_UseRADIOStatus)
    PWRLib_RADIOStatus_t          PWRLib_RADIOStatus;
  #endif  /* #if (cPWR_UseRADIOStatus) */

  /*--- Zigbee STACK status ---*/ 
  PWRLib_StackPS_t                PWRLib_StackPS;

  volatile PWRLib_WakeupReason_t       PWRLib_MCU_WakeupReason   cPlaceInZeroPageSegment;


/*---------------------------------------------------------------------------*/
/*--- Only include the rest if needed / enabled                           ---*/
#if (cPWR_UsePowerDownMode==1)


/******************************************************************************
*******************************************************************************
* Private definitions
*******************************************************************************
*******************************************************************************/
#ifndef PROCESSOR_MC1323X
  /* SCM filter values for ~2 MHz bus clock (lowest possible value) */
  #define cICGFLTH_VALUE_2MHZ             0x00
  #define cICGFLTL_VALUE_2MHZ             0x11 /* 0x10 */
  #define mSCM_2MHZ_DEFAULT_ICG_FILTER    ICGFLTL = cICGFLTL_VALUE_2MHZ; \
                                          ICGFLTH = cICGFLTH_VALUE_2MHZ;

  /* SCM filter values for ~4 MHz bus clock */
  #define cICGFLTH_VALUE_DEFAULT          0x00
  #define cICGFLTL_VALUE_DEFAULT          0xC0
  #define mSCM_DEFAULT_ICG_FILTER         ICGFLTL = cICGFLTL_VALUE_DEFAULT; \
                                          ICGFLTH = cICGFLTH_VALUE_DEFAULT;

  /* SCM filter values for ~8 MHz bus clock */
  #define cICGFLTH_VALUE_8MHZ             0x02
  #define cICGFLTL_VALUE_8MHZ             0x40
  #define mSCM_8MHZ_DEFAULT_ICG_FILTER    ICGFLTL = cICGFLTL_VALUE_8MHZ; \
                                          ICGFLTH = cICGFLTH_VALUE_8MHZ;

  /* SCM filter values for ~16 MHz bus clock */
  #define cICGFLTH_VALUE_16MHZ            0x06
  #define cICGFLTL_VALUE_16MHZ            0x40
  #define mSCM_16MHZ_DEFAULT_ICG_FILTER   ICGFLTL = cICGFLTL_VALUE_16MHZ; \
                                          ICGFLTH = cICGFLTH_VALUE_16MHZ;

  /* SCM filter values for ~20 MHz bus clock */
  #define cICGFLTH_VALUE_20MHZ            0x08
  #define cICGFLTL_VALUE_20MHZ            0x40
  #define mSCM_20MHZ_DEFAULT_ICG_FILTER   ICGFLTL = cICGFLTL_VALUE_20MHZ; \
                                          ICGFLTH = cICGFLTH_VALUE_20MHZ;

  /* Some ICG module bit positions */
  #define cICG_IRQ_PENDING                0x01
  #define cICG_FLL_LOCKED                 0x08
#endif /* PROCESSOR_MC1323X */

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
*******************************************************************************/

/******************************************************************************
* Description : Used for waiting 6-7 clock periods
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
#ifndef PROCESSOR_MC1323X
void WAIT_6_NOPS( void)  {
  __asm nop;
  __asm nop;
  __asm nop;
  __asm nop;
  __asm nop;
}
#endif

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
*******************************************************************************/

//------------------------------------------------------------------------------ 
//------------------------------------------------------------------------------
// MCU interface functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------



/*******************************************************************************/
/* Please see in PWRLib.h for description                                    */
void PWRLib_SaveDebugIOState(void) {
  #if (cPWR_UseDebugOutputs)
    SavedDebugPTBD = PTBD;
    SavedDebugPTCD = PTCD;
    SavedDebugPTDD = PTDD;
  #endif  /*#if (cPWR_UseDebugOutputs) */
}  /* PWRLib_SaveDebugIOState ===*/


/*******************************************************************************/
/* Please see in PWRLib.h for description                                    */
void PWRLib_RestoreDebugIOState(void) {
  #if (cPWR_UseDebugOutputs)
    PTBD = (PTBD & (~mPWRLib_DEBUG_MASK_PTBD)) | (SavedDebugPTBD & mPWRLib_DEBUG_MASK_PTBD);
    PTCD = (PTCD & (~mPWRLib_DEBUG_MASK_PTCD)) | (SavedDebugPTCD & mPWRLib_DEBUG_MASK_PTCD);
    PTDD = (PTDD & (~mPWRLib_DEBUG_MASK_PTDD)) | (SavedDebugPTDD & mPWRLib_DEBUG_MASK_PTDD);
  #endif  /*#if (cPWR_UseDebugOutputs) */
}  /* PWRLib_RestoreDebugIOState =====*/


/*******************************************************************************/
/* Please see in PWRLib.h for description                                    */
void PWRLib_SetMCUIOForPowerSavingMode(void) {
  #if (cPWR_SetupIOWhenInPD > 0)
    PWRLib_SaveDebugIOState();
    #if (cPWR_SetupIOWhenInPD >= 1)
      SavedPTAD  = PTAD ;
      SavedPTBD  = PTBD ;
      SavedPTCD  = PTCD ;
      SavedPTDD  = PTDD ;
     #ifndef PROCESSOR_MC1323X 
      SavedPTED  = PTED ;
      SavedPTFD  = PTFD ;
      SavedPTGD  = PTGD ;
     #endif 
    #endif
    #if (cPWR_SetupIOWhenInPD == 1)
      SavedPTADD = PTADD;
      SavedPTBDD = PTBDD;
      SavedPTCDD = PTCDD;
      SavedPTDDD = PTDDD;
     #ifndef PROCESSOR_MC1323X 
      SavedPTEDD = PTEDD;
      SavedPTFDD = PTFDD;
      SavedPTGDD = PTGDD;
     #endif 
      SavedPTAPE = PTAPE;
      SavedPTBPE = PTBPE;
      SavedPTCPE = PTCPE;
      SavedPTDPE = PTDPE;
     #ifndef PROCESSOR_MC1323X 
      SavedPTEPE = PTEPE;
      SavedPTFPE = PTFPE;
      SavedPTGPE = PTGPE;
     #endif 
    #endif
    mPWRLib_SETUP_PORT_A;
    mPWRLib_SETUP_PORT_B;
    mPWRLib_SETUP_PORT_C;
    mPWRLib_SETUP_PORT_D;
   #ifndef PROCESSOR_MC1323X 
    mPWRLib_SETUP_PORT_E;
    mPWRLib_SETUP_PORT_F;
    mPWRLib_SETUP_PORT_G;
   #endif 
    PWRLib_RestoreDebugIOState();
    mPWRLib_SPIPowerSaveMode;
  #endif  /* #if (cPWR_SetupIOWhenInPD) */
}   /* PWRLib_SetMCUIOForPowerSavingMode =====*/


/******************************************************************************/
/* Please see in PWRLib.h for description */
void PWRLib_ResetMCUIOAfterPowerSavingMode(void) {
  #if (cPWR_SetupIOWhenInPD > 0)
    PWRLib_SaveDebugIOState();
    #if (cPWR_SetupIOWhenInPD >= 1)
      PTAD  = SavedPTAD ;
      PTBD  = SavedPTBD ;
      PTCD  = SavedPTCD ;
      PTDD  = SavedPTDD ;
     #ifndef PROCESSOR_MC1323X  
      PTED  = SavedPTED ;
      PTFD  = SavedPTFD ;
      PTGD  = SavedPTGD ;
     #endif 
    #endif
    #if (cPWR_SetupIOWhenInPD == 1)
      PTAPE = SavedPTAPE;
      PTBPE = SavedPTBPE;
      PTCPE = SavedPTCPE;
      PTDPE = SavedPTDPE;
     #ifndef PROCESSOR_MC1323X  
      PTEPE = SavedPTEPE;
      PTFPE = SavedPTFPE;
      PTGPE = SavedPTGPE;
     #endif 
      PTADD = SavedPTADD;
      PTBDD = SavedPTBDD;
      PTCDD = SavedPTCDD;
      PTDDD = SavedPTDDD;
     #ifndef PROCESSOR_MC1323X  
      PTEDD = SavedPTEDD;
      PTFDD = SavedPTFDD;
      PTGDD = SavedPTGDD;
     #endif 
    #endif
    mPWRLib_RESTORE_PORT_A;
    mPWRLib_RESTORE_PORT_B;
    mPWRLib_RESTORE_PORT_C;
    mPWRLib_RESTORE_PORT_D;
   #ifndef PROCESSOR_MC1323X  
    mPWRLib_RESTORE_PORT_E;
    mPWRLib_RESTORE_PORT_F;
    mPWRLib_RESTORE_PORT_G;
   #endif 
    PWRLib_RestoreDebugIOState();
    mPWRLib_SPINormalMode;
  #endif  /* #if (cPWR_SetupIOWhenInPD > 0) */
}   /* PWRLib_ResetMCUIOAfterPowerSavingMode ====*/


/*****************************************************************************/
/* Please see in PWRLib.h for description */
void PWRLib_MCUWait(void) {
  mMCU_SetStatus( MCU_Wait);
  mSETPIN_WAIT_MODE;
  __asm  WAIT;
  mRESETPIN_WAIT_MODE;
}   /* PWRLib_MCUWait =====*/


/*****************************************************************************/
/* Please see in PWRLib.h for description */
void PWRLib_MCUStop3(void) {
  mMCU_SetStatus( MCU_Stop3);
  mSETPIN_STOP3_MODE;
#if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
  SPMSC2 &=  0xFE;      /* Clear the PDC bit for Stop3 mode */  
#endif /* PROCESSOR_QE128 */  
  __asm STOP;
  mRESETPIN_STOP3_MODE;
}   /* PWRLib_MCUStop3 ===== */


/*****************************************************************************/
/* Please see in PWRLib.h for description  */
void PWRLib_MCUStop2(void) {
  mMCU_SetStatus( MCU_Stop2);
  mSETPIN_STOP2_MODE;
#ifdef PROCESSOR_HCS08  
  SPMSC2 |=  0x03;      /* Sets PDC and PPDC bits for Stop2 mode */
#else
  SPMSC2 |=  0x01;      /* Sets the PDC bit for Stop2 mode */
#endif  /* PROCESSOR_QE128 */
  __asm STOP;
/*  mRESETPIN_STOP2_MODE; This can't be called. Reset occurs instead */
}   /* PWRLib_MCUStop2 =====*/


/*****************************************************************************/
/* Please see in PWRLib.h for description */
void PWRLib_MCUStop1(void) {
  mMCU_SetStatus( MCU_Stop1);
#ifdef PROCESSOR_HCS08
  SPMSC2 |=  0x02;      /* Sets PDC bit for Stop1 mode */
#else
  /* Stop1 is not available on QE128 or MC1323X. Use the Stop2 instead */
  SPMSC2 |=  0x01;      /* Sets the PDC bit for Stop2 mode */  
#endif  /* PROCESSOR_QE128 */
  __asm STOP;
}   /* PWRLib_MCUStop1 =====*/


/*****************************************************************************/

#ifdef PROCESSOR_HCS08
/* Please see in PWRLib.h for description  */
void PWRLib_ICG_Mode(PWRLib_ICGMode_t Mode, PWRLib_ICGClockType_t Clock) {
    PWRLib_ICGMode = Mode; /* Store in global variable */
    ICGC1 = ((Clock<<6) | (0<<5) | (Mode<<3) | (0<<2));
    if( Mode == SCM)
        /* Select only one below */
        /*mSCM_2MHZ_DEFAULT_ICG_FILTER  */
        mSCM_DEFAULT_ICG_FILTER /* 4 MHz bus frequency. 8MHz CPU. */
        /*mSCM_8MHZ_DEFAULT_ICG_FILTER */
        /*mSCM_16MHZ_DEFAULT_ICG_FILTER */
        /*mSCM_20MHZ_DEFAULT_ICG_FILTER */
} /* PWRLib_ICG_Mode =====*/
#endif /* PROCESSOR_HCS08 */ 

/*****************************************************************************/
/* Please see in PWRLib.h for description  */
bool_t PWRLib_ICG_Wait_Clock_Lock(void) {
    if((PWRLib_ICGMode == FEI) || (PWRLib_ICGMode == FEE)) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}  /* PWRLib_ICG_Wait_Clock_Lock =====*/


/*****************************************************************************/
/* Please see in PWRLib.h for description   */
#ifdef PROCESSOR_HCS08
void PWRLib_ICG_Divider_Setup(PWRLib_ICGMultiplier_t N, PWRLib_ICGDivider_t R) {
    ICGC2 = ((0<<7) | (N<<4) | (0<<3) | (R<<0));
} /* PWRLib_ICG_Divider_Setup ====*/
#endif /* PROCESSOR_HCS08 */ 

/*****************************************************************************/
#ifndef PROCESSOR_MC1323X
/* Please see in PWRLib.h for description   */
void PWRLib_ClockSetup(PWRLib_ICGClkMode_t ClkMode) {
    uint8_t CCR;    
/*    uint16_t i = 1550; */
    uint16_t i = 800;
    IrqControlLib_BackupIrqStatus(CCR)
    IrqControlLib_DisableAllIrqs();
				
#ifdef PROCESSOR_HCS08
    /* Set LOW_CLOCK to insure FLL LOCK certain modes.   */
    PWRLib_ICG_Mode( SCM, LOW_CLOCK);
    while(i) { i--;WAIT_6_NOPS();}
    
    if ( ClkMode == Normal_16MHz) {
      /*--- Setup RADIO CLKO ---*/
      MC1319xDrv_WriteSpi(ABEL_regA, (gHardwareParameters.Abel_Clock_Out_Setting & 0xfff8) | RADIO_CLKO_62_50KHZ);
      WAIT_6_NOPS();     
      /*--- Setup the ICG module ---*/
      PWRLib_ICG_Divider_Setup( N8, R1);
      PWRLib_ICG_Mode( FEE, LOW_CLOCK);    
    }
    else if ( ClkMode == Precice_1MHz) {
      /*--- Setup RADIO CLKO ---*/
      MC1319xDrv_WriteSpi(ABEL_regA, (gHardwareParameters.Abel_Clock_Out_Setting & 0xfff8) | RADIO_CLKO_62_50KHZ);
      WAIT_6_NOPS();
           
      /*--- Setup the ICG module ---*/
      PWRLib_ICG_Divider_Setup( N4, R8);
      PWRLib_ICG_Mode( FEE, LOW_CLOCK);
    }
    else if ( ClkMode == SelfClk_2MHz) {
      PWRLib_ICG_Divider_Setup( N8, R2);
      PWRLib_ICG_Mode( SCM, LOW_CLOCK);
    }
    else if ( ClkMode == SelfClk_1MHz) {
      PWRLib_ICG_Divider_Setup( N8, R4);
      PWRLib_ICG_Mode( SCM, LOW_CLOCK);
    }
    else if ( ClkMode == SelfClk_125kHz) {
      PWRLib_ICG_Divider_Setup( N8, R32);
      PWRLib_ICG_Mode( SCM, LOW_CLOCK);
    }
    /* Wait for clock to lock in some ICG modes */
    WAIT_6_NOPS();
    if(PWRLib_ICG_Wait_Clock_Lock() == TRUE)
        while((ICGS1 & cICG_FLL_LOCKED) != cICG_FLL_LOCKED);
#endif    
#ifdef PROCESSOR_QE128
    /* PROCESSOR is QE128 (PROCESSOR_QE128) */
    {
      uint8_t icssc_temp;

      /* Set LOW_CLOCK to insure FLL LOCK certain modes.   */
      PWRLib_ICGMode = SCM;
      ICSC1 |=  0x04; /* Set IREFS */   
      while(!(ICSSC & 0x10));   /* Wait for the FLL to lock on the internal clock source */
      
      switch(ClkMode) 
      {
        case  Normal_16MHz:
                  /*--- Setup RADIO CLKO ---*/
                  MC1319xDrv_WriteSpi(ABEL_regA, (gHardwareParameters.Abel_Clock_Out_Setting & 0xfff8) | RADIO_CLKO_62_50KHZ);
                  WAIT_6_NOPS();                       
                  /*--- Setup the ICG module ---*/
                  /*            BDIV     RANGE    HGO       LP      EREFS   ERCLKEN  EREFSTEN */       
                  ICSC2      = (0<<6) | (0<<5) | (0<<4) | (0<<3) | (0<<2) | (0<<1) | (0<<0);  /* Don't set the LP bit here */                
                  /*            CLKS     RDIV     IREFS   IRCLKEN  IREFSTEN */
                  ICSC1      = (0<<6) | (1<<3) | (0<<2) | (0<<1) | (0<<0);                  
                  /*            DRST     DMX32    IREFST */
                  icssc_temp = (1<<6) | (0<<5) | (0<<4);
                  ICSSC = icssc_temp;
                  PWRLib_ICGMode = FEE;
        break;
        case  Precice_1MHz:
                  /*--- Setup RADIO CLKO ---*/
                  MC1319xDrv_WriteSpi(ABEL_regA, (gHardwareParameters.Abel_Clock_Out_Setting & 0xfff8) | RADIO_CLKO_62_50KHZ);
                  WAIT_6_NOPS();
                  /*--- Setup the ICG module ---*/
                  /*            BDIV     RANGE    HGO       LP      EREFS   ERCLKEN  EREFSTEN */
                  ICSC2      = (3<<6) | (0<<5) | (0<<4) | (0<<3) | (0<<2) | (0<<1) | (0<<0);  /* Don't set the LP bit here */                
                  /*            CLKS     RDIV     IREFS   IRCLKEN  IREFSTEN */
                  ICSC1      = (0<<6) | (1<<3) | (0<<2) | (0<<1) | (0<<0);
                  /*            DRST     DMX32    IREFST */
                  icssc_temp = (0<<6) | (0<<5) | (0<<4);
                  ICSSC = icssc_temp;
                  PWRLib_ICGMode = FEE;
        break;
        case      SelfClk_2MHz:
                  /*            BDIV     RANGE    HGO       LP      EREFS   ERCLKEN  EREFSTEN */
                  ICSC2      = (2<<6) | (0<<5) | (0<<4) | (0<<3) | (0<<2) | (0<<1) | (0<<0);  /* Don't set the LP bit here */                
                  /*            CLKS     RDIV     IREFS   IRCLKEN  IREFSTEN */
                  ICSC1      = (0<<6) | (0<<3) | (1<<2) | (0<<1) | (0<<0);
                  /*            DRST     DMX32    IREFST */
                  icssc_temp = (0<<6) | (0<<5) | (1<<4);
                  ICSSC = icssc_temp;
        break;
        case      SelfClk_1MHz:
                  /*            BDIV     RANGE    HGO       LP      EREFS   ERCLKEN  EREFSTEN */
                  ICSC2      = (3<<6) | (0<<5) | (0<<4) | (0<<3) | (0<<2) | (0<<1) | (0<<0);  /* Don't set the LP bit here */                
                  /*            CLKS     RDIV     IREFS   IRCLKEN  IREFSTEN */
                  ICSC1      = (0<<6) | (0<<3) | (1<<2) | (0<<1) | (0<<0);
                  /*            DRST     DMX32    IREFST */
                  icssc_temp = (0<<6) | (0<<5) | (1<<4);
                  ICSSC = icssc_temp;
        break;
      }
      
      while(ICSSC != icssc_temp);   /* Wait for the FLL to lock */
      
    }
#endif /* PROCESSOR_QE128 */     
    IrqControlLib_RestoreIrqStatus(CCR)
    
} /* PWRLib_ClockSetup =====*/
#endif /*PROCESSOR_MC1323X*/

/*****************************************************************************/
/* Please see in PWRLib.h for description  */
void PWRLib_RTIClockStart(uint8_t ClkMode, uint32_t Ticks) {
  uint8_t CCR;
  IrqControlLib_BackupIrqStatus(CCR)
  IrqControlLib_DisableAllIrqs();
  mSETPIN_RTISTARTED_MODE;
  
 #ifndef PROCESSOR_MC1323X
  #if (cPWR_RTIFromExternalClock)
   #ifdef PROCESSOR_HCS08 
    ICGC1 |= 0x02;      /* Set the OSCSTEN to enable RTI from external clk in stop modes 2 and 3*/
   #endif
    SRTISC = cSRTISC_Start | cSRTISC_IntClear | ClkMode | cSRTISC_External;   
  #else
    SRTISC = cSRTISC_Start | cSRTISC_IntClear | ClkMode;
  #endif
  
    PWRLib_RTIRemainingTicks = Ticks;	 //  Use specified tick count
 #else    
   
   if(Ticks > 0)   
   {
    Ticks -= 1;    // MC1323X RTC issues the interrupt when the counter is reset to 0 after match
   }
   
   RTCMODH = (uint8_t)(Ticks >> 8);
   RTCMODL = (uint8_t)Ticks;        
   
   PWRLib_RTIRemainingTicks = Ticks;	 //  Use specified tick count
   
   SRTISC = cSRTISC_Start | cSRTISC_IntClear | ClkMode | cPWR_RTIClockSource; 
 #endif  
 
  IrqControlLib_RestoreIrqStatus(CCR)
}  /* PWRLib_RTIClockStart ======*/


/*****************************************************************************/
/* Please see in PWRLib.h for description  */
uint32_t PWRLib_RTIClockCheck(void) {
#ifndef PROCESSOR_MC1323X
  uint32_t  x ;
  uint8_t CCR;
  IrqControlLib_BackupIrqStatus(CCR)
  IrqControlLib_DisableAllIrqs();
  x = PWRLib_RTIRemainingTicks;
   IrqControlLib_RestoreIrqStatus(CCR)
  return x;
#else
  uint16_t x;
  x = RTCCNTL;
  x |= (uint16_t)(RTCCNTH << 8);
  return (PWRLib_RTIRemainingTicks - x);
#endif     
}  /* PWRLib_RTIClockCheck ======*/

/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
void PWRLib_ResetTicks(void)
{
    uint8_t CCR;
  IrqControlLib_BackupIrqStatus(CCR)
  IrqControlLib_DisableAllIrqs();
	PWRLib_RTIRemainingTicks = 0;
   IrqControlLib_RestoreIrqStatus(CCR)
}
/*****************************************************************************/
/* Please see in PWRLib.h for description  */
void PWRLib_RTIClockStop(void) {
    uint8_t CCR;
  IrqControlLib_BackupIrqStatus(CCR)
  IrqControlLib_DisableAllIrqs();
  mRESETPIN_RTISTARTED_MODE;
  #ifdef PROCESSOR_MC1323X
    if(SRTISC != 0)
    {
     uint16_t x;
     /* RTI is still running */ 
     x = RTCCNTL;
     x |= (uint16_t)(RTCCNTH << 8);
     PWRLib_RTIRemainingTicks -= x;
    }
  #endif
  SRTISC = cSRTISC_Stop;
   IrqControlLib_RestoreIrqStatus(CCR)
}  /* PWRLib_RTIClockStop ======*/

#endif  /* #if (cPWR_UsePowerDownMode==1) */

/*******************************************************************************/
/* Please see in PWRLib.h for description     */
void PWRLib_Reset(void) {
  IrqControlLib_DisableAllIrqs();
  mEXECUTE_ILLEGAL_INSTRUCTION
  //mEXECUTE_JMP_TO_START
}  /* PWRLib_Reset */

/*****************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
/* Please see WARNINGS in PWRLib.h                                           */
INTERRUPT_KEYWORD void PWRLib_RTIClock_ISR(void) {
 #ifdef PROCESSOR_MC1323X
  SRTISC = cSRTISC_IntClear | cSRTISC_Stop;
 #else  
  SRTISC |= cSRTISC_IntClear;
 #endif  
 
  PWRLib_MCU_WakeupReason.Bits.FromRTI = 1;
  
  #if (cPWR_UsePowerDownMode)    
   #ifndef PROCESSOR_MC1323X
    if ( PWRLib_RTIRemainingTicks > 0) {
      PWRLib_RTIRemainingTicks--;
    }
   #else 
    PWRLib_RTIRemainingTicks = 0;
   #endif /* PROCESSOR_MC1323X */ 
    /*--- Low Voltage Check */
    #if (cPWR_LVD_Enable==2)
      if ( --PWRLib_LVD_CollectCounter == 0) {
        PWRLib_MCU_WakeupReason.Bits.LVD_Updated = 1;
        PWRLib_LVD_CollectCounter = cPWR_LVD_Ticks;
        PWRLib_LVD_SavedLevel = PWRLib_LVD_CollectLevel();
      }
    #endif  /* #if (cPWR_LVD_Enable==2) */         
  #endif  /* #if (cPWR_UsePowerDownMode==1) */
}  /* PWRLib_RTIClock_ISR ======*/


/*****************************************************************************/
/* Please see in PWRLib.h for description       */
#if (cPWR_KBIInitAndVectorEnable)
INTERRUPT_KEYWORD void PWRLib_KBI_ISR(void) {
 #ifdef PROCESSOR_QE128
  KBI2SC =  cKBI1SC | cKBI1SC_Ack;
 #else
  KBI1SC =  cKBI1SC | cKBI1SC_Ack;
 #endif    
  PWRLib_MCU_WakeupReason.Bits.FromKBI = 1;
}  /* PWRLib_KBI_ISR ====== */
#endif
#pragma CODE_SEG DEFAULT

/*****************************************************************************/
/* Please see in PWRLib.h for description                                    */
PWRLib_LVD_VoltageLevel_t PWRLib_LVD_CollectLevel(void) {
#if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2))
  /*--- Check low detect voltage ~1.8V */
  if (SPMSC1 & cSPMSC1_LVDFlag)  {
    /* Low detect voltage reached */
    SPMSC1 = cSPMSC2_ClearWarning; /* Clear flag */
    return(PWR_NODEPOWER_LEVEL_CRITICAL);
  }

  /*--- Check low trip voltage ~2.1V */
  #ifdef PROCESSOR_HCS08
  SPMSC2 = (cSPMSC2_ClearWarning | cSPMSC2_LowTripVoltage); /* Set low trip voltage and clear warning flag */
  if (SPMSC2 & cSPMSC2_LVWFlag) {
    /* Low trip voltage reached */
    SPMSC2 = cSPMSC2_ClearWarning; /* Clear flag (and set low trip voltage) */
    return(PWR_NODEPOWER_LEVEL_33);
  }
  #endif // PROCESSOR_HCS08
  
  #if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
    SPMSC3 = (cSPMSC3_ClearWarning | cSPMSC3_LowTripVoltage); /* Set low trip voltage and clear warning flag */
  if (SPMSC3 & cSPMSC3_LVWFlag) {
    /* Low trip voltage reached */
    SPMSC3 = cSPMSC3_ClearWarning; /* Clear flag (and set low trip voltage) */
    return(PWR_NODEPOWER_LEVEL_33);
  }
  #endif /* PROCESSOR_QE128 */
  
  /*--- Check high trip voltage ~2.4V and below 2.4V */
  
  #ifdef PROCESSOR_HCS08
  SPMSC2 = (cSPMSC2_ClearWarning | cSPMSC2_HighTripVoltage); /* Set high trip voltage and clear warning flag */
  if(SPMSC2 & cSPMSC2_LVWFlag) {
      /* High trip voltage reached */
      SPMSC2 = cSPMSC2_ClearWarning; /* Clear flag (and set low trip voltage) */
  #endif//PROCESSOR_HCS08 
     
  #if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
  SPMSC3 = (cSPMSC3_ClearWarning | cSPMSC3_HighTripVoltage); /* Set high trip voltage and clear warning flag */
  if(SPMSC3 & cSPMSC3_LVWFlag) {
      /* High trip voltage reached */
      SPMSC3 = cSPMSC3_ClearWarning; /* Clear flag (and set low trip voltage) */

  #endif /* PROCESSOR_QE128 */    
      if(PWRLib_LVD_L2Counter == 0) {
          return(PWR_NODEPOWER_LEVEL_50); /* Timer controlled level */
      } else {
        PWRLib_LVD_L2Counter--;
        return(PWR_NODEPOWER_LEVEL_66); /* Voltage below 2.4V */
      }
  } else {
    PWRLib_LVD_L2Counter = cPWR_LVD_LEVEL_50_Ticks; /* Reset counter */
  }
#endif  /* #if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2)) */
  /*--- Voltage level is okay > 2.4V */
  return(PWR_NODEPOWER_LEVEL_100);
}  /* PWRLib_LVD_CollectLevel ======= */


#if (cPWR_UsePowerDownMode==1)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// RADIO interface functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef PROCESSOR_MC1323X
/*****************************************************************************/
/* Please see in PWRLib.h for description   */
uint8_t PWRLib_RadioWakeReq(void) {
  uint8_t res;

  Asp_WakeReq(); /* No retun value */
  res = gSuccess_c;

  mRADIO_SetStatus( RADIO_Idle);
  return(res);
} /* PWRLib_RadioWakeReq ======*/


/*****************************************************************************/
/* Please see in PWRLib.h for description   */
uint8_t PWRLib_RadioDozeReq(zbClock24_t DozeDuration, bool_t ClkOutEnabled) {
  zbClock24_t   actualDozeTime;
  uint8_t       *pTmpTime;

  if (gSuccess_c == Asp_DozeReq(&DozeDuration, ClkOutEnabled)) {    
    pTmpTime = (uint8_t *)&DozeDuration;
    FLib_MemCpy( &actualDozeTime, pTmpTime, sizeof(zbClock24_t));
    actualDozeTime &= 0xffffff;                             /* Only 3 bytes internally */
    if( DozeDuration == actualDozeTime ) {
      mRADIO_SetStatus( RADIO_Doze);
       MC1319xDrv_AttEnable();                                            /* Set att. pin high so we do not waste any power..*/
      return(TRUE);
    } else {
      mRADIO_SetStatus( RADIO_Idle);
      return(FALSE);
    }
  }
  mRADIO_SetStatus( RADIO_Idle);
    return(FALSE);
} /* PWRLib_RadioDozeReq ======*/


/*****************************************************************************/
/* Please see in PWRLib.h for description      */
uint8_t PWRLib_RadioAcomaDozeReq(bool_t ClkOutEnabled) {

    if (gSuccess_c == Asp_AcomaReq(ClkOutEnabled)) {      
      mRADIO_SetStatus( RADIO_AcomaDoze);
        MC1319xDrv_AttEnable();/* Set att. pin high so we do not waste any power..*/
      return(TRUE);
    } else {
      mRADIO_SetStatus( RADIO_Idle);
      return(FALSE);
    }
} /* PWRLib_RadioAcomaDozeReq ======*/


/*****************************************************************************/
/* Please see in PWRLib.h for description     */
uint8_t PWRLib_RadioHibernateReq(void) {

  if (gSuccess_c == Asp_HibernateReq()) {    
    mRADIO_SetStatus( RADIO_Hibernate);
     MC1319xDrv_AttEnable(); /* Set att. pin high so we do not waste any power.*/
    return(TRUE);
  } else {
    mRADIO_SetStatus( RADIO_Idle);
    return(FALSE);
  }
} /* PWRLib_RadioHibernateReq ======*/


/*****************************************************************************/
/* Please see in PWRLib.h for description     */
void PWRLib_RadioOffReq(void) {
    MC1319xDrv_AssertReset(); 			 /* Reset RADIO */
    mPWRLib_SETUPFOR_RADIORESET;  
  mRADIO_SetStatus( RADIO_Off);
} /* PWRLib_RadioOffReq ======*/


/*****************************************************************************/
/* Please see in PWRLib.h for description     */
void PWRLib_RadioOnReq(void) {
    mPWRLib_SETUPAFTER_RADIORESET;
    MC1319xDrv_DeassertReset();
	MC1319xDrv_AttDisable(); 
  mRADIO_SetStatus( RADIO_Idle);
} /* PWRLib_RadioOnReq ======*/

#endif /*PROCESSOR_MC1323X*/

/*****************************************************************************/
/* Please see in PWRLib.h for description        */
uint8_t PWRLib_GetMacStateReq() {
    return Asp_GetMacStateReq();
}  /* PWRLib_GetMacStateReq() =====*/


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Zigbee stack interface functions
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/*#define PWRLib_GetCurrentZigbeeStackPowerState     PWRLib_StackPS;*/
/*#define PWRLib_SetCurrentZigbeeStackPowerState( x) PWRLib_StackPS = x;*/


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Common init function
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif  /*#if (cPWR_UsePowerDownMode==1) */
/*****************************************************************************/
/* Please see in PWRLib.h for description     */
void PWRLib_Init(void) {
  #if (cPWR_UsePowerDownMode)
    PWRLib_ICGMode              = FEE;
    mMCU_SetStatus( MCU_Running);
    mRADIO_SetStatus( RADIO_Idle);
        
    #if (cPWR_SetupIOWhenInPD >= 1)
      SavedPTAD             = PTAD ;
      SavedPTBD             = PTBD ;
      SavedPTCD             = PTCD ;
      SavedPTDD             = PTDD ;
     #ifndef PROCESSOR_MC1323X
      SavedPTED             = PTED ;
      SavedPTFD             = PTFD ;
      SavedPTGD             = PTGD ;
     #endif 
    #endif  /* (cPWR_SetupIOWhenInPD >= 1) */
    #if (cPWR_SetupIOWhenInPD == 1)
      SavedPTADD            = PTADD;
      SavedPTBDD            = PTBDD;
      SavedPTCDD            = PTCDD;
      SavedPTDDD            = PTDDD;
     #ifndef PROCESSOR_MC1323X 
      SavedPTEDD            = PTEDD;
      SavedPTFDD            = PTFDD;
      SavedPTGDD            = PTGDD;
     #endif 
      SavedPTAPE            = PTAPE;
      SavedPTBPE            = PTBPE;
      SavedPTCPE            = PTCPE;
      SavedPTDPE            = PTDPE;
     #ifndef PROCESSOR_MC1323X 
      SavedPTEPE            = PTEPE;
      SavedPTFPE            = PTFPE;
      SavedPTGPE            = PTGPE;
     #endif 
    #endif  /* #if (cPWR_SetupIOWhenInPD == 1) */
    mRESETPIN_STOP2_MODE;
    #if (cPWR_KBIInitAndVectorEnable)
      #if (cPWR_KBIWakeupEnable)
       #if defined(PROCESSOR_QE128) 
        KBI2PE = cKBI1PE;                                         /* KBI setup */
        KBI2SC = cKBI1SC | cKBI1SC_Ack;  
       #elif defined(PROCESSOR_MC1323X)        
        KBIPE  =  gMC1323xKBIPinEnableMask_d; /* KBI pin enable controls initialized*/
        KBIES  = ~gMC1323xKBIPinEnableMask_d; /* KBI interrupt edge select*/
        KBISC = cKBI1SC | cKBI1SC_Ack; 
       #else
       //PROCESSOR_HCS08
        KBI1PE = cKBI1PE;                                         /* KBI setup */
        KBI1SC = cKBI1SC | cKBI1SC_Ack;
       #endif             
      #endif
    #endif
    PWRLib_MCU_WakeupReason.AllBits = 0;
    if ( SPMSC2 & 0x08) {
#if defined(PROCESSOR_MC1323X)
	  PWRLib_MCU_WakeupReason.Bits.FromSCI = 1;                     /* Wakeup from SCI */
#else
      PWRLib_MCU_WakeupReason.Bits.FromStop2 = 1;                     /* Wakeup from STOP2 mode */
#endif	  
    } else {
      PWRLib_MCU_WakeupReason.Bits.FromReset = 1;                     /* Ordinary reset */
    }
    PWRLib_SetCurrentZigbeeStackPowerState( StackPS_DeepSleep);
    // LVD_Init_Start
    #if (cPWR_LVD_Enable == 0)
       SPMSC1 = cSPMSC1Init_LVD_Disable; /* Write once (SRTISC). Done here to conserve power */
    #elif ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2))
       SPMSC1 = cSPMSC1Init_LVD_Enable;  /* Write once (SRTISC). Done here enable power level */  
    #elif (cPWR_LVD_Enable==3)
       SPMSC1 = cSPMSC1Init_LVD_Enable | cSPMSC1Init_LVDR_Enable;  /* Write once (SRTISC). Done here enable power level */  
       #ifdef PROCESSOR_HCS08
       SPMSC2 |= cSPMSC2_LVD_V_High;
       #endif
       #ifdef PROCESSOR_QE128
       SPMSC3 |= cSPMSC3_LVD_V_High;
       #endif
       #ifdef PROCESSOR_MC1323X
       SPMSC3 |= cSPMSC3_LVD_V_High;
       #endif
       
    #endif /* #if (cPWR_LVD_Enable) */
  
    #if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2))
      PWRLib_LVD_SavedLevel = PWR_NODEPOWER_LEVEL_100;
      PWRLib_LVD_L2Counter  = cPWR_LVD_LEVEL_50_Ticks;
    #endif  /* #if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2)) */
    #if (cPWR_LVD_Enable == 2)
      PWRLib_LVD_CollectCounter = cPWR_LVD_Ticks;
    #endif  /* #if (cPWR_LVD_Enable==2) */
    // LVD_Init_End
    SPMSC2 |=  0x04;      /* Sets PPDACK bit to restore output drivers after Stop2 mode */
  #endif  /*#if (cPWR_UsePowerDownMode) */

} /*PWR_Init =====*/
  