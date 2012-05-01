/************************************************************************************
* Implements SetTrxStateRequest Primitive.
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
#include "phy.h"
#include "PhyMacMsg.h"
#include "MacPhy.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
volatile bool_t mClockReadFlag;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
void LowLevelReadClockSync(zbClock24_t *pRetClk)
{
  /* Must be called from a place protected against Abel interrupts. */
  MC1319xDrv_ReadSpiSync(ABEL_CURR_TIME_LSB_REG,  (uint16_t*)pRetClk); // HACK: Always clear latch if set by async call. 
  MC1319xDrv_ReadSpiSyncBurst(ABEL_CURR_TIME_MSB_REG, (uint16_t*)pRetClk);
  mClockReadFlag=TRUE;    // Set semaphore
}

void LowLevelReadClockAsync(zbClock24_t *pRetClk)
{
  /* May be called from anywhere. But is slower than the one above*/
  do{
    uint16_t* pRetClkTmp = (uint16_t*)pRetClk;              // Prevent unoptimal HCS08 code through local variable
    mClockReadFlag=FALSE;                                   // Clear semaphore 
    MC1319xDrv_ReadSpiAsyncBurst(ABEL_CURR_TIME_MSB_REG, pRetClkTmp);
  } while(mClockReadFlag);                                  // Check if sync clock read interrupted async read
}

void InitRxFastAction(void){
    // Init RxStream function pointer
  gIsrFastAction = IrqStreamingRxFirst;  
}

void InitRxPointers(void)
{
  // Init rx buffer pointer and index
  gpPhyPacketHead = gpPhyRxData->rxData;
  gPhyMacDataRxIndex = gRxDataIndexOffset_c;  // Do this to comply with structure index. If removed, update filter index values!!!!!!
}



/************************************************************************************
* Restart broken Rx sequence.
*
* Interface assumptions:
*
* THIS FUNCTION MAY ONLY BE CALLED FROM THE RXSTREAM INTERRUPT OR FROM macRxFilterProcess
*
*
* Return value:
*   NONE
*
* Revision history:
*
************************************************************************************/
void ReStartRx(void)
{
    // Setup Rx action immediately 
  mpfPendingSetup = SetupImmediateRx; 
  SetupPendingProtected(); // Not really pending at this point, but this saves code! (clears mpfPendingSetup)
  gIsrAsyncTailFunction = DummyFastIsr;

#ifdef I_AM_A_SNIFFER
  if (gpRxDataToBeSentOnHostInterface != NULL) GREEN_LED_TOGGLE; // We have not emptied the previous packets (mainloop to slow)
  if (gpPhyRxData == &gRxDataBuffer) {
    gpPhyRxData = &gRxDataBuffer2; // Sets the pointer to the other buffer, so that we can empty the first buffer from the mainloop
    gpRxDataToBeSentOnHostInterface = &gRxDataBuffer; 
  }
  else {
    gpPhyRxData = &gRxDataBuffer; // ...and the other way around
    gpRxDataToBeSentOnHostInterface = &gRxDataBuffer2;
  }
#endif /*I_AM_A_SNIFFER*/
  
  InitRxFastAction();
  InitRxPointers();

#if USE_INTERRUPT_RXEOF  // 
    // Clear MC1319x Status Register   
    // and handle if RxTimeouts occured just now
    // NOTE: This will only be necessary when using the EOF interrupt for Rx. 
    // NOTE: Removed when using GPIO1 polling because of known HW issue: 
    //       Reading the status register while interrupts occur eliminates the interrupt
  {
    uint16_t retReg;
    MC1319xDrv_ReadSpiAsync(ABEL_reg24, &retReg);  // Clears all pending MC1319x interrupt 
    IrqControlLib_AckMC1319xIrq();                // Ack irqs in mcu if any latched
    if (retReg & cTMR4_IRQ) RxTimeout();          // If timeout occured just now act accordingly
  }
#else
    // HACK: When Using GPIO polling it has been observed that an Rx Stream Irq has occurred in rare circumstances on CRC invalid
    // To ensure this is always cleared perform a dummy read of the data register
  {
    uint16_t dummy; 
    MC1319xDrv_ReadSpiSync(RX_DATA, &dummy); // Read rx data register to clear RxStream irq 
    IrqControlLib_AckMC1319xIrq();          // Ack irq (clears latch in mcu if any)
  }

#endif // USE_INTERRUPT_RXEOF
 
  RXFILTER_PROFILING_CLRPIN_1;
}


