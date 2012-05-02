/************************************************************************************
* End of frames are handled here. Called from TxDone and RxDone interrupts in Abel.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#include "MacPhyGlobalHdr.h"
#include "Mc1319xDrv.h"
#include "IrqControlLib.h"
#include "Phy.h"
#include "PhyMacMsg.h"
#include "MacPhy.h"

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#if !(USE_INTERRUPT_RXEOF && USE_INTERRUPT_TXEOF)
#define POLL_UNTIL_IDLE {while(MC1319xDrv_IsActive());}
#endif 

#if USE_INTERRUPT_TXEOF
#define DELAY_5US 
#else
#define DELAY_5US { {uint8_t i=10; do {} while(--i);}  }
#endif

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
#if gSchedulerIntegration_d 
extern tsTaskID_t gMacTaskID_c; 
#else // gSchedulerIntegration_d 
extern bool_t gPanIdConflict; 
#endif // gSchedulerIntegration_d 

extern uint8_t gLowLatencyTX;

#ifdef I_AM_A_SNIFFER
extern rxPacket_t gRxDataBuffer;
rxPacket_t gRxDataBuffer2;
rxPacket_t * gpRxDataToBeSentOnHostInterface  = NULL;  
uint8_t SnifferPacketCounter = 0;
#endif I_AM_A_SNIFFER

void (*pfPhyRxTxEndCallback)(void) = NULL;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* Fast Eof interrupt routine for Ed.
*
* Interface assumptions:
*   Fast action setup
*
* Return value:
*   NONE
*
************************************************************************************/
void DoFastEdEof(void)
{
  uint8_t energyLevel;

  MC1319xDrv_RxLnaDisable();
  CODE_PROFILING_SETPIN_0;
  mPhyTxRxState=cIdle;
  gIsrFastAction = DummyFastIsr;

  energyLevel=GetEnergyLevel();

  PhyPlmeEdConfirm(energyLevel);
  gIsrMask = (uint16_t) ~cCCA_IRQ;
}

/************************************************************************************
* Fast Eof interrupt routine for CCA
*
* Interface assumptions:
*   Fast action setup
*
* Return value:
*   NONE
*
************************************************************************************/
void DoFastCcaEof(void)
{
  register bool_t channel_busy;
  MC1319xDrv_RxLnaDisable();
  mPhyTxRxState=cIdle;

  channel_busy = MC1319xDrv_IsCcaBusy();
  if (channel_busy){
    if(gLowLatencyTX == 1) SetupPendingCca();//When using Low Latency TX, program new CCA
    mpfPendingSetup = NULL; // Prevent Tx action setup when channel busy
    CODE_PROFILING_SETPIN_0;
  }
  if (mpfPendingSetup) {
    SetupPendingProtected();
  }

  gIsrFastAction = DummyFastIsr;
  PhyPlmeCcaConfirm(channel_busy);  // Pass message 
  gIsrMask = (uint16_t) ~cCCA_IRQ;
}



/************************************************************************************
* In order to speed up EOF generation, it is possible to poll Abel GPIO's. This
* busy-waiting scheme is only suggested if timing does not allow use of interrupt!
*
* After calling this function, the TxEof routine is automatically exectuted
* (transparent to the system whether from poll or interrupt).
*
* Interface assumptions:
*   Fast action setup
*
* Return value:
*   NONE
*
************************************************************************************/
void DoFastTxEof(void)
{
    // Enter here on expected EOF
    // Assert if not in tx state
  if (mPhyTxRxState != cBusy_Tx) return;  // EOF assumed to be generated due to action abort by sequential action
    // Set state to idle
  mPhyTxRxState=cIdle;
  gIsrFastAction = DummyFastIsr;

  if (mpfPendingSetup) {
    SetupPendingProtected();
  }
  else{
    CODE_PROFILING_SETPIN_0;
    if(IsCurrentActionAutoRx()){
      mpfPendingSetup = SetupImmediateRx; // Fall back to Rx if in Auto Rx mode
      SetupPendingProtected();
    }
  }
    // Disable PA if present
  MC1319xDrv_TxPaDisable();   // Allowed after action setup, as Tx-Tx is not possible

    // Send Data.confirm to MAC
  PhyPdDataConfirm();
  gIsrMask = (uint16_t) ~cTX_DONE;
}


