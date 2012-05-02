/************************************************************************************
* Main file for physical layer in ZigBee.
* Handle initialization, etc.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#include "MacPhyGlobalHdr.h"
#include "MacPhyInit.h"
#include "MacPhy.h"
#include "IrqControlLib.h"
#include "Phy.h"

// **************************************************************************
// Define strings to remove warnings when building
#define VER   " V "
#define BUILD " Build: "
#define SPACE " " 

#if gVersionStringCapability_d
#pragma gVersionStringPhyBegin_d

// DO NOT CHANGE OR REMOVE

// This string will be located in the NV RAM0 section.
// Note!! Check that item is location in the sequence as specified. 

const uint8_t PHY_Version[47] = "PHY" VER PHY_Version_No BUILD __DATE__ SPACE __TIME__;

#pragma gVersionStringPhyEnd_d
#endif // gVersionStringCapability_d

// **************************************************************************
// The following table contains addresses of Abel register to be initialized
// Each address has a corresponding value in the setupValueTable table. Do NOT
// change the order of occurence. It might be worth considering entering
// the initialization tables into the NV section for costumer control
static const uint8_t setupAddrTable[] = {
  ABEL_reg8,
  ABEL_LO1DIV_REG,
  ABEL_LO1NUM_REG,
  ABEL_reg11,
  ABEL_reg12,
  ABEL_reg13,
  ABEL_reg31,
  ABEL_reg38,
#if gTransceiverType_d != gMC1321x_c // If not using toroweap
  // Abel 2.2 only. May be removed for Abel 2.3. Fix for excess power consumption during hibernate and doze. 
  // Must activate thermometer and deactivate later for power to be low enough in these modes 
  ABEL_CONTROL2_REG,// Abel 2.2 only
  ABEL_reg9,        // Abel 2.2 only
#endif
  //
  ABEL_CONTROL2_REG,
  ABEL_MASK_REG,
  ABEL_reg9,
  ABEL_TMR1_HI_REG, // Stop all wrap around interrupts
  ABEL_TMR2_HI_REG,
  ABEL_TMR3_HI_REG,
  ABEL_TMR4_HI_REG,
  ABEL_regB         // Configured to all outputs (inputs on all but GPIO1 and GPIO2 : 0x01FC)
};



// The following table contains values of Abel registers to be initialized
// Each value has a address value in the setupAddrTable table. Do NOT
// change the order of occurence
static const uint16_t setupValueTable[] = {
  0xFFF7,           // R0x08: 0xFFF7
  0x0F95,           // 0x0F8F for 2305Mhz, 0x0F95 for 2405Mhz
  0x5000,           // 0x1000 for 2305MHz, 0x5000 for 2405MHz
  0xA0FF,           // 0xA0FF: Recommended setting from Phoenix team
  0x00BC,           // V1.2:0x80CC, default V2.0:0x000C : PA level
  0x1843,           // V1.2:0x1803, default V2.0:0x1863 : Sigma-delta fade detector differs
  0xA000,
  0x0008,           // Correlator VT, 0x0000=Abel 1.2
#if gTransceiverType_d != gMC1321x_c // If not using toroweap
  // The next two are for Abel 2.2 only. May be removed for Abel 2.3. Fix for excess power consumption
  // during hibernate and doze. Must activate thermometer and deactivate later for power to be low enough
  // in these modes 
  0x7D3C,           // For Abel 2.2 only
  0xF3FD,           // For Abel 2.2 only
#endif
  // The next three values may not be evaluated in a pretty way, but it is very effective - let the preprocessor do the work

#if gTransceiverType_d == gMC1321x_c // If using toroweap
  // Default is single-ended antenna
#define tmp1 (0x5c00 | cUSE_STRM_MODE | (0<<8)) /* bit 8=0 is SFD throw away length 0,1,2, and restart Rx */
#else
#define tmp1 (0x7c00 | cABEL2SPI_MASK | cUSE_STRM_MODE)
#endif // gTransceiverType_d == gMC1321x_c
#if USE_INTERRUPT_TXEOF 
#define tmp2 (tmp1 | (uint16_t) cTX_DONE_MASK)  // Yes, we want Tx done interrupts
#else
#define tmp2 (tmp1)
#endif
#if USE_INTERRUPT_RXEOF
#define tmp3 (tmp2 | (uint16_t) cRX_DONE_MASK)  // Yes, we want Rx done interrupts
#else
#define tmp3 (tmp2)
#endif
  tmp3,             // Setup interrupt mask

#define tmp4 (cTMR4_MASK | cTMR2_MASK | cLO1_LOCK_MASK)
#if gAspPowerSaveCapability_d
#define tmp5 (tmp4 | cDOZE_MASK | cTMR1_MASK)
#else 
#define tmp5 (tmp4)
#endif
  tmp5,             // Write Abel Mask registers ("Control2" and "Mask")  

#define tmp6 (0xF3FD & ~cTMR_PRESCALEmask)
#define tmp7 (tmp6 | (5 << cTMR_PRESCALEshift)) // 5=62.5kHz, 3=250kHz(default)
#define tmp8 (tmp7 | 0x80)  // Alternate GPIO enable
  tmp8,

  0x8000,           // Stop all wrap around interrupts
  0x8000,
  0x8000,
  0x8000,
  0x3F80            // Configured to all outputs (inputs on all but GPIO1 and GPIO2 : 0x01FC)
};



