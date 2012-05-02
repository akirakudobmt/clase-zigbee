/************************************************************************************
* Functions to handle the different power states.
*
* (c) Copyright 2008, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _PWRLIB_H_
#define _PWRLIB_H_


/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
#include "MsgSystem.h"
//#include "hwdrv_hcs08.h"
#include "crt0.h"
#include "PWR_Configuration.h"
#include "IrqControlLib.h"

/************************************************************************************
*************************************************************************************
* Segment definitions
*************************************************************************************
************************************************************************************/
#pragma DATA_SEG __SHORT_SEG MY_ZEROPAGE
#pragma DATA_SEG DEFAULT
#define cPlaceInZeroPageSegment   @ "MY_ZEROPAGE"


/************************************************************************************
*************************************************************************************
* Module configuration constants
*************************************************************************************
************************************************************************************/


//---------------------------------------------------------------------------//
//--- Definitions for RTI timer setup (SRTISC @0x1808)                    ---//
#if defined(PROCESSOR_QE128)

  #define cSRTISC_IntDisabled   0b000
  #define cSRTISC_Int0008ms     0b001
  #define cSRTISC_Int0032ms     0b010
  #define cSRTISC_Int0064ms     0b011
  #define cSRTISC_Int0128ms     0b100
  #define cSRTISC_Int0256ms     0b101
  #define cSRTISC_Int0512ms     0b110
  #define cSRTISC_Int1024ms     0b111

  #define cSRTISC_Start         0x10
  #define cSRTISC_External      0x20
  #define cSRTISC_Stop          0x00
  #define cSRTISC_IntClear      0x80


#elif defined(PROCESSOR_MC1323X)
  #define cSRTISC_IntDisabled   0b0000
  
  /* RTC period for 1KHz source clock */  
  #define cSRTISC_Int0008ms     0b0001
  #define cSRTISC_Int0032ms     0b0010
  #define cSRTISC_Int0064ms     0b0011
  #define cSRTISC_Int0128ms     0b0100
  #define cSRTISC_Int0256ms     0b0101
  #define cSRTISC_Int0512ms     0b0110
  #define cSRTISC_Int1024ms     0b0111
  #define cSRTISC_Int0001ms     0b1000
  #define cSRTISC_Int0002ms     0b1001
  #define cSRTISC_Int0004ms     0b1010
  #define cSRTISC_Int0010ms     0b1011
  #define cSRTISC_Int0016ms     0b1100
  #define cSRTISC_Int0100ms     0b1101
  #define cSRTISC_Int0500ms     0b1110
  #define cSRTISC_Int1000ms     0b1111  
  
  /* RTC period for 32MHz source clock */
  #define cSRTISC_Int0032us     0b0001
  #define cSRTISC_Int0064us     0b0010
  #define cSRTISC_Int0128us     0b0011
  #define cSRTISC_Int0256us     0b0100
  #define cSRTISC_Int0512us     0b0101
  #define cSRTISC_Int1024us     0b0110
  #define cSRTISC_Int2048us     0b0111  

  #define cSRTISC_Start         0x10
  #define cSRTISC_Stop          0x00
  #define cSRTISC_IntClear      0x80

  #define cSRTISC_SourceInt1KHz  0x00
  #define cSRTISC_SourceExt32MHz 0x20
  #define cSRTISC_SourceExt32KHz 0x40

#else 
//PROCESSOR_HCS08
  #define cSRTISC_IntDisabled   0b000
  #define cSRTISC_Int0008ms     0b001
  #define cSRTISC_Int0032ms     0b010
  #define cSRTISC_Int0064ms     0b011
  #define cSRTISC_Int0128ms     0b100
  #define cSRTISC_Int0256ms     0b101
  #define cSRTISC_Int0512ms     0b110
  #define cSRTISC_Int1024ms     0b111
  //                              .--------- RTIF(R)  : Real-Time Interrupt flag
  //                              |.-------- RTIACK(W): RTI acknowledge
  //                              ||.------- RTICLKS  : Clock select. 1: Ext. 0: Int. 1kHz
  //                              |||.------ RTIE     : Real Time Interrupt Enable
  //                              ||||.----- 0
  //                              |||||...-- RTIS0-2  : Delay Select. See above
  #define cSRTISC_Start         0b00010000
  #define cSRTISC_External      0b00100000
  #define cSRTISC_Stop          0b00000000
  #define cSRTISC_IntClear      0b01000000


