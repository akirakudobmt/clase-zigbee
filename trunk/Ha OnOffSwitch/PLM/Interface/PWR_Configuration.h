/************************************************************************************
* Function to handle/set the different power down states.
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
***********************************************************************************/
#ifndef _PWR_CONFIG_H_
#define _PWR_CONFIG_H_

#include "PortConfig.h"

/************************************************************************************
*************************************************************************************
* Module configuration constants
*************************************************************************************
************************************************************************************/


//-----------------------------------------------------------------------------
// To enable/disable all of the code in this PWR/PWRLib files.
//   TRUE =  1: Use PowerDown functions (Normal)
//   FALSE = 0: Don't use PowerDown. Will cut variables and code out. But
//     functions still exist. Useful for debugging and test purposes
#ifndef cPWR_UsePowerDownMode
#define cPWR_UsePowerDownMode                   1
#endif


//-----------------------------------------------------------------------------
// The way that DeepSleep mode are handled. Following possibilities exist:
//   0: No DeepSleep done, but PTC application can set modes
//   1: Ext. KBI int wakeup
//   2: RTI timer wakeup +-30%

//   3: Ext. KBI int and RTI timer wakeup +-30%, radio in OFF/reset mode.
//   3: MC1323X: MCU in STOP3, RTI/SCI/KBI wakeup, KBI enabled

//   4: Ext. KBI int and RTI timer wakeup +-30%, radio in hibernate - not reset
//   4: MC1323X: MCU in STOP3, RTI/SCI/KBI wakeup, KBI enabled. RTI wakeup timeout selectable at runtime. RTI resolution fixed to 32us

//   5: Radio in acoma/doze, supplying 62.5kHz clock to MCU, MCU in STOP3,
//      RTI wakeup from ext clock 512mS (max avail with ext 62.5khz)
//   5: MC1323X: Mode 5: MCU in STOP3, RTI/SCI/KBI wakeup, KBI enabled

//      Suggested use: debug mode for modes 3 or 4

//   6: Radio in doze mode, supplying 62.5KHz to MCU, MCU in STOP3
//      Radio wakeup after cPWR_DozeDurationMs. KBI Int Wake-up available also.
//   6: MC1323X: MCU in STOP3, Wakeup on 802.15.4 timer, Other wake-up sources: KBI/SCI

//-------- Test modes : -------------------------------------------------------
//  30: Test of some stack code (Not good or valid anymore)
//  31: Test as #3 but power down only done once
//  33: Test of MCU Wait
//  34: Test of MCU Stop3
//  35: Test of MCU Stop2
//  36: Test of MCU Stop1
//  37: Test of RADIO Doze without clkout
//  38: Test of RADIO AcomaDoze without clkout
//  39: Test of RADIO Hibernate
//  40: Test of RADIO Off
#ifndef cPWR_DeepSleepMode
#define cPWR_DeepSleepMode                      3
#endif

//-----------------------------------------------------------------------------
// The way that Sleep mode are handled. Following possibilities exist:
//   0: No Sleep done, but PTC application can set modes
//   1: Doze mode on RADIO with clock enabled (MAC 1.06 only) and WAIT on MCU
#ifndef cPWR_SleepMode
#define cPWR_SleepMode                          1
#endif

//-----------------------------------------------------------------------------
// The distance between RTI interrupts when enabled and in Stop2/3
// Following possibilities exist:  cSRTISC_IntDisabled,
// cSRTISC_Int0008ms, cSRTISC_Int0032ms, cSRTISC_Int0064ms, cSRTISC_Int0128ms,
// cSRTISC_Int0256ms, cSRTISC_Int0512ms, cSRTISC_Int1024ms
// MC1323X: This define represents the RTC tick time and depends on the cPWR_RTIClockSource
#ifndef cPWR_RTITickTime
 #ifndef PROCESSOR_MC1323X
  #define cPWR_RTITickTime                        7
 #else
  #define cPWR_RTITickTime                        7
 #endif   
#endif
//-----------------------------------------------------------------------------
// The doze duration in ms when enabled in Stop3. Used in cPWR_DeepSleepMode == 6
// Default value of 3072ms set to match the default timeout set when using RTI modes
#ifndef cPWR_DozeDurationMs
 #ifndef PROCESSOR_MC1323X
  #define cPWR_DozeDurationMs                     (3072)
 #else
  #define cPWR_DozeDurationMs                     (2000)
 #endif
