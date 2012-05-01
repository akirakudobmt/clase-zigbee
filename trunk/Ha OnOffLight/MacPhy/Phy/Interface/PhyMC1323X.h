/************************************************************************************
* Header file for interface of the PHY layer.
* The functionality declared in this file all resides in the PHY layer of ZigBee
*
* (c) Copyright 2009, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _PHY_MC1323X_H_
#define _PHY_MC1323X_H_

/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
#include "IoConfigMC1323X.h"

/************************************************************************************
*************************************************************************************
* Public Macros and Definitions
*************************************************************************************
************************************************************************************/

#define gPhyMaxDataLength_c   127

#define PHY_IRQ_LIB_MASK  PP_PHY_CTL2_TRCV_MSK_BIT

//MC1323X: PHY CTOV modes
enum {
  ctovRxMode_c,
  ctovTxMode_c
};

//MC1323X: PHY states
enum {
  gIdle_c,
  gRX_c,
  gTX_c,
  gCCA_c,
  gTR_c,
  gCCCA_c
};

//MC1323X: channel state
enum {
  gChannelIdle_c,
  gChannelBusy_c
};

//MC1323X: PHY requests exit states
enum {
  gPhySuccess_c,
  gPhyBusy_c,
  gPhyInvalidParam_c
};

//MC1323X: PANCORDNTR bit in PP
enum {
  gMacRole_DeviceOrCoord_c,
  gMacRole_PanCoord_c
};

//MC1323X: Cca types
enum {
  gCcaED_c,            // energy detect - CCA bit not active, not to be used for T and CCCA sequences
  gCcaCCA_MODE1_c,     // energy detect - CCA bit ACTIVE
  gCcaCCA_MODE2_c,     // 802.15.4 compliant signal detect - CCA bit ACTIVE
  gInvalidCcaType_c    // illegal type
};

//MC1323X: Cca modes
enum {
  gNormalCca_c,
  gContinuousCca_c    
};

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// PhyPdDataRequest and PhyPlmeCcaEdRequest parameter bit map:
//  |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
//  slottedEn   x       x    ackReq   ccaEn  cont.En     ccaType
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// slottedEn == 1 -> slotted
// slottedEn == 0 -> unslotted
//   ackReq  == 1 -> TxRxAck
//   ackReq  == 0 -> Tx
//   ccaEn   == 1 -> CcaTx or CcaTxRxAck depending on AckReq
//   ccaEn   == 0 -> no CCA before Tx  or ED scan
//   cont.En == 1 -> Continuous CCA
//   cont.En == 0 -> normal CCA
//   ccaType == 3 -> do not use !
//   ccaType == 2 ->  CCA mode 2
//   ccaType == 1 ->  CCA mode 1
//   ccaType == 0 ->  ED

#define gSlottedEnPos_c    7
#define gAckReqPos_c       4
#define gCcaEnPos_c        3
#define gContinuousEnPos_c 2
#define gCcaTypePos_c      0

#define gSlottedEnMask_c    (1 << gSlottedEnPos_c)
#define gAckReqMask_c       (1 << gAckReqPos_c)
#define gCcaEnMask_c        (1 << gCcaEnPos_c)
#define gContinuousEnMask_c (1 << gContinuousEnPos_c)
#define gCcaTypeMask_c      (3 << gCcaTypePos_c)

// argument definitions for PhyPlmeCcaRequest()
#define gCcaReq_Continuous_Mode1_c             ((gContinuousEnMask_c)| (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Continuous_Mode2_c             ((gContinuousEnMask_c)| (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Slotted_Mode1_c                ( (gSlottedEnMask_c)  | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Slotted_Mode2_c                ( (gSlottedEnMask_c)  | (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Unslotted_Mode1_c              (                       (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Unslotted_Mode2_c              (                       (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Ed_c                           (                       (gCcaEnMask_c) | (gCcaED_c        << gCcaTypePos_c) )
#define gCcaReq_Mode1_c                        (                       (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Mode2_c                        (                       (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Continuous_Default_c           ((gContinuousEnMask_c)| (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Slotted_Default_c              ( (gSlottedEnMask_c)  | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Default_c                      (                       (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )

// argument definitions for Tx settings when calling PhyPdDataRequest()
#define gDataReq_NoAck_NoCca_Slotted_c         ( (gSlottedEnMask_c) |                                    (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_NoCca_Unslotted_c       (                                                         (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Slotted_c           ( (gSlottedEnMask_c) |                   (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Unslotted_c         (                                        (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Slotted_c           ( (gSlottedEnMask_c) | (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Unslotted_c         (                      (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Slotted_c             ( (gSlottedEnMask_c) | (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Unslotted_c           (                      (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )

#define gDataReq_NoAck_NoCca_Slotted_Mode1_c   ( (gSlottedEnMask_c) |                                    (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_NoCca_Unslotted_Mode1_c (                                                         (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Slotted_Mode1_c     ( (gSlottedEnMask_c) |                   (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Unslotted_Mode1_c   (                                        (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Slotted_Mode1_c     ( (gSlottedEnMask_c) | (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Unslotted_Mode1_c   (                      (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Slotted_Mode1_c       ( (gSlottedEnMask_c) | (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Unslotted_Mode1_c     (                      (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )

#define gDataReq_NoAck_NoCca_Slotted_Mode2_c   ( (gSlottedEnMask_c) |                                    (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_NoAck_NoCca_Unslotted_Mode2_c (                                                         (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Slotted_Mode2_c     ( (gSlottedEnMask_c) |                   (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Unslotted_Mode2_c   (                                        (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Slotted_Mode2_c     ( (gSlottedEnMask_c) | (gAckReqMask_c) |                  (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Unslotted_Mode2_c   (                      (gAckReqMask_c) |                  (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Slotted_Mode2_c       ( (gSlottedEnMask_c) | (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Unslotted_Mode2_c     (                      (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )


/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

#define PhyPlmeEdRequest()           PhyPlmeCcaEdRequest(gCcaED_c << gCcaTypePos_c)
#define PhyPlmeCcaRequest(arg)       PhyPlmeCcaEdRequest(arg)
#define PhyPpGetState()             (PP_PHY_CTL1 & PP_PHY_CTL1_XCVSEQ_MASK)
#define PhyPlmeForceTrxOffRequest()  PhyAbort()

#define PhyGetSeqState()       PhyPpGetState()

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

typedef struct phyPacket_tag {
  uint8_t frameLength;
  uint8_t data[gPhyMaxDataLength_c];
} phyPacket_t;

typedef struct phyRxParams_tag {
  zbClock24_t timeStamp;
  uint8_t     linkQuality;
} phyRxParams_t;

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

/***********************************************************************************/

void    PhyEnableBER(void);
void    PhyDisableBER(void);
void    PhyEnableTxNoModulation(void);
void    PhyDisableTxNoModulation(void);

//MAC to PHY Interface functions
//PD/PLME
uint8_t PhyPdDataRequest(phyPacket_t *pTxPacket, uint8_t phyTxMode, phyRxParams_t *pRxParams);
uint8_t PhyPlmeCcaEdRequest(uint8_t ccaType);//sets the CCA type and sets slotted or unslotted mode
uint8_t PhyPlmeSetCurrentChannelRequest(uint8_t channel);
uint8_t PhyPlmeSetPwrLevelRequest(uint8_t pwrLevel);
uint8_t PhyPlmeRxRequest(phyPacket_t *pRxData, phyRxParams_t *pRxParams);
uint8_t PhyPlmeGetCurrentChannelRequest(void);

//PHY - TIMING
void    PhyTimeSetEventTrigger(uint16_t startTime);
void    PhyTimeSetEventTimeout(zbClock24_t *pEndTime);
void    PhyTimeDisableEventTimeout(void);
void    PhyTimeReadClock(zbClock24_t *pRetClk);
void    PhyTimeInitEventTimer(zbClock24_t *pAbsTime);

//PHY - Packet Processor
void    PhyInit(void);
void    PhyAbort(void);

void    PhyPpSetPanId(uint8_t *pPanId);
void    PhyPpSetShortAddr(uint8_t *pShortAddr);
void    PhyPpSetLongAddr(uint8_t *pLongAddr);
void    PhyPpSetMacRole(bool_t macRole);
void    PhyPpSetCcaThreshold(uint8_t ccaThreshold);
void    PhyPpSetPromiscuous(bool_t mode); // TRUE - promiscuous mode, FALSE - normal behaviour

bool_t  PhyPpIsTxAckDataPending(void);    // returns pending data for ack TX frame (SRCADDR in Status1 Reg.)
bool_t  PhyPpIsRxAckDataPending(void);    // returns the value of data pending bit for RX frames (FP bit in Status2 Reg.)
bool_t  PhyPpIsPollIndication(void);      // returns the value of PI bit from STATUS1 reg
uint8_t PhyPpReadLatestIndex(void);       // returns the value of latest received packet index, if present in queue

void    PhyPp_AddToIndirect(uint8_t index, uint8_t *pPanId, uint8_t *pAddr, uint8_t AddrMode); // Adds to HW indirect queue the checksum to index pos.
void    PhyPp_RemoveFromIndirect(uint8_t index);                  // Removes from indirect queue the packet from index pos.

uint8_t PhyGetLastRxLqiValue(void);

/***********************************************************************************/

// PHY to MAC Interface functions
// these functions should be at least defined as stubs in the upper layer

//PD/PLME
void    PhyPdDataConfirm(void);
void    PhyPdDataIndication(void);
void    PhyPlmeCcaConfirm(bool_t channelInUse);
void    PhyPlmeEdConfirm(uint8_t energyLevel);
void    PhyPlmeSyncLossIndication(void);
void    PhyPlmeRxSfdDetect(void);
void    PhyPlmeUnlockTx(void);
void    PhyPlmeUnlockRx(void);
void    PhyPlmeFilterFailTx(void);
void    PhyPlmeFilterFailRx(void);

//PHY-TIMING
void    PhyTimeRxTimeoutIndication(void);
void    PhyTimeStartEventIndication(void);

/***********************************************************************************/

#pragma CODE_SEG DEFAULT

#endif /* _PHY_MC1323X_H_ */