#endif


#define cRTI_OneTickInMillis   (uint16_t)( (cPWR_RTITickTime ==1) \
                               ?8: ((uint16_t)0x01<<(3 + cPWR_RTITickTime)) )                            

//---------------------------------------------------------------------------//
//--- Definitions for SPMSC1 (PMCSC1 @0x1809)                             ---//
//                                .---------  LVDF(R)   : Low Voltage Detect flag
//                                |.--------  LVDACK(W) : Low Voltage Detect Acknowledge
//                                ||.-------  LVDIE     : Low Voltage Detect Interrupt Enable
//                                |||.------  LVDRE     : Low Voltage Detect Reset Enable (Writable only once)
//                                ||||.-----  LVDSE     : Low Voltage Detect Stop Enable (Writable only once)
//                                |||||.----  LVDDE     : Low Voltage Detect Detect Enable (Writable only once)
//                                ||||||..--  00        : Unused
#define cSPMSC1Init_LVD_Disable 0b00000000    // Write ONCE (SRTISC). Done to conserve power -> no Low voltage detection
#define cSPMSC1Init_LVD_Enable  0b00000100    // No interrupt on low voltage, no low voltage detection in STOP mode,
                                              // Enable voltage detection - Write ones register
#define cSPMSC1Init_LVDR_Enable 0b00010000    //                                               
#define cSPMSC1_LVDFlag         0b10000000    // Low voltage flag set when threshold value reaced


//---------------------------------------------------------------------------//
//--- Definitions for SPMSC2 (PMCSC2 @0x180a)                             ---//
//                                .---------  LVWF(R)   : Low Voltage Warning flag
//                                |.--------  LVWACK(W) : Low Voltage Warning Acknowledge
//                                ||.-------  LVDV      : Low Voltage Detect Voltage Select
//                                |||.------  LVWV      : Low Voltage Warning Voltage Select
//                                ||||.-----  PPDF      : Partial Power Down Flag
//                                |||||.----  PPDACK    : Partial Power Down Acknowledge (W) Clears PPDF bit
//                                ||||||.---  PDC       : Power Down Control
//                                |||||||.--  PPDC      : Partial Power Down Control
#define cSPMSC2_HighTripVoltage 0b00010000      // Set high trip voltage
#define cSPMSC2_LowTripVoltage  0b00000000      // Set low  trip voltage
#define cSPMSC2_ClearWarning    0b01000000      // Clear warning flag
#define cSPMSC2_LVWFlag         0b10000000      // Low voltage flag set when threshold value reached
#define cSPMSC2_LVD_V_High      0b00100000      // Set high trip voltage


//---------------------------------------------------------------------------//
#if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
//---------------------------------------------------------------------------//
//--- Definitions for SPMSC3 (PMCSC2 @0x180b)                             ---//
//                                .---------  LVWF(R)   : Low Voltage Warning flag
//                                |.--------  LVWACK(W) : Low Voltage Warning Acknowledge
//                                ||.-------  LVDV      : Low Voltage Detect Voltage Select
//                                |||.------  LVWV      : Low Voltage Warning Voltage Select
//                                ||||.-----  LVWIE      : Partial Power Down Flag
//                                |||||.----  0
//                                ||||||.---  0
//                                |||||||.--  0
#define cSPMSC3_HighTripVoltage 0b00010000      // Set high trip voltage
#define cSPMSC3_LowTripVoltage  0b00000000      // Set low  trip voltage
#define cSPMSC3_ClearWarning    0b01000000      // Clear warning flag
#define cSPMSC3_LVWFlag         0b10000000      // Low voltage flag set when threshold value reached
#define cSPMSC3_LVD_V_High      0b00100000      // Set high trip voltage