/************************************************************************************
* In order to speed up EOF generation, it is possible to poll Abel GPIO's. This
* busy-waiting scheme is only suggested if timing does not allow use of interrupt!
*
* These two functions serve as the fast actions on Eof. The functions are called 
* according to the status of the crc. (Set up in super fast action).
* In the busy wait scenario these functions are called directly from the polling routine. 
*
* Interface assumptions:
*   Fast action 
*
* Return value:
*   NONE
*
************************************************************************************/
void DoFastRxEofCrcValid(void)
{
  gIsrAsyncTailFunction = DummyFastIsr;
  gIsrFastAction = DummyFastIsr;

  if(IsCurrentActionAutoRx()){
    gIsrMask = (uint16_t) ~(cRX_DONE);
  }
  else {
    DisableEventTimeout();
    gIsrMask = (uint16_t) ~(cRX_DONE | cTMR4_IRQ);
  }
  ConvertLinkQuality();
  PhyPdDataIndication();
}


void DoFastRxEofCrcInvalid(void)
{
  RestartRxOrTimeout();                               // Handle CRC failure the same way as when aborting Rx in Filter
  gIsrMask = (uint16_t) ~(cRX_DONE | cTMR2_IRQ | cRX_RCVD_IRQ);  // cRX_RCVD_IRQ added in case function called from RxStreamIsr
}


/************************************************************************************
* In order to speed up EOF generation, it is possible to poll Abel GPIO's. This
* busy-waiting scheme is only suggested if timing does not allow use of interrupt!
*
* Since the Eof on Rx is delayed we need to setup a pending action ASAP in the Eof 
* interrupt.   
*
* Interface assumptions:
*   Fast action setup with.
*
* Return value:
*   NONE
*
************************************************************************************/
#ifndef I_AM_A_SNIFFER
void DoFastRxEof(void)
{
  MC1319xDrv_RxLnaDisable();
  
  if (pfPhyRxTxEndCallback)
     pfPhyRxTxEndCallback();

  if(MC1319xDrv_IsCrcOk()){
    if (gBeaconWithBadSrcDetected) {
      gBeaconWithBadSrcDetected = FALSE;
#if gSyncReqLossIndPanIdConflictCapability_d    
      if (gpPhyRxData->rxData[gpPhyRxData->headerLength + 1] & 0x40) { // chech PAN coordinator bit in beacon superframe spec. (bit 14)
#if gSchedulerIntegration_d
        TS_SendEvent(gMacTaskID_c, gMacEventPanIdConflict_c);
#else // gSchedulerIntegration_d
        gPanIdConflict= TRUE;
#endif // gSchedulerIntegration_d
        IrqControlLib_EnableAllIrqs();
        DoFastRxEofCrcInvalid();
        return;
      }
#endif gSyncReqLossIndPanIdConflictCapability_d      
    }
    if (mpfPendingSetup) {
      SetupPendingProtected(); // disables and enables all interrupts
    }
    else{
      CODE_PROFILING_SETPIN_0;
      IrqControlLib_EnableAllIrqs(); 
      mPhyTxRxState = cIdle;
      if(IsCurrentActionAutoRx()){
        // Fall back to Rx if in Auto Rx mode
        mpfPendingSetup = SetupImmediateRx;
        SetupPendingProtected(); // disables and enables all interrupts
      }
    }
    DoFastRxEofCrcValid();
  }
  else{
    IrqControlLib_EnableAllIrqs(); 
    DoFastRxEofCrcInvalid();
  }
}

#else
void DoFastRxEof(void)
{
  if(MC1319xDrv_IsCrcOk()){
    gpPhyRxData->timeStampMCU |= 0x80; // Set bit7 in this element if CRC is ok
  }
  else {
    gpPhyRxData->timeStampMCU &= ~0x80; // Clear bit7 in this element if CRC failed
  }
  IrqControlLib_EnableAllIrqs(); 
  DoFastRxEofCrcInvalid();
}
#endif I_AM_A_SNIFFER