/************************************************************************************
* Set event trigger. Used for timed actions.
*
* Interface assumptions:
*
*
* Return value:
*   NONE
*
************************************************************************************/
void PhySyncSetEventTrigger(zbClock16_t startTime)
{
  startTime = startTime - gAbelWarmupTimeSym_c;
  MC1319xDrv_WriteSpiSync(ABEL_TMR2_PRIME_REG, startTime); // Set new T2' timer trigger
}

/************************************************************************************
* Set event timeout. Used for timed actions.
*
* Interface assumptions:
*
*
* Return value:
*   NONE
*
************************************************************************************/
void PhySyncSetEventTimeout(zbClock24_t* pEndTime )
{ 
  MC1319xDrv_WriteSpiSyncBurst(ABEL_TMR4_HI_REG, (uint16_t*)pEndTime); // Set new TC4 timeout. This will arm the timeout!
}

/************************************************************************************
* Kill event timeout
*
* Interface assumptions:
*
*
* Return value:
*   NONE
*
************************************************************************************/
void DisableEventTimeout(void)
{
    // Disabling the timer prevents future interrupts, and clears pending interrupts in Abel.
    // Both the irq level pin and reg 0x24 (bit3) are cleared
    // In case a pending timer interrupt is cleared we must remember to "ack" the irq in the MCU 
  MC1319xDrv_WriteSpiAsync(ABEL_TMR4_HI_REG, 0x8000);
  IrqControlLib_AckMC1319xIrq();
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// Correlation threshold commands
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

#ifndef DO_NOT_TOGGLE_SYNCH_THRESHOLD
// HACK: Set threshold to normal for Rx
void SetCorrThresholdNormal(){
  MC1319xDrv_WriteSpiSync(ABEL_reg38, 0x0008); // Set correlation threshold to normal value
}

// HACK: CCA-Rx issue. Set threshold to max value for Ed and Cca to prevent correlation
void SetCorrThresholdHigh(){
  MC1319xDrv_WriteSpiSync(ABEL_reg38, 0x03FF); // Set correlation threshold to high value
}
#else //DO_NOT_TOGGLE_SYNCH_THRESHOLD
void SetCorrThresholdNormal(){
}
void SetCorrThresholdHigh(){
}
#endif //DO_NOT_TOGGLE_SYNCH_THRESHOLD

// Helper function for action setup (used to save code, not used with tx due to tight critical path)
void SetupAction(uint16_t command)
{
#if gTransceiverType_d == gMC1319x_c
  MC1319xDrv_RxTxDisable(); // We need to force the transceiver into idle state when using abel.
#endif // gTransceiverType_d == gMC1319x_c

  MC1319xDrv_WriteSpiSyncFast(ABEL_CONTROL_REG, command);

#if gTransceiverType_d == gMC1319x_c  
  MC1319xDrv_RxTxEnable();
#endif // gTransceiverType_d == gMC1319x_c
}

void SetupPendingTx(void)
{
  uint16_t command = cTX_STRM | cTX_SENT_MASK | cTMR_TRIG_EN;    
  SetupAction(command);
  MC1319xDrv_TxAntennaSwitchEnable();  
  mPhyTxRxState=cBusy_Tx;
  gIsrPendingFastAction = IrqStreamingTx; 
}

void SetupPendingRx(void)
{
  uint16_t command = cRX_RCV_ALL | cRX_STRM | cRX_RCVD_MASK | cRX_AGC | cTMR_TRIG_EN;    
  SetupAction(command);
  MC1319xDrv_RxAntennaSwitchEnable();
  mPhyTxRxState=cBusy_Rx;
  gIsrPendingFastAction = IrqStreamingRxFirst; 
}

void SetupImmediateRx(void)
{
  uint16_t command = cRX_RCV_ALL | cRX_STRM | cRX_RCVD_MASK | cRX_AGC;
  SetupAction(command);
  MC1319xDrv_RxAntennaSwitchEnable();
  mPhyTxRxState=cBusy_Rx;
}

void SetupPendingTtNop(void)
{
  uint16_t command=SEQ_NOP | cTMR_TRIG_EN;
  mPhyTxRxState=cBusy_Wait;
  SetupAction(command);
  gIsrPendingFastAction = DummyFastIsr;
}

void SetupPendingNop(void)
{
  uint16_t command=SEQ_NOP;

  MC1319xDrv_RxLnaDisable();
  MC1319xDrv_TxPaDisable();
  mPhyTxRxState=cIdle;
  SetupAction(command);
}

void SetupPendingCca(void)
{
  uint16_t command = cCCA_MASK | cCCA_MODE0 | SEQ_C | cTMR_TRIG_EN;
  SetupAction(command);
  MC1319xDrv_RxAntennaSwitchEnable();
  mPhyTxRxState = cRxCCA;
  gIsrPendingFastAction=DoFastCcaEof;
}

void SetupPendingEd(void)
{
  uint16_t command = cCCA_MASK | cCCA_ED | SEQ_C | cTMR_TRIG_EN;
  SetupAction(command);
  MC1319xDrv_RxAntennaSwitchEnable();
  mPhyTxRxState = cRxED;
  gIsrPendingFastAction=DoFastEdEof;
}

void SetupPendingProtected(void){
  IrqControlLib_DisableAllIrqs();  
  mpfPendingSetup();
  mpfPendingSetup=NULL;
  CODE_PROFILING_CLRPIN_1
  IrqControlLib_EnableAllIrqs();
}


/************************************************************************************
* Wait request 
* Setup wait as pending action if Phy is busy. Enter wait otherwise 
*
* Interface assumptions:
*
*
* Return value:
*   NONE
*
************************************************************************************/
void PhyPlmeWaitRequest(void)
{
  mpfPendingSetup = SetupPendingTtNop; 
  SetupPendingProtected();
}


/************************************************************************************
* Force Trx state in PHY layer to idle
*
* Interface assumptions:
*
*
* Return value:
*   NONE
*
************************************************************************************/
void PhyPlmeForceTrxOffRequest(void){
    mpfPendingSetup = SetupPendingNop;
    SetupPendingProtected();

    gIsrPendingFastAction=NULL;
    
      // Reset pointers and Timeout
    gIsrFastAction = DummyFastIsr;
    gIsrAsyncTailFunction = DummyFastIsr;
    DisableEventTimeout();
      
    {
        // Clear MC1319x status register... we are now IDLE!!!!!!!!
      uint16_t retReg;
      MC1319xDrv_ReadSpiAsync(ABEL_reg24, &retReg);  // Clears all pending MC1319x interrupt 
      IrqControlLib_AckMC1319xIrq();                // Ack irqs in mcu if any latched
    }
}

/************************************************************************************
* Set state in PHY layer to Rx
* Setup Rx as pending action if Phy is busy. Enter Rx otherwise 
*
* Interface assumptions:
*
*
* Return value:
*   NONE
*
************************************************************************************/
void PhyPlmeRxRequest(void){
    mpfPendingSetup = SetupPendingRx;
    if (mPhyTxRxState==cIdle){  // Is Abel running, so that command must be pended?
        // Setup action immediately 
      SetupPendingProtected(); // Not really pending at this point, but this saves code! (clears mpfPendingSetup)
    }
}

/************************************************************************************
* Set state in PHY layer to Tx
* Setup Tx as pending action if Phy is busy. Enter Tx otherwise 
*
* Interface assumptions:
*
*
* Return value:
*   NONE
*
************************************************************************************/
void PhyPlmeTxRequest(void){
    mpfPendingSetup = SetupPendingTx;
    if (mPhyTxRxState==cIdle){  // Is Abel running, so that command must be pended?
        // Setup action immediately 
      SetupPendingProtected(); // Not really pending at this point, but this saves code! (clears mpfPendingSetup)
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

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
 #pragma CODE_SEG DEFAULT