//---------------------------------------------------------------------------//
#endif /* PROCESSOR_QE128 */

//--- Register redefinitions for compliance with MC9S08GB60/D             ---//
#define KBI1SC                    KBISC                     // Register at 0x0016
#define KBI1PE                    KBIPE                     // Register at 0x0017
#define SPI1C1                    SPIC1                     // Register at 0x0028

#if defined (PROCESSOR_QE128)
#define SRTISC                    RTCSC                    // Register at 0x1830
#elif defined (PROCESSOR_MC1323X)
#define SRTISC                    RTCSC                    // Register at 0x1828
#else
//PROCESSOR_HCS08
#define SRTISC                    PMCRSC                    // Register at 0x1808
#endif

#ifndef PROCESSOR_MC1323X
#define SPMSC1                    PMCSC1                    // Register at 0x1809 on HCS08; 0x1808 on QE128 and MC1323x
#define SPMSC2                    PMCSC2                    // Register at 0x180a on HCS08; 0x1809 on QE128 and MC1323x
#endif

#if defined(PROCESSOR_QE128)
#define SPMSC3                    PMCSC3                    // Register at 0x180b
#endif 
//---------------------------------------------------------------------------//
//--- Definitions for the reset function                                  ---//
#define mEXECUTE_ILLEGAL_INSTRUCTION __asm ldhx illegal_opcode_ptr; \
                                     __asm jmp ,x;

#define mEXECUTE_JMP_TO_START        __asm jmp _Startup;

/************************************************************************************
*************************************************************************************
* Public types
*************************************************************************************
************************************************************************************/

//---------------------------------------------------------------------------//
//--- Specifics for setting clock modes                                   ---//
#ifdef PROCESSOR_HCS08
typedef enum {Normal_16MHz=0, Precice_1MHz, SelfClk_2MHz, \
              SelfClk_1MHz, SelfClk_125kHz}               PWRLib_ICGClkMode_t;
#endif
              
#ifdef PROCESSOR_QE128
typedef enum {Normal_16MHz=0, Precice_1MHz, SelfClk_2MHz, \
              SelfClk_1MHz}               PWRLib_ICGClkMode_t;
#endif

#ifdef PROCESSOR_MC1323X
typedef enum {Normal_16MHz=0}               PWRLib_ICGClkMode_t;
#endif
              
typedef enum {SCM = 0,FEI, FBE, FEE}                      PWRLib_ICGMode_t;
typedef enum {LOW_CLOCK = 0, HIGH_CLOCK}                  PWRLib_ICGClockType_t;
typedef enum {N4 = 0, N6, N8, N10, N12, N14, N16, N18}    PWRLib_ICGMultiplier_t;
typedef enum {R1 = 0, R2, R4, R8, R16, R32, R64, R128}    PWRLib_ICGDivider_t;


//---------------------------------------------------------------------------//
//--- Specifics for chip modes and wakeup reason                          ---//
typedef enum {MCU_Running=66, MCU_Wait, MCU_Stop3, MCU_Stop2, MCU_Stop1}  PWRLib_MCUStatus_t;
typedef enum {RADIO_Idle=88, RADIO_Doze, RADIO_AcomaDoze, \
              RADIO_Hibernate, RADIO_Off}                                 PWRLib_RADIOStatus_t;


//---------------------------------------------------------------------------//
//--- RADIO CLKO register valuedefines. Note: Has to start at 0           ---//
typedef enum {RADIO_CLKO_16MHZ = 0, RADIO_CLKO_8MHZ, RADIO_CLKO_4MHZ, RADIO_CLKO_2MHZ, \
              RADIO_CLKO_1MHZ, RADIO_CLKO_62_50KHZ, RADIO_CLKO_32_78KHZ, \
              RADIO_CLKO_16_39KHZ}                                        PWRLib_RADIO_CLKO_t;


//---------------------------------------------------------------------------//
//--- Zigbee stack power state                                            ---//
typedef enum {StackPS_Running=122, StackPS_Sleep, StackPS_DeepSleep}      PWRLib_StackPS_t;