#if USE_INTERRUPT_RXEOF
/************************************************************************************
* Tail function when using Eof interrupt
* Polls for packet completion and initialises super-fast action to occur at first 
* coming interrupt (assumed eof interrupt).
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void PollForRx(void)
{
  EXIT_IF_RXINDEX_NOT_REACHED((uint8_t)(gpPhyRxData->frameLength-2));
  gIsrAsyncTailFunction = DummyFastIsr;
  gIsrFastAction=DoFastRxEof;
}

#else /* USE_INTERRUPT_RXEOF */
/************************************************************************************
* Tail polling function when NOT using Eof interrupt
* Polls for packet completion using GPIO and sets up pending actions
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void PollForRx(void)
{
  EXIT_IF_RXINDEX_NOT_REACHED((uint8_t)(gpPhyRxData->frameLength-2));

  POLL_UNTIL_IDLE;

  MC1319xDrv_RxLnaDisable();
  IrqControlLib_DisableAllIrqs(); // ensure nothing influences the delay after polling is done
  // Short delay (5 us) inserted to ensure Abel Sequencer stable writing Abel Ctrl register. Prevents GPIO1 held low through Tx.
  DELAY_5US // Empty Macro if using TxEOF irq
    
  DoFastRxEof();  // Enables all interrupts inside
}
#endif /* USE_INTERRUPT_RXEOF */


#if !USE_INTERRUPT_TXEOF
/************************************************************************************
* Tail polling function when NOT using Eof interrupt
* Polls for packet completion using GPIO and sets up pending actions
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void PollForTx(void)
{
  gIsrAsyncTailFunction = DummyFastIsr;
  POLL_UNTIL_IDLE;
  DoFastTxEof();
}
#endif /* USE_INTERRUPT_TXEOF */


/************************************************************************************
* Helper function for aborting Phy action upon timeout
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
#ifndef ALLOW_TX_ABORT_ON_TIMEOUT
void PhyAbortAndSendTimeoutInd(void){
  PhyPlmeForceTrxOffRequest();
  PhyPlmeRxTimeoutIndication();
}
#endif // ALLOW_TX_ABORT_ON_TIMEOUT


/************************************************************************************
* If Timeout occurs, generate event to MAC layer
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void RxTimeout(void)
{
  MC1319xDrv_RxLnaDisable();
  CODE_PROFILING_SETPIN_0;
    // We have received a timeout on a phy (Rx) action. On TMR4 (timeout) received, the PHY is forced idle
#ifndef ALLOW_TX_ABORT_ON_TIMEOUT
  if (mPhyTxRxState == cBusy_Tx) {
     gIsrFastAction = PhyAbortAndSendTimeoutInd;   // HACK: Timeout in TxAck must only abort Tx synchronously 
   }
   else{
     PhyAbortAndSendTimeoutInd();
   }
#else //ALLOW_TX_ABORT_ON_TIMEOUT
  PhyPlmeForceTrxOffRequest();
  PhyPlmeRxTimeoutIndication();
#endif //ALLOW_TX_ABORT_ON_TIMEOUT

}


/************************************************************************************
* If TMR2 expires, then it is a TC2' interrupt. If exiting doze mode, the TC2
* timer interrupt is also used, but the vector must point to ExitDoze.
*
* Sanity check: If TC2' occurs, and no action has been setup, then it is assumed that
* we are out of sync.
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void IsrTx2prime(void)
{
  CODE_PROFILING_CLRPIN_0

    // If gIsrPendingFastAction is NULL, TC2' expired, before new command was programmed
  if (NULL==gIsrPendingFastAction){
    PhyPlmeSyncLossIndication();
    return;
  }

  gIsrFastAction=gIsrPendingFastAction;
  gIsrPendingFastAction=NULL; // Clear pending setup

  PhyPlmeB2BIndication(); // Indicate that new TC2' is ready for setup
  if (gIsrFastAction==IrqStreamingRxFirst)
    InitRxPointers();
}

#pragma CODE_SEG DEFAULT
/************************************************************************************
*************************************************************************************
* Module debug stuff
*************************************************************************************
************************************************************************************/




/************************************************************************************
*************************************************************************************
* Level 1 block comment
*************************************************************************************
************************************************************************************/

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// Level 2 block comment
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

/* Level 3 block comment */




// Delimiters

/***********************************************************************************/

//-----------------------------------------------------------------------------------
