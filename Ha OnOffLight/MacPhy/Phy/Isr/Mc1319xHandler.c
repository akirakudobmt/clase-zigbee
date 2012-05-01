/************************************************************************************
* This module is the PHY layer MC1319x (interrupt) handler.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

/************************************************************************************
* Includes
************************************************************************************/
#include "MacPhyGlobalHdr.h"
#include "MacPhy.h"
#include "Phy.h"
#include "Mc1319xReg.h"
#include "Mc1319xDrv.h"
#include "IrqControlLib.h"
#include "PowerManager.h"
#include "NV_Data.h"


/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/
extern bool_t gSeqInErrorState;

void SeqIsrCompletedInErrorState(void);
void SeqGenerateWakeInd(void);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

// Needed to initialize Timer1 vector
void DummyIsr(void)
{
  ;
}

void (*pTimer1Isr)(void) = DummyIsr; // Makes it possible to dynamically change interrupt vector for testing!


/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

/*************************************************************************************
* Wake up from doze or hibernate                                                     *
*************************************************************************************/
#if gAspPowerSaveCapability_d

void WakeUpIsr(void)
{
    // Don't do anything if we are already awake (ATTN IRQ will always cause
    // this ISR to be called even though the ABEL is out of pover save mode).
  if(PowerManager_GetPowerMode() != 0) {
    uint16_t ctrl2val, maskval;

    SeqGenerateWakeInd();

    PowerManager_SetPowerMode(gSeqPowerModeAwake_c);

      // Disable ATTN Irq when not in low power mode
    MC1319xDrv_ReadSpiSync(ABEL_MASK_REG, &maskval);
    maskval=maskval & (uint16_t)(~cATTN_MASK);
    MC1319xDrv_WriteSpiSync(ABEL_MASK_REG, maskval);

    MC1319xDrv_ReadSpiSync(ABEL_CONTROL2_REG, &ctrl2val);

    ctrl2val = ctrl2val & ~((uint16_t)cDOZE_EN | (uint16_t)cCLKO_DOZE_EN | (uint16_t)cHIB_EN);

#if USE_INTERRUPT_TXEOF
    ctrl2val = ctrl2val | (uint16_t) cTX_DONE_MASK;
#endif  
#if USE_INTERRUPT_RXEOF
    ctrl2val = ctrl2val | (uint16_t) cRX_DONE_MASK;
#endif  
    MC1319xDrv_WriteSpiSync(ABEL_CONTROL2_REG, ctrl2val);

    // Ensure mcu clock is stable  
    PowerManager_WaitForPowerUp();
  }
}
#endif // gAspPowerSaveCapability_d


/*************************************************************************************
* Must signal to gIsrAsyncTailFunction/Header calculation, that the calculation must *
* stop now, or receiver to be restarted.                                             *
*************************************************************************************/
void Lo1OutOfLockIsr(void)
{
    // Handle Lo1 out of lock
    // If current action is Rx (typical), merely restart Rx, ensuring that TO did not just occur. 
    // If current action is Tx, act as if everything went ok here by calling fast action TxEof code.
    // If either Ed of Cca should get Lo1 unlock, fast action is already set up for Eof 
    // and will have been called on irq H-L, so ignore here!
  if (mPhyTxRxState==cBusy_Rx)
    RxTimeout();
  if (mPhyTxRxState==cBusy_Tx) 
  {
    MC1319xDrv_ReadSpiSync(ABEL_CURR_TIME_LSB_REG, &gRxTimeStamp);
    gRxTimeStamp = gRxTimeStamp - 2*aAckUnslottedSize_b;
    DoFastTxEof();
  }
}

/*************************************************************************************
* This is where everything happens...  Abel driver runs through here                 *
*************************************************************************************/