//---------------------------------------------------------------------------//
//--- Wakeup reason UNION definitions                                     ---//
typedef  union {
  uint8_t AllBits;
  struct {
    unsigned int   FromReset       :1;                // Comming from Reset
#if defined(PROCESSOR_MC1323X)
    unsigned int   FromSCI       :1;                // SCI wakeup
#else
    unsigned int   FromStop2       :1;                // Comming from Stop2 mode
#endif
    unsigned int   FromKBI         :1;                // Wakeup by KBI interrupt
    unsigned int   FromRTI         :1;                // Wakeup by RTI timer interrupt
    unsigned int   FromTimer       :1;                // Wakeup by Radio Timer
    unsigned int   DeepSleepTimeout:1;                // DeepSleep timer ran out
    unsigned int   SleepTimeout    :1;                // Sleep timer ran out
    unsigned int   LVD_Updated     :1;                //
  } Bits;
} PWRLib_WakeupReason_t;


//---------------------------------------------------------------------------//
//--- Reported voltage levels. Level 4 is highest (best)                  ---//
//---        Highest ----------------------------- Lowest voltage         ---//
typedef enum{ PWR_NODEPOWER_LEVEL_100 = 0x0c, PWR_NODEPOWER_LEVEL_66=0x08, \
              PWR_NODEPOWER_LEVEL_50, PWR_NODEPOWER_LEVEL_33=0x04, \
              PWR_NODEPOWER_LEVEL_CRITICAL=0x00}             PWRLib_LVD_VoltageLevel_t;


/************************************************************************************
*************************************************************************************
* External variable declarations
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Public macroes
*************************************************************************************
************************************************************************************/

//---------------------------------------------------------------------------//
//--- Definitions for PWRLib_RADIOStatus writing                          ---//
#if (cPWR_UseRADIOStatus)
  #define mRADIO_SetStatus( x)     PWRLib_RADIOStatus = x;
  #define mRADIO_GetStatus         PWRLib_RADIOStatus
#else
  #define mRADIO_SetStatus( x)     ;
  #define mRADIO_GetStatus
#endif  // #if (cPWR_UseRADIOStatus)


//---------------------------------------------------------------------------//
//--- Definitions for PWRLib_MCUStatus writing                            ---//
#if (cPWR_UseMCUStatus)
  #define mMCU_SetStatus( x)     PWRLib_MCUStatus = x;
#else
  #define mMCU_SetStatus( x)     ;
#endif  // #if (cPWR_UseMCUStatus)


/************************************************************************************
*************************************************************************************
* Public variable declarations (External)
*************************************************************************************
************************************************************************************/
//--- Chip status ---//
#if (cPWR_UseMCUStatus)
  extern PWRLib_MCUStatus_t               PWRLib_MCUStatus;
#endif  // #if (cPWR_UseMCUStatus)

#if (cPWR_UseRADIOStatus)
  extern PWRLib_RADIOStatus_t             PWRLib_RADIOStatus;
#endif  // #if (cPWR_UseRADIOStatus)

extern volatile PWRLib_WakeupReason_t     PWRLib_MCU_WakeupReason   cPlaceInZeroPageSegment;


//--- Zigbee stack status ---//
extern PWRLib_StackPS_t                   PWRLib_StackPS;

//--- For LVD function ---//
#if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2))
  #if (cPWR_LVD_Enable == 2)
    extern uint16_t                PWRLib_LVD_CollectCounter;
  #endif  // #if (cPWR_LVD_Enable == 2)
  extern uint16_t                  PWRLib_LVD_L2Counter;
  extern PWRLib_LVD_VoltageLevel_t PWRLib_LVD_SavedLevel;
#endif // #if ((cPWR_LVD_Enable == 1) || (cPWR_LVD_Enable == 2))


//---------------------------------------------------------------------------//
//--- Only include the rest if needed / enabled                           ---//
#if (cPWR_UsePowerDownMode==1)


/************************************************************************************
*************************************************************************************
* Public function prototypes
*************************************************************************************
************************************************************************************/

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// MCU interface functions
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------