#endif

//-----------------------------------------------------------------------------
// This number multiplied with the above cPWR_RTITickTime time gives the time
// to DeepSleep
#ifndef cPWR_RTITicks
 #ifndef PROCESSOR_MC1323X
  #define cPWR_RTITicks                           3
 #else 
  #define cPWR_RTITicks                           3
 #endif 
#endif

//-----------------------------------------------------------------------------
// Enabling of external call to a procedure each time that DeepSleep are exited
//   0: Don't call any functions after DeepSleep (MAC)
//   1: Call a function after DeepSleep (Stack)
#ifndef cPWR_CallWakeupStackProcAfterDeepSleep
#define cPWR_CallWakeupStackProcAfterDeepSleep  1
#endif

//-----------------------------------------------------------------------------
// The extra function to call every time RTI clock run's out. Used by Stack.
#if (cPWR_CallWakeupStackProcAfterDeepSleep == 0)
  #define cPWR_DeepSleepWakeupStackProc         ;
#else
  extern void                                   DeepSleepWakeupStackProc(void);
  #define cPWR_DeepSleepWakeupStackProc         DeepSleepWakeupStackProc();  
#endif


//-----------------------------------------------------------------------------
// KBI interrupt vector and initialization enable
//   TRUE = 1: KBI interrupt vector used and KBI registers are initialized in PWRLIB(Normal)
//   FALSE= 0: KBI interrupt vector and init has to be done externally
#ifndef cPWR_KBIInitAndVectorEnable
#define cPWR_KBIInitAndVectorEnable            0
#endif

//-----------------------------------------------------------------------------
// Select if the KBI interrupt has to be enabled or disabled if the define:
// cPWR_KBIInitAndVectorEnable are TRUE
//   TRUE = 1: Also use KBI interrupt to wakeup
//   FALSE= 0: Don't use KBI int. on PTAx pins
#ifndef cPWR_KBIWakeupEnable
#define cPWR_KBIWakeupEnable                   1
#endif

//-----------------------------------------------------------------------------
// Setting up the port settings when entering and exiting PowerDown
//   0: No change done to IO when entering PD (Debug)
//   1: Setup all ports (Most as outputs) when entering PD. Restoring all
//      ports using saved values. Uses 21 bytes in RAM. (Safest)
//   2: Setup all ports (Most as outputs) when entering PD (Normal). Restoring
//      all ports. Only Data part from saved values restored from preset values.
//      Uses only 7 bytes in RAM. (Normal).
#ifndef cPWR_SetupIOWhenInPD
#define cPWR_SetupIOWhenInPD                    0
#endif

//-----------------------------------------------------------------------------
// Whether to setup RADIO SPI port as I/O when entering PD mode
//   TRUE =  1: Use the SPI pins as outputs under PD (Normal)
//   FALSE = 0: Don't
#ifndef cPWR_UseSPIDisable
#define cPWR_UseSPIDisable                      0
#endif

//-----------------------------------------------------------------------------
// On EVB (DIG528-2) an connector J107 can be used for debugging
//   FALSE = 0: No debug info on J107. Normal customer version. (Normal)
//   TRUE =  1: Use J107 as DEBUG port. Debug status are valid also during PD.
//   Note: A small glitch can occour when going in/out of PD.
//   J107.1  : RTI Running
//   J107.2  : PWR_CheckForAndEnterNewPowerState called/active
//   J107.3  : Available
//   J107.4  : Available
//   J107.5  : Available
//   J107.6  : Available
//   J107.7  : Available
//   J107.8  : MCU Wait called/active
//   J107.9  : MCU Stop2 called/active
//   J107.10 : MCU Stop3 called/active
#ifndef cPWR_UseDebugOutputs
#define cPWR_UseDebugOutputs                    0
#endif

//-----------------------------------------------------------------------------
// As a service to Application this variable can be created and updated
//   TRUE =  1: Use/update the PWRLib_RADIOStatus variable (Debug/Application)
//   FALSE = 0: Don't (Normal)
#ifndef cPWR_UseRADIOStatus
#define cPWR_UseRADIOStatus                     0
#endif

