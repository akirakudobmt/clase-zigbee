/************************************************************************************
* Implements Rx and Tx streaming interrupts
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#include "MacPhyGlobalHdr.h"
#include "Phy.h"
#include "MacPhy.h"


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#define mMaxRxLength_c (127)

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
void __near DummyFastIsr(void);

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

void (*pfPhyRxTxStartCallback)(void) = NULL;

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
#if !USE_INTERRUPT_TXEOF
extern void PollForTx(void);
#endif


/************************************************************************************
* First streaming interrupt
* When a streaming Rx interrupt occurs, copy length into data buffer and get timestamp
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
#ifdef I_AM_A_SNIFFER
  extern uint8_t SnifferPacketCounter;
#endif I_AM_A_SNIFFER
  
void IrqStreamingRxFirst(void)
{
  // Assert if not in rx state
  PhyAssert(mPhyTxRxState==cBusy_Rx);
  RXFILTER_PROFILING_SETPIN_3;
 
  if (pfPhyRxTxStartCallback) 
     pfPhyRxTxStartCallback();

#ifdef I_AM_A_SNIFFER
//  GREEN_LED_ON;
  // Read timestamp for reception
  MC1319xDrv_ReadSpiSync(ABEL_TIMESTAMP_MSB_REG, &(gpPhyRxData->headerLength) ); 
  MC1319xDrv_ReadSpiSync(ABEL_TIMESTAMP_LSB_REG, &(gpPhyRxData->timeStampAbelLSB1) ); 
  gpPhyRxData->timeStampMCU = TPM1C1VH; // Both bytes (H/L) needs to be read before the buffer gets unlatched.  
  gpPhyRxData->timeStampMCU = TPM1C1VL & 0x0f; // Use this element in the structure to store timestamp (only 4 bit)
  gpPhyRxData->headerLength = SnifferPacketCounter++; // Overwrite the first timestamp with counter. The first timestamp is not valid anyway
#endif I_AM_A_SNIFFER

  // Read Link Quality / Length from Abel
  MC1319xDrv_ReadSpiSyncLE(ABEL_ENERGYLVL_REG, (uint16_t*)&(gpPhyRxData->frameLength) );  // Reads both Length and LQ to buffer
  if(gpPhyRxData->frameLength < gMacMinHeaderLengthAck_c ||
     gpPhyRxData->frameLength > mMaxRxLength_c){
    // Check for illegal length. 
    // We choose 5 as minimum as for valid MAC packets,but one may argue for 2 as limited by HW. 
    // Choosing 2 would require additional check in async part for packets less than 5. 

    mpfPendingSetup = SetupPendingNop;
    SetupPendingProtected();          // Kill current Rx
    DoFastRxEofCrcInvalid();    // Restart everything like if the Crc was invalid
    RXFILTER_PROFILING_CLRPIN_3;
    return;
  }

    // Read timestamp for reception
  MC1319xDrv_ReadSpiSyncBurst(ABEL_TIMESTAMP_MSB_REG, (zbClock16_t*)&gRxTimeStamp24); // adjustment for drift and latching time (2symb) cancelled out by propagation delay (~24 us) in rx/tx chain.
  gRxTimeStamp=(zbClock16_t) gRxTimeStamp24;

    // Init "async abort flag" and "bad packet indicator flag"
  gStopAsyncRx = FALSE;

  gIsrAsyncTailFunction = IrqAsyncRxFilterEntry;
  gIsrFastAction = IrqStreamingRx;
  RXFILTER_PROFILING_CLRPIN_3;
}

/************************************************************************************
* Normal streaming Rx interrupt
* When a streaming Rx interrupt occurs, copy data word into data buffer
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void IrqStreamingRx(void)
{
  RXFILTER_PROFILING_SETPIN_3;
    // Read data from Abel (this also clears RxStream irq)
  MC1319xDrv_ReadSpiSyncLE(RX_DATA, (uint16_t*)gpPhyPacketHead);
  gpPhyPacketHead+=2;
  gPhyMacDataRxIndex+=2;
  RXFILTER_PROFILING_CLRPIN_3;
}

/************************************************************************************
* When a streaming Tx interrupt occurs, copy data word from data buffer to Abel
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void IrqStreamingTx(void)
{
  uint16_t tmpData;

    // Assert if not in tx state
  PhyAssert(mPhyTxRxState==cBusy_Tx);
  PhyAssert(gPhyMacDataTxIndex!=0);

    // Copy data from data buffer to Abel
  tmpData = gpPhyTxPacket[gPhyMacDataTxIndex] | (gpPhyTxPacket[gPhyMacDataTxIndex+1] << 8);
  MC1319xDrv_WriteSpiSync(TX_DATA, tmpData);
  
  gPhyMacDataTxIndex+=2;
  gTxRemainingLength-=2;

  if (gTxRemainingLength <= 0) {
#if !USE_INTERRUPT_TXEOF
    gIsrFastAction = DummyFastIsr;
    gIsrAsyncTailFunction = PollForTx;
#else
    gIsrFastAction = DoFastTxEof;
#endif
  }
}

#pragma CODE_SEG DEFAULT
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