/******************************************************************************
* Description : Save state of all debug pins
* Assumptions : Assumes that direction of ports are correct
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_SaveDebugIOState(void);


/******************************************************************************
* Description : Restore state off all debug pins
* Assumptions : Assumes that direction of ports are correct
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_RestoreDebugIOState(void);


/******************************************************************************
* Description : Set all MCU IO pins for low power mode. Most as outputs with a
*               zero value.
* Assumptions : Assumes interrupt has been turned off, to ensure that no other
*               process changes IO.
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_SetMCUIOForPowerSavingMode(void);


/******************************************************************************
* Description : Restore all MCU IO pins after a low power mode. Using previous-
*               ly saved values or reset values.
* Assumptions : Assumes interrupt has been turned off, to ensure that no other
*               process changes IO.
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_ResetMCUIOAfterPowerSavingMode(void);


/******************************************************************************
* Description : MCU will enter the WAIT mode and stay there until an interrupt
*               restarts the execution.
*               CPU standby, CPU registers OK, RAM OK, RTI OK, KBI OK, Regulator
*               on, I/O latched, BUS clk on.
* Assumptions : Intended for use anywhere in code where execution may pause.
* Input       : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_MCUWait(void);


/******************************************************************************
* Description : MCU will enter the STOP3 mode and stay there until an interrupt
*               restarts the execution.
*               CPU standby, CPU registers OK, RAM OK, RTI OK, KBI OK, Regulator
*               on, I/O latched, BUS clk off
* Assumptions : None
* Input       : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_MCUStop3(void);


/******************************************************************************
* Description : MCU will enter the STOP2 mode and stay there until an interrupt
*               restarts the execution.
*               CPU standby, CPU registers Lost, RAM OK, RTI OK, KBI OK, Regulator
*               off, I/O latched, BUS clk off
* Note        : Remember to set PPDACK to enable ports after STOP2
* Input       : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_MCUStop2(void);


/******************************************************************************
* Description : MCU will enter the STOP1 mode and stay there until an RESET
*               occurs.
*               CPU off, CPU registers Lost, RAM OFF, No RTI, No KBI, Regulator
*               off, I/O tristated, BUS clk off
* Assumptions : None
* Input       : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_MCUStop1(void);

#ifdef PROCESSOR_HCS08
/******************************************************************************
* Description : For setting up the ICG mode.
* Assumptions : None
* Input       : ICG_mode - the ICG mode to enter
*               Clock    - the ICG clock type (low or high)
* Output      : None
* Errors      : Not handled
* History     : 20050329  pbbusk1  Created from mvchr1 source
******************************************************************************/
void PWRLib_ICG_Mode(PWRLib_ICGMode_t ICG_mode, PWRLib_ICGClockType_t Clock);
#endif

/******************************************************************************
* Description : Returns TRUE if it is necessary to wait for the clock to lock (DCO).
* Assumptions : None
* Input       : None
* Output      : TRUE -  ICG clock must lock in current mode - wait for lock
*               FALSE - ICG clock cannot lock in current mode (Not nesesary)
* Errors      : Not handled
******************************************************************************/
bool_t PWRLib_ICG_Wait_Clock_Lock(void);

#ifdef PROCESSOR_HCS08
/******************************************************************************
* Description : For setup of ICG multiplier and divider (N/R).
* Assumptions : None
* Inputs      : N - multiplier factor - not used in all ICG clock modes.
*               R - divider factor - used in all ICG clock modes.
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_ICG_Divider_Setup(PWRLib_ICGMultiplier_t N, PWRLib_ICGDivider_t R);
#endif

/******************************************************************************
* Description : Setup the MCU ICG clock mode and RADIO ClkO.
* Assumptions : None
* Inputs      : ClkMode: The chosen clock mode
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_ClockSetup(PWRLib_ICGClkMode_t ClkMode);


/******************************************************************************
* Description : Setup the MCU RTI timer for generating interrupts and set the
*               counter to selected value. This will be decremented auto-
*               matically by the interrupt. Remember that the resulting
*               interrupt are only with an accuracy of +-30%
* Assumptions : None
* Inputs      : ClkMode : The chosen clock mode
*               Ticks   : Number of ticks (Time depends on ClkMode)
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_RTIClockStart(uint8_t ClkMode, uint32_t Ticks);


/******************************************************************************
* Description : Returns remaining time to timeout.
* Assumptions : None
* Inputs      : None
* Output      : Remaining number of ticks
* Errors      : Not handled
******************************************************************************/
uint32_t PWRLib_RTIClockCheck(void);