//-----------------------------------------------------------------------------
// As a service to Application this variable can be created and updated
//   TRUE =  1: Use/update the MCU_Status variable (Debug/Application)
//   FALSE = 0: Don't (Normal)
#ifndef cPWR_UseMCUStatus
#define cPWR_UseMCUStatus                       0
#endif
//-----------------------------------------------------------------------------
// Whether to run RTI from internal or external oscillator
//   TRUE =  1: Use external clock for RTI timer
//   FALSE = 0: Uses internal clock (Normal)

/* Use external clock in debugging mode */
/* Use internal clock in normal, lowest-power mode */

#ifndef PROCESSOR_MC1323X
 #if (cPWR_DeepSleepMode != 5)	 // Debug mode?
   #define cPWR_RTIFromExternalClock  0  // Normal case, MCU's low-power internal clock
 #else
   #define cPWR_RTIFromExternalClock  1  // Force external clock in debug mode
 #endif
#else
  #ifndef cPWR_RTIClockSource
    #define cPWR_RTIClockSource        cSRTISC_SourceInt1KHz
  #endif
#endif /* PROCESSOR_MC1323X */  
//-----------------------------------------------------------------------------
// The use of Low Voltage detection has the following possibilities:
//   0: Don't use Low voltage detection at all
//   1: Use polled => Check made each time the function is called.
//   2: RTI timer used for handling when to poll LVD, according
//      to the cPWR_LVD_Ticks constant
//   3: LVDE and LVDRE  are set to hold MCU in reset while LVD_VH  condition is detected
//      
#ifndef cPWR_LVD_Enable
#define cPWR_LVD_Enable                         3
#endif

//-----------------------------------------------------------------------------
// How often to check the LVD level when cPWR_LVD_Enable == 2
// This is the number of RTI interrupts before voltage is checked (Consumes
// current and time)
#ifndef cPWR_LVD_Ticks
#define cPWR_LVD_Ticks                          3600
#endif

//-----------------------------------------------------------------------------
// A counter value to control LEVEL 2. The value will be decremented whenever
// PWR_NODEPOWER_LEVEL_66 is valid. System will change to PWR_NODEPOWER_LEVEL_50
// when the counter reaches 0. MUST BE ADJUSTED ACCORDING TO HOW OFTEN THE LVD
// is polled (cPWR_LVD_LEVEL_50_Ticks * cPWR_LVD_Ticks * cPWR_RTITickTime).
#ifndef cPWR_LVD_LEVEL_50_Ticks
#define cPWR_LVD_LEVEL_50_Ticks                 10
#endif

//-----------------------------------------------------------------------------
// Definitions for KBI interrupt setup (KBI1SC)
//                              .--------- KBEDG7    : 1:Rising edges/high. 0:Falling edge/low level
//                              |.-------- KBEDG6    : 1:Rising edges/high. 0:Falling edge/low level
//                              ||.------- KBEDG5    : 1:Rising edges/high. 0:Falling edge/low level
//                              |||.------ KBEDG4    : 1:Rising edges/high. 0:Falling edge/low level
//                              ||||.----- KBF       : Keyboard Int. status Flag (Readable only)
//                              |||||.---- KBACK     : Keyboard Int. Acknowledge (Writable only)
//                              ||||||.--- KBIE      : Keyboard Int. Enable
//                              |||||||.-- KBIMOD    : Keyboard Detection Mode. 1:Edge and level, 0:Edge
#define cKBI1SC               0b00000010                    // Enable Edge triggered interrupts from KBI
#define cKBI1SC_Ack           0b00000100                    // Acknowledge of int


#if (cPWR_KBIWakeupEnable)
  //-----------------------------------------------------------------------------
  // Definitions for KBI input selection if used (KBI1PE)
  //                            .--------- KBIPE7    : Enable KPI on PTA7
  //                            |.-------- KBIPE6    : Enable KPI on PTA6
  //                            ||.------- KBIPE5    : Enable KPI on PTA5
  //                            |||.------ KBIPE4    : Enable KPI on PTA4
  //                            ||||.----- KBIPE3    : Enable KPI on PTA3
  //                            |||||.---- KBIPE2    : Enable KPI on PTA2
  //                            ||||||.--- KBIPE1    : Enable KPI on PTA1
  //                            |||||||.-- KBIPE0    : Enable KPI on PTA1
    #define cKBI1PE         0b00010000                    // Only PTA4 = S103 interrupt
#else
  #define cKBI1PE             0b00000000                    // No inputs enabled
#endif   // #if (cPWR_KBIWakeupEnable)