// **************************************************************************
#ifdef MEMORY_MODEL_BANKED 
void __near(* __near mpfPendingSetup)(void);
void __near(* __near gpTimer2Isr)(void);
#else
void (* __near mpfPendingSetup)(void);
void (* __near gpTimer2Isr)(void);
#endif  /* MEMORY_MODEL_BANKED */

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
void __near DummyFastIsr(void)
{
  ;
}
#pragma CODE_SEG DEFAULT

/*******************************************************************
* Now use SPI to initialize ABEL                                   *
*******************************************************************/
void Phy_InitOnStartup(uint16_t phyRfCalibration, bool_t useDualAntenna)
{
  uint8_t   idx, addr;
  uint16_t  retReg;

  // MSB = CCA Energy detect threshold: abs(power in dBm)*2, 0x96=-75dBm
  // LSB = Power compensation Offset added to RSSI. Typically 0x74 FFJ/JK 13/01/04 (Abel 013)
  MC1319xDrv_WriteSpiAsync(ABEL_reg4, phyRfCalibration);
  
   // Now set up various register from the tables (see tables above)

  for (idx = 0; idx < sizeof(setupAddrTable)/sizeof(setupAddrTable[0]); idx++) {
    addr = setupAddrTable[idx]; // It saves a few code bytes to use a stack variable
    MC1319xDrv_WriteSpiAsync(addr, setupValueTable[idx]);
  }

  /* Apply trimming for Toro 1.3 */
  MC1319xDrv_ReadSpiAsync(ABEL_reg2C, &retReg);
  if(retReg == 0x6800)
  {
   /* Toro 1.3 version detected */
   MC1319xDrv_ReadSpiAsync(ABEL_reg31, &retReg);
   retReg |= 0x00C0;
   MC1319xDrv_WriteSpiAsync(ABEL_reg31, retReg);
   
   MC1319xDrv_ReadSpiAsync(ABEL_reg34, &retReg);
   retReg |= 0x0004;
   MC1319xDrv_WriteSpiAsync(ABEL_reg34, retReg);
  }
  
    // Setup default antenna switch mode
  if (useDualAntenna == TRUE){
    MC1319xDrv_ReadSpiAsync(ABEL_CONTROL2_REG, &retReg);
    retReg &= 0x8FFF;
    retReg |= 0x4000; // Dual antenna setup at boot, CT_Bias_en = 1, RF_switch_mode = 0, CT_Bias_inv = 0
    MC1319xDrv_WriteSpiAsync(ABEL_CONTROL2_REG, retReg);
  }

  MC1319xDrv_ReadSpiAsync(ABEL_STATUS_REG, &retReg);   // Clear interrupts by reading interrupt status
  MC1319xDrv_WriteSpiAsync(ABEL_TMR2_HI_REG, 0x0000); // Enable TMR2 interrupts
}

void InitializePhy(void)
{
  CODE_PROFILING_ENABLE_PINS;
  RXFILTER_PROFILING_ENABLE_PINS;

  IrqControlLib_InitMC1319xIrq();      // Enable external interrupt from MC1319x, low level and edge-triggered


  mPhyTxRxState = cIdle;
  gpTimer2Isr = IsrTx2prime;

  gIsrFastAction = DummyFastIsr;
  gIsrAsyncTailFunction = DummyFastIsr;
  gIsrEntryCounter = 0;

  mpfPendingSetup = NULL;
}


/**
  mPhyTxRxState is used as internal state reference for the PHY layer. Upon reset, the state is 'cIdle'
  State is changed from to either 'cBusy_Rx' or 'cBusy_Tx' by SetRxTxState primitive.
  Next state can be cRxED or xRxCCA or cTxData, depending on sunsequent command. (or return to cIdle)
  Once an action is completed (EOF), cIdle is set again
*/
phyTxRxState_t mPhyTxRxState; // Allowed states: cBusy_Rx, cBusy_Tx, cIdle, cRxED, cRxCCA

int8_t gTxRemainingLength;

uint8_t gPhyMacDataRxIndex;
uint8_t gPhyMacDataTxIndex;

uint8_t *gpPhyTxPacket;

uint8_t *gpPhyPacketHead;
rxPacket_t *gpPhyRxData;

zbClock16_t gRxTimeStamp;
zbClock24_t gRxTimeStamp24;

uint8_t gIsrEntryCounter;
uint16_t gIsrMask;

uint8_t gCurrentPALevel;

uint8_t *gpaPowerLevelLimits;

#ifdef MEMORY_MODEL_BANKED
void __near(* __near gIsrFastAction)(void);
void __near(* __near gIsrPendingFastAction)(void);
void __near(* __near gIsrAsyncTailFunction)(void);
#else
void (* __near gIsrFastAction)(void);
void (* __near gIsrPendingFastAction)(void);
void (* __near gIsrAsyncTailFunction)(void);
#endif /* MEMORY_MODEL_BANKED */