/******************************************************************************
* Description : Stops the MCU RTI timer from generating interrupts in STOP2/3
* Assumptions : None
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_RTIClockStop(void);


/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
/******************************************************************************
* Description : Interrupt occurring every time the RTI timer times out.
* Assumptions : None
* WARNINGS    : Internal RTI oscillator is only running when not in RUN or WAIT
*               mode (AN2493). RTI is ok in Simulation mode except it runs 16
*               times too fast. Internal RTI oscillator don't run when debugger
*               is connected. DISCONNECT DEBUGGER or stop debugger and reset.
* Inputs      : None
* Output      : Remaining number of ticks. Uses PWRLib_MCU_WakeupReason.Bits.FromRTI
*               = 1 to set reason for wakeup.
* Errors      : Not handled
******************************************************************************/
INTERRUPT_KEYWORD void PWRLib_RTIClock_ISR(void);


/******************************************************************************
* Description : Interrupt occurring every time the Keyboard Interrupt pins are
*               activated.
* Assumptions : None
* Inputs      : None
* Output      : Uses PWRLib_MCU_WakeupReason.Bits.FromKBI = 1 to set reason
* Errors      : Not handled
******************************************************************************/
INTERRUPT_KEYWORD void PWRLib_KBI_ISR(void);
#pragma CODE_SEG DEFAULT

/******************************************************************************
* Description : Detection of low voltage.
*               Will use the MCU internal LVI module to get a battery voltage level.
*               This can be polled directly from application code or use the RTI
*               interrupt for collecting and storing the level. This last method
*               is less power demanding.
*
*               Possible returned levels are:
*               PWR_NODEPOWER_LEVEL_100  >  2.4V  - "Very Good" - bus frequency up to max. 20 MHz
*
*               PWR_NODEPOWER_LEVEL_66  <= 2.4V  - "Good" - bus frequency up to max. 20 MHz still supported
*
*               PWR_NODEPOWER_LEVEL_50  >  2.1V & < 2.4V - "Low" a user selectable level (see below)
*
*               PWR_NODEPOWER_LEVEL_33  <= 2.1V  - "Critical" for bus frequencies > 8 MHz
*
*               PWR_NODEPOWER_LEVEL_CRITICAL  <  1.8V  - "Hazardous"  - System must be shut down to avoid resets etc.
*
*               PWR_NODEPOWER_LEVEL_50 in more detail (user selectable):
*               The user can define this level as a timed values according to the application's
*               current consumption, i.e. the selected voltage level on the decreasing voltage
*               slope for the used battery type.
*
*               The user must select a voltage level on the batteries decreasing voltage slope
*               and measure the time for the battery voltage to drop from 2.4V down to the
*               selected voltage level above 2.1V.
*
*               Note! The battery voltage drop depends strongly on the application's current
*               draw and temperature.
*
*               The counter value "cPWR_LVD_LEVEL_50_Ticks" must be set according to how often
*               the LVD module is polled. Ex. If it takes 10 hours to discharge from 2.4V to
*               2.2V and the LVD module is called ones every 1 hour cPWR_LVD_LEVEL_50_Ticks = 10.
*
* Assumptions : None
* NOTE        : Voltage threshold levels differs slightly from device to device
*               Typical values are used above
*               This function are normally not called from external modules
*
* Inputs      : None
* Output      : PWR_NODEPOWER_LEVEL_CRITICAL..PWR_NODEPOWER_LEVEL_100 : As specified above
* Errors      : Not handled
******************************************************************************/
PWRLib_LVD_VoltageLevel_t PWRLib_LVD_CollectLevel(void);


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// RADIO interface functions
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
#ifndef PROCESSOR_MC1323X
/******************************************************************************
* Description : Request MAC to Wake up the RADIO
* Assumptions : None
* Inputs      : None
* Output      : TRUE if successful
* Errors      : Not handled but reported
******************************************************************************/
uint8_t PWRLib_RadioWakeReq(void);