#if (defined (TARGET_AXIOM_GB60) || defined (TARGET_TOROWEAP) || defined (TARGET_DIG528_2) || defined(TARGET_RD01) || defined(TARGET_DIG536_2) || defined(TARGET_FSL556_1) || defined(TARGET_USER_DEFINED) ) 

  //-----------------------------------------------------------------------------
  // Macroes for IO setup for returning to normal RUNNING mode.
  // Will set IO direction, pull-ups and port value for each port, if
  // enabled by cPWR_SetupIOWhenInPD
  //   PTxD : Output value for Port-x - 1: high,          0: low
  //   PTxPE: Input pullup for Port-x - 1: pullup active, 0: inactive
  //   PTxDD: Direction for Port-x    - 1: output,        0: input
  // Uses a copy of existing definitions defined in Target.h and App_Target.h
  // and some are created directly.
  // NOTE, must be changed to match your hardware!!!

  //--- From mAPP_SETUP_PORT_A  in  App_Target.h
  #define  mPWRLib_RESTORE_PORT_A               PTAPE = mSWITCH_MASK;\
                                                PTADD = 0x42;           // Set as input except 2 * CTS

  //--- From mSETUP_PORT_B  in  Target.h
  #define  mPWRLib_RESTORE_PORT_B               PTBPE = 0x00;\
                                                PTBDD = (gMC1319xAntSwMask_c);\
                                                PTBDD &=(~gMC1319xGPIO1Mask_c \
                                                          &~gMC1319xGPIO2Mask_c); // Set as input 
                                                              

  //--- From mSETUP_PORT_C  in  Target.h
  #define  mPWRLib_RESTORE_PORT_C               PTCPE = 0x00;\
                                                PTCDD = 0xfd | (gMC1319xResetMask_c \
                                                                |gMC1319xAttnMask_c \
                                                                | gMC1319xRxTxMask_c );
                                                                  
                                                                 

  //--- From mAPP_SETUP_PORT_D  in  App_Target.h
  #define  mPWRLib_RESTORE_PORT_D               PTDPE = 0x00;\
                                                PTDDD = 0xff;       // LED_MASK;

  #define  mPWRLib_RESTORE_PORT_E               PTEPE = 0x00;\
                                                PTEDD = 0xf5;

  #define  mPWRLib_RESTORE_PORT_F               PTFPE = 0x00;\
                                                PTFDD = 0xff;

  #define  mPWRLib_RESTORE_PORT_G               PTGPE = 0x00;\
                                                PTGDD = 0xfb;


  //---------------------------------------------------------------------------//
  //--- Macros for IO setup for WAIT, STOP3 and STOP2 modes and reset.      ---//
  //--- Will set IO direction, pull-ups and port value for each port, if    ---//
  //--- enabled by the cPWR_SetupIOWhenInPD define.                         ---//
  //---   PTxD : Output value for Port-x - 1: high,          0: low         ---//
  //---   PTxPE: Input pullup for Port-x - 1: pullup active, 0: inactive    ---//
  //---   PTxDD: Direction for Port-x    - 1: output,        0: input       ---//
  #define  mPWRLib_SETUP_PORT_A                 
  /*
                                                PTAD  = 0x00;\
                                                PTAPE = 0x10;\
                                                PTADD = 0x6E & ~cKBI1PE;
  */
  #define  mPWRLib_SETUP_PORT_B                 PTBD  = 0x00;\
                                                PTBPE = 0x00;\
                                                PTBDD = 0xcf;

  #define  mPWRLib_SETUP_PORT_C                 PTCD  = 0x1c;\
                                                PTCPE = 0x00;\
                                                PTCDD = 0xfd;

  #define  mPWRLib_SETUP_PORT_D                 PTDD  = 0x1b;\
                                                PTDPE = 0x00;\
                                                PTDDD = 0xff;

  #define  mPWRLib_SETUP_PORT_E                 PTED  = 0x04;\
                                                PTEPE = 0x00;\
                                                PTEDD = 0xf5;

  #define  mPWRLib_SETUP_PORT_F                 PTFD  = 0x00;\
                                                PTFPE = 0x00;\
                                                PTFDD = 0xff;

  #define  mPWRLib_SETUP_PORT_G                 PTGD  = 0x01;\
                                                PTGPE = 0x00;\
                                                PTGDD = 0xfb;


  //---------------------------------------------------------------------------//
  //--- Extra settings for SPI pin setup when going in/out of PD            ---//
  #if (cPWR_UseSPIDisable)
    #define mPWRLib_SPIPowerSaveMode            SPI1C1 &= ~0x40;
    #define mPWRLib_SPINormalMode               SPI1C1 |=  0x40;
  #else
    #define mPWRLib_SPIPowerSaveMode            ;
    #define mPWRLib_SPINormalMode               ;
  #endif  // #if (cPWR_UseSPIDisable)


  //---------------------------------------------------------------------------//
  //--- Extra settings when entering the RADIO Reset mode                   ---//
  //--- Setup GPIO1 & 2 as high outputs                                     ---//
  //--- Setup MISOo as high output                                          ---//
  //--- Setup CLKO as high output                                           ---//
  #if (cPWR_SetupIOWhenInPD > 0)
    #define mPWRLib_SETUPFOR_RADIORESET         PTBD &= ~0x30; \
                                                PTBDD|=  0x30; \
                                                PTED &= ~0x08; \
                                                PTEDD|=  0x08; \
                                                PTGD &= ~0x04; \
                                                PTGDD|=  0x04;
  #else
    #define mPWRLib_SETUPFOR_RADIORESET         ;
  #endif

  //---------------------------------------------------------------------------//
  //--- Extra settings when exiting the RADIO Reset mode                    ---//
  //--- Setup GPIO1 & 2 as inputs                                           ---//
  //--- Setup MISOo as input                                                ---//
  //--- Setup CLKO as input                                                 ---//
  #if (cPWR_SetupIOWhenInPD > 0)
    #define mPWRLib_SETUPAFTER_RADIORESET       PTBDD&= ~0x30; \
                                                PTEDD&= ~0x08; \
                                                PTGDD&= ~0x04;
  #else
    #define mPWRLib_SETUPAFTER_RADIORESET       ;
  #endif


  //---------------------------------------------------------------------------//
  //--- Bitmask for DEBUG info pins                                         ---//
  #define mPWRLib_DEBUG_MASK_PTBD               0x8e
  #define mPWRLib_DEBUG_MASK_PTCD               0xe0
  #define mPWRLib_DEBUG_MASK_PTDD               0x04


  //---------------------------------------------------------------------------//
  //--- Definitions of DEBUG info pins                                      ---//
  #if (cPWR_UseDebugOutputs)
    #define mSETPIN_STOP3_MODE                  PTDD |= 0x04;   // Set PTD2     J107.10
    #define mRESETPIN_STOP3_MODE                PTDD &= ~0x04;  // Reset PTD2
    #define mSETPIN_STOP2_MODE                  PTBD |= 0x02;   // Set PTB1     J107.9
    #define mRESETPIN_STOP2_MODE                PTBD &= ~0x02;  // Reset PTB1
    #define mSETPIN_WAIT_MODE                   PTBD |= 0x04;   // Set PTB2     J107.8
    #define mRESETPIN_WAIT_MODE                 PTBD &= ~0x04;  // Reset PTB2
    #define mSETPIN_CHKPWRSTATE_MODE            PTCD |= 0x40;   // Set PTC6     J107.2
    #define mRESETPIN_CHKPWRSTATE_MODE          PTCD &= ~0x40;  // Reset PTC6
    #define mSETPIN_RTISTARTED_MODE             PTCD |= 0x20;   // Set PTC5     J107.1
    #define mRESETPIN_RTISTARTED_MODE           PTCD &= ~0x20;  // Reset PTC5
  #else
    #define mSETPIN_STOP3_MODE                  ;
    #define mRESETPIN_STOP3_MODE                ;
    #define mSETPIN_STOP2_MODE                  ;
    #define mRESETPIN_STOP2_MODE                ;
    #define mSETPIN_WAIT_MODE                   ;
    #define mRESETPIN_WAIT_MODE                 ;
    #define mSETPIN_CHKPWRSTATE_MODE            ;
    #define mRESETPIN_CHKPWRSTATE_MODE          ;
    #define mSETPIN_RTISTARTED_MODE             ;
    #define mRESETPIN_RTISTARTED_MODE           ;
  #endif