INTERRUPT_KEYWORD void MC1319xInterrupt(void)
{
  uint8_t *pTmp;

  RXFILTER_PROFILING_SETPIN_0;

  gIsrEntryCounter++;
  gIsrMask = 0xFFFF;
  
  IrqControlLib_DisableMC1319xIrq();  // Disable MC1319x irqs to avoid reentrance
  IrqControlLib_EnableAllIrqs();      // Enable other MCU irqs (e.g. application irqs). 

    // Execute predicted irq action
  (gIsrFastAction)();
  IrqControlLib_AckMC1319xIrq();      // Interrupt acknowledge, for clearing the irq register when gIsrFastAction=RxStreaming..

  if(IrqControlLib_IsMC1319xIrqAsserted())  //IRQF test
  {
    uint16_t retReg;
    MC1319xDrv_ReadSpiSync(ABEL_reg24, &retReg);
    IrqControlLib_AckMC1319xIrq();   //IACK, Could be initiated by new/next edge.
    retReg = retReg & gIsrMask;

    if (retReg & 0xFF00)
    {
      gStopAsyncRx = TRUE;         // If interrupt other than streaming Rx occured kill filter
      // LOCK_IRQ
      if (retReg & cLO_LOCK_IRQ)
      {
        Lo1OutOfLockIsr(); 
        if (retReg & cTMR4_IRQ) 
          RxTimeout();
        goto IsrExit;
      }
      

      // RX_DONE
      if (retReg & cRX_DONE)
      {
              
#if USE_INTERRUPT_RXEOF
        PhyPlmeSyncLossIndication();
        goto IsrExit;
#else
        DummyIsr();
#endif        
      }

      // TX_DONE
      if (retReg & cTX_DONE)
      {         
      
#if USE_INTERRUPT_TXEOF      
        DoFastTxEof();
        goto IsrExit;
        /*PhyPlmeSyncLossIndication();*/ //HACK for Timeout in TxAck 
#else
        DummyIsr();
#endif        
       } 

      // TMR1
      if (retReg & cTMR1_IRQ)
        pTimer1Isr();
      
#if gAspPowerSaveCapability_d
      // ResetATTN
      if (retReg & cResetATTN_IRQ)
        WakeUpIsr();
      
      // DOZE
      if (retReg & cDOZE_IRQ)      
        WakeUpIsr();
      
#endif // gAspPowerSaveCapability_d

      // STRM_DATA_ERR
      if (retReg & cSTRM_DATA_ERR)
      {                
        if(mPhyTxRxState == cBusy_Rx)
        {
          RxTimeout();
          goto IsrExit;
        } 
        else 
        {
          PhyPlmeSyncLossIndication();
          goto IsrExit;
        }
      }
      
      // HG
      if (retReg & cHG_IRQ) 
        DummyIsr();
    }

    if (retReg & 0x00FC)
    {
      gStopAsyncRx = TRUE;         // If interrupt other than streaming Rx occured kill filter
      
      // CCA
      if (retReg & cCCA_IRQ)
        DummyIsr(); // Ignore CCA irq here. Fast action catches it. (Otherwise ignore the failure)
        
      // TMR2
      if (retReg & cTMR2_IRQ)
        gpTimer2Isr();
                 
      // TMR3
      if (retReg & cTMR3_IRQ)
      {
        RxTimeout();  
        goto IsrExit;      
      }
                        
      // TMR4      
      if (retReg & cTMR4_IRQ)
      {          
          RxTimeout(); 
          goto IsrExit;          
       }  
                                    
      // RX_RCVD
      if (retReg & cRX_RCVD_IRQ)
      {
      	// Don't do anything about it while ED scanning
		    if (cRxED != mPhyTxRxState)
		    {		      
			RxTimeout();
      		goto IsrExit;
		    }
      }
      
      // TX_SENT
      if (retReg & cTX_SENT_IRQ)
      {
        PhyPlmeSyncLossIndication();
        goto IsrExit;
      }
    }
  }

  gIsrAsyncTailFunction();
  
  
IsrExit:
    // Assume that interrupts are DISABLED at this point!
  pTmp = &gIsrEntryCounter;
  if (!(--*pTmp)){
    if (gSeqInErrorState){ 
      SeqIsrCompletedInErrorState();
    }
  }
  IrqControlLib_DisableAllIrqs();   // Disable all interrupts from mcu
  IrqControlLib_EnableMC1319xIrq(); // Reenable MC1319xIrqs

  RXFILTER_PROFILING_CLRPIN_0;
}
#pragma CODE_SEG DEFAULT