/******************************************************************************
* Description : Request MAC to set radio in doze mode for requested number of
*               symbols (16usec).
* Assumptions : None
* Inputs      : DozeDuration is the time to doze
*               ClkOutEnabled is TRUE if ClkO should be active.
*                 FALSE will disable ClkO and consequently save power
* Output      : TRUE if DozeDuration req. granted
* Errors      : Not handled but reported
******************************************************************************/
uint8_t PWRLib_RadioDozeReq(zbClock24_t DozeDuration, bool_t ClkOutEnabled);


/******************************************************************************
* Description : Request MAC to set radio in AcomaDoze.
* Assumptions : None
* Inputs      : ClkOutEnabled is TRUE if ClkO should be active.
*                 FALSE will disable ClkO and consequently save power
* Output      : TRUE if req. granted
* Errors      : Not handled but reported
******************************************************************************/
uint8_t PWRLib_RadioAcomaDozeReq(bool_t ClkOutEnabled);


/******************************************************************************
* Description : Request MAC to set RADIO in Hibernate mode
* Assumptions : None
* Inputs      : None
* Output      : TRUE if Hibernate req. granted
* Errors      : Not handled but reported
******************************************************************************/
uint8_t PWRLib_RadioHibernateReq(void);


/******************************************************************************
* Description : Resets the RADIO. Will use the least amount of current.
* Assumptions : None
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_RadioOffReq(void);


/******************************************************************************
* Description : Remove the Reset signal to the RADIO. It will return to Idle
*               mode within 25msec.
* Assumptions : None
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_RadioOnReq(void);

#endif /*PROCESSOR_MC1323X*/
/******************************************************************************
* Description : Get status from MAC. Functions just as Asp_GetMacStateReq().
* Assumptions : None
* Inputs      : None
* Output      : gAspMacStateIdle_c     : MAC ready for Sleep or DeepSleep
*               gAspMacStateBusy_c     : Don't sleep
*               gAspMacStateNotEmpty_c : MAC allows Wait
* Errors      : Not handled
******************************************************************************/
uint8_t PWRLib_GetMacStateReq(void);


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// Zigbee stack interface functions
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

/******************************************************************************
* Description : Macros to get the current status of the STACK
* Assumptions : None
* Inputs      : None
* Output      : Status from PWRLib_StackPS
* Errors      : Not handled
******************************************************************************/
#define PWRLib_GetCurrentZigbeeStackPowerState      PWRLib_StackPS


/******************************************************************************
* Description : Macro to set the status of the STACK
* Inputs      : NewStackPS  : New state
*                 StackPS_Running   : Busy
*                 StackPS_Sleep     : Sleep allowed. Duration is appl. specific
*                 StackPS_DeepSleep : DeepSleep allowed
* Output      : None
* Errors      : Not handled
******************************************************************************/
#define PWRLib_SetCurrentZigbeeStackPowerState( NewStackPS)  PWRLib_StackPS = NewStackPS;
#else  // #if (cPWR_UsePowerDownMode==1)
#define PWRLib_SetCurrentZigbeeStackPowerState( NewStackPS)
#endif  // #if (cPWR_UsePowerDownMode==1)


/******************************************************************************
* Description : Initialize the Power Lib functions for correct functioning.
* Assumptions : Assumes normal running with MCU and RADIO with correct IO etc.
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/



void PWRLib_Init(void);

/******************************************************************************
* Description : Executes a illegal instruction to force a MCU reset.
* Assumptions : None
* Inputs      : None
* Output      : None
* Errors      : Not handled
******************************************************************************/
void PWRLib_Reset(void);

#endif _PWRLIB_H_