#elif defined(TARGET_MC1323x)                                                       

  //-----------------------------------------------------------------------------
  // Macroes for IO setup for returning to normal RUNNING mode.
  // Will set IO direction, pull-ups and port value for each port, if
  // enabled by cPWR_SetupIOWhenInPD
  //   PTxD : Output value for Port-x - 1: high,          0: low
  //   PTxPE: Input pullup for Port-x - 1: pullup active, 0: inactive
  //   PTxDD: Direction for Port-x    - 1: output,        0: input
  // Uses a copy of existing definitions defined in Target.h and App_Target.h
  // and some are created directly.
  // NOTE, must be changed to match your hardware!!!

  //--- From mSETUP_PORT_A  in  Target.h
  #define  mPWRLib_RESTORE_PORT_A               mSETUP_PORT_A

  //--- From mSETUP_PORT_B  in  Target.h
  #define  mPWRLib_RESTORE_PORT_B               mSETUP_PORT_B

  //--- From mSETUP_PORT_C  in  Target.h
  #define  mPWRLib_RESTORE_PORT_C               mSETUP_PORT_C


  //--- From mSETUP_PORT_D  in  Target.h
  #define  mPWRLib_RESTORE_PORT_D               mSETUP_PORT_D

  #define  mPWRLib_RESTORE_PORT_E               

  #define  mPWRLib_RESTORE_PORT_F               

  #define  mPWRLib_RESTORE_PORT_G               


  //---------------------------------------------------------------------------//
  //--- Macros for IO setup for WAIT, STOP3 and STOP2 modes and reset.      ---//
  //--- Will set IO direction, pull-ups and port value for each port, if    ---//
  //--- enabled by the cPWR_SetupIOWhenInPD define.                         ---//
  //---   PTxD : Output value for Port-x - 1: high,          0: low         ---//
  //---   PTxPE: Input pullup for Port-x - 1: pullup active, 0: inactive    ---//
  //---   PTxDD: Direction for Port-x    - 1: output,        0: input       ---//
  #define  mPWRLib_SETUP_PORT_A                 
  /*
                                                PTAD  = 0x00;\
                                                PTAPE = 0x10;\
                                                PTADD = 0x6E & ~cKBI1PE;
  */
  #define  mPWRLib_SETUP_PORT_B                 PTBD  = 0x00;\
                                                PTBPE = 0x00;\
                                                PTBDD = 0xcf;

  #define  mPWRLib_SETUP_PORT_C                 PTCD  = 0x1c;\
                                                PTCPE = 0x00;\
                                                PTCDD = 0xfd;

  #define  mPWRLib_SETUP_PORT_D                 PTDD  = 0x1b;\
                                                PTDPE = 0x00;\
                                                PTDDD = 0xff;

  #define  mPWRLib_SETUP_PORT_E                 

  #define  mPWRLib_SETUP_PORT_F                 

  #define  mPWRLib_SETUP_PORT_G                 


  //---------------------------------------------------------------------------//
  //--- Extra settings for SPI pin setup when going in/out of PD            ---//
  #if (cPWR_UseSPIDisable)
    #define mPWRLib_SPIPowerSaveMode            SPI1C1 &= ~0x40;
    #define mPWRLib_SPINormalMode               SPI1C1 |=  0x40;
  #else
    #define mPWRLib_SPIPowerSaveMode            ;
    #define mPWRLib_SPINormalMode               ;
  #endif  // #if (cPWR_UseSPIDisable)


  //---------------------------------------------------------------------------//
  //--- Extra settings when entering the RADIO Reset mode                   ---//
  //--- Setup GPIO1 & 2 as high outputs                                     ---//
  //--- Setup MISOo as high output                                          ---//
  //--- Setup CLKO as high output                                           ---//
  #if (cPWR_SetupIOWhenInPD > 0)
    #define mPWRLib_SETUPFOR_RADIORESET         ;
  #else
    #define mPWRLib_SETUPFOR_RADIORESET         ;
  #endif

  //---------------------------------------------------------------------------//
  //--- Extra settings when exiting the RADIO Reset mode                    ---//
  //--- Setup GPIO1 & 2 as inputs                                           ---//
  //--- Setup MISOo as input                                                ---//
  //--- Setup CLKO as input                                                 ---//
  #if (cPWR_SetupIOWhenInPD > 0)
    #define mPWRLib_SETUPAFTER_RADIORESET       ;
  #else
    #define mPWRLib_SETUPAFTER_RADIORESET       ;
  #endif


  //---------------------------------------------------------------------------//
  //--- Bitmask for DEBUG info pins                                         ---//
  #define mPWRLib_DEBUG_MASK_PTBD               0x8e
  #define mPWRLib_DEBUG_MASK_PTCD               0xe0
  #define mPWRLib_DEBUG_MASK_PTDD               0x04


  //---------------------------------------------------------------------------//
  //--- Definitions of DEBUG info pins                                      ---//
  #if (cPWR_UseDebugOutputs)
    #define mSETPIN_STOP3_MODE                  PTDD |= 0x04;   // Set PTD2     J107.10
    #define mRESETPIN_STOP3_MODE                PTDD &= ~0x04;  // Reset PTD2
    #define mSETPIN_STOP2_MODE                  PTBD |= 0x02;   // Set PTB1     J107.9
    #define mRESETPIN_STOP2_MODE                PTBD &= ~0x02;  // Reset PTB1
    #define mSETPIN_WAIT_MODE                   PTBD |= 0x04;   // Set PTB2     J107.8
    #define mRESETPIN_WAIT_MODE                 PTBD &= ~0x04;  // Reset PTB2
    #define mSETPIN_CHKPWRSTATE_MODE            PTCD |= 0x40;   // Set PTC6     J107.2
    #define mRESETPIN_CHKPWRSTATE_MODE          PTCD &= ~0x40;  // Reset PTC6
    #define mSETPIN_RTISTARTED_MODE             PTCD |= 0x20;   // Set PTC5     J107.1
    #define mRESETPIN_RTISTARTED_MODE           PTCD &= ~0x20;  // Reset PTC5
  #else
    #define mSETPIN_STOP3_MODE                  ;
    #define mRESETPIN_STOP3_MODE                ;
    #define mSETPIN_STOP2_MODE                  ;
    #define mRESETPIN_STOP2_MODE                ;
    #define mSETPIN_WAIT_MODE                   ;
    #define mRESETPIN_WAIT_MODE                 ;
    #define mSETPIN_CHKPWRSTATE_MODE            ;
    #define mRESETPIN_CHKPWRSTATE_MODE          ;
    #define mSETPIN_RTISTARTED_MODE             ;
    #define mRESETPIN_RTISTARTED_MODE           ;
  #endif

//---------------------------------------------------------------------------//
//--- Sanity checks
#else
  #error "*** ERROR: target not supported !"
#endif  // #if defined (TARGET_RD01) #else

#if (cPWR_UsePowerDownMode > 1)
  #error "*** ERROR: Illegal value in cPWR_UsePowerDownMode"
#endif


#if (cPWR_CallWakeupStackProcAfterDeepSleep > 1)
  #error "*** ERROR: Illegal value in cPWR_CallWakeupStackProcAfterDeepSleep"
#endif

#if (cPWR_KBIInitAndVectorEnable > 1)
  #error "*** ERROR: Illegal value in cPWR_KBIInitAndVectorEnable"
#endif

#if (cPWR_KBIWakeupEnable > 1)
  #error "*** ERROR: Illegal value in cPWR_KBIWakeupEnable"
#endif

#if (cPWR_SetupIOWhenInPD > 2)
  #error "*** ERROR: Illegal value in cPWR_SetupIOWhenInPD"
#endif

#if (cPWR_UseSPIDisable > 1)
  #error "*** ERROR: Illegal value in cPWR_UseSPIDisable"
#endif

#if ((cPWR_UseSPIDisable > 0) && (cPWR_SetupIOWhenInPD == 0))
  #error "*** ERROR: If no IO port setup is done - then changing SPI gives no meaning"
#endif

#if (cPWR_UseDebugOutputs > 1)
  #error "*** ERROR: Illegal value in cPWR_UseDebugOutputs"
#endif

#if (cPWR_UseRADIOStatus > 1)
  #error "*** ERROR: Illegal value in cPWR_UseRADIOStatus"
#endif

#if (cPWR_UseMCUStatus > 1)
  #error "*** ERROR: Illegal value in cPWR_UseMCUStatus"
#endif

#ifndef PROCESSOR_MC1323X
 #if (cPWR_RTIFromExternalClock > 1)
  #error "*** ERROR: Illegal value in cPWR_RTIFromExternalClock"
 #endif
#endif

#if (cPWR_LVD_Enable > 3)
  #error "*** ERROR: Illegal value in cPWR_LVD_Enable"
#endif

#endif

