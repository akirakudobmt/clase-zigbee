/************************************************************************************
* MC1323X: Implements PD and PLME PHY primitives
*
* (c) Copyright 2009, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
#include "IoConfig.h"

#include "PhyDebugMC1323X.h"
#include "PhyMC1323X.h"



#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

static uint8_t gPhyCurrentChannel = 0x0B;
void (*pfPhyRxTxStartCallback)(void) = NULL;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#define SUPPORTED_POWER_LEVELS      15

//IAR 0x10 settings for TX power output
uint8_t pwrOutIAR10[SUPPORTED_POWER_LEVELS] = {0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26};
//IAR 0x11 settings for TX power output
uint8_t pwrOutIAR11[SUPPORTED_POWER_LEVELS] = {0x49,0x49,0x49,0x4A,0x4A,0x4A,0x4E,0x4D,0x4D,0x4D,0x4D,0x4D,0x4C,0x4D,0x4D};
//IAR 0x20 settings for TX power output
uint8_t pwrOutIAR20[SUPPORTED_POWER_LEVELS] = {0x00,0x01,0x01,0x00,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x01,0x01};
//PP_PHY_PAPWRCTRL settings for TX power output
uint8_t  pwrOutCtrl[SUPPORTED_POWER_LEVELS] = {0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x40,0x40,0x41,0x41,0x42,0x43,0x44};

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
extern void PhyPassRxParams(phyRxParams_t * pRxParams);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

uint8_t PhyPdDataRequest(phyPacket_t *pTxPacket, uint8_t phyTxMode, phyRxParams_t *pRxParams)
{
  PHY_DEBUG_LOG(PDBG_DATA_REQUEST);
  
#ifdef PHY_PARAMETERS_VALIDATION
  // null pointer
  if(NULL == pTxPacket)
  {
    return gPhyInvalidParam_c;
  }

  // cannot have packets shorter than FCS field (2 byte)
  if(pTxPacket->frameLength < 2)
  {
    return gPhyInvalidParam_c;
  }

  // if CCA required ...
  if(gCcaEnMask_c & phyTxMode)
  { // ... cannot perform other types than MODE1 and MODE2
    if( (gCcaCCA_MODE1_c != (gCcaTypeMask_c & phyTxMode)) && (gCcaCCA_MODE2_c != (gCcaTypeMask_c & phyTxMode)) )
    {
      return gPhyInvalidParam_c;
    }
    // ... cannot perform Continuous CCA on T or TR sequences
    if(gContinuousEnMask_c & phyTxMode)
    {
      return gPhyInvalidParam_c;
    }
  }

#endif // PHY_PARAMETERS_VALIDATION

  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }
  
  PP_PHY_TXP = pTxPacket;
  
  // perform CCA before TX if required by phyTxMode parameter
  if(gCcaEnMask_c & phyTxMode)
  {
    PP_PHY_CTL1 |=  PP_PHY_CTL1_CCABFRTX_BIT;
    PP_PHY_CTL2 &= ~PP_PHY_CTL2_CCA_TYPE_MASK;
    PP_PHY_CTL2 |= ((gCcaTypeMask_c & phyTxMode) << (PP_PHY_CTL2_CCA_TYPE_POS - gCcaTypePos_c));
  }
  else
  {
    PP_PHY_CTL2 &= ~PP_PHY_CTL2_CCA_TYPE_MASK; // use the default type setting
    PP_PHY_CTL1 &= ~PP_PHY_CTL1_CCABFRTX_BIT;
  }

  // slotted operation
  if(gSlottedEnMask_c & phyTxMode)
  {
    PP_PHY_CTL1 |=  PP_PHY_CTL1_SLOTTED_BIT;
  }
  else
  {
    PP_PHY_CTL1 &= ~PP_PHY_CTL1_SLOTTED_BIT;
  }

  PP_PHY_CTL4 |=  PP_PHY_CTL4_TMR3MSK_BIT; // mask TMR3 interrupt
  PP_PHY_CTL3 &= ~PP_PHY_CTL3_SEQ_MSK_BIT; // unmask SEQ interrupt

  
  // ensure that no spurious interrupts are raised
  PP_PHY_STATUS1 =  PP_PHY_STATUS1;
  PP_PHY_STATUS2 =  PP_PHY_STATUS2;

  // perform TxRxAck sequence if required by phyTxMode
  if(gAckReqMask_c & phyTxMode)
  {
    PhyPassRxParams(pRxParams);

    PP_PHY_CTL1 |=  PP_PHY_CTL1_RXACKRQD_BIT;
    PP_PHY_CTL1 |=  gTR_c;
  }
  else
  {
    PhyPassRxParams(NULL);
    PP_PHY_CTL1 &= ~PP_PHY_CTL1_RXACKRQD_BIT;
    PP_PHY_CTL1 |=  gTX_c;
  }
  
  // Start a platform timer to prevent Transceiver lock up
  if (pfPhyRxTxStartCallback)
    pfPhyRxTxStartCallback();
	
  return gPhySuccess_c;
}

/***********************************************************************************/

uint8_t  PhyPlmeRxRequest(phyPacket_t *pRxData, phyRxParams_t *pRxParams)
{
  PHY_DEBUG_LOG(PDBG_PLME_RX_REQUEST);

#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pRxData)
  {
    return gPhyInvalidParam_c;
  }
#endif // PHY_PARAMETERS_VALIDATION

  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }

  PhyPassRxParams(pRxParams);
  PP_PHY_RXP = pRxData;

//  these should be resolved in HW
//  PP_PHY_CTL2 &= ~PP_PHY_CTL2_CCA_TYPE_MASK;
//  PP_PHY_CTL2 |= (gCcaED_c << (PP_PHY_CTL2_CCA_TYPE_POS - gCcaTypePos_c));

  PP_PHY_CTL4 |=  PP_PHY_CTL4_TMR3MSK_BIT; // mask TMR3 interrupt
  PP_PHY_CTL3 &= ~PP_PHY_CTL3_SEQ_MSK_BIT; // unmask SEQ interrupt

  // ensure that no spurious interrupts are raised
  PP_PHY_STATUS1 =  PP_PHY_STATUS1;
  PP_PHY_STATUS2 =  PP_PHY_STATUS2;

  // start the RX sequence
  PP_PHY_CTL1 |=  gRX_c;
  
  // Start a platform timer to prevent Transceiver lock up
  if (pfPhyRxTxStartCallback)
    pfPhyRxTxStartCallback();
	
  return gPhySuccess_c;
}

/***********************************************************************************/

uint8_t PhyPlmeCcaEdRequest(uint8_t ccaParam)
{
  PHY_DEBUG_LOG(PDBG_PLME_CCA_ED_REQUEST);

#ifdef PHY_PARAMETERS_VALIDATION
  // illegal CCA type
  if(gInvalidCcaType_c == (gCcaTypeMask_c & ccaParam))
  {
    return gPhyInvalidParam_c;
  }

  // cannot perform Continuous CCA using ED type
  if( (gContinuousEnMask_c & ccaParam) && (gCcaED_c == (gCcaTypeMask_c & ccaParam)) )
  {
    return gPhyInvalidParam_c;
  }

  // cannot perform ED request using other type than gCcaED_c
  if( (gCcaEnMask_c & ccaParam) && (gCcaED_c != (gCcaTypeMask_c & ccaParam)) )
  {
    return gPhyInvalidParam_c;
  }

  // cannot perform ED request using Continuous mode
  if( (gContinuousEnMask_c & ccaParam) && ((gCcaEnMask_c & ccaParam)==0) )
  {
    return gPhyInvalidParam_c;
  }

#endif // PHY_PARAMETERS_VALIDATION

  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }

  // write in CTL2 the desired type of CCA
  PP_PHY_CTL2 &= ~PP_PHY_CTL2_CCA_TYPE_MASK;
  PP_PHY_CTL2 |= ((gCcaTypeMask_c & ccaParam) << (PP_PHY_CTL2_CCA_TYPE_POS - gCcaTypePos_c));

  // slotted operation
  if(gSlottedEnMask_c & ccaParam)
  {
    PP_PHY_CTL1 |=  PP_PHY_CTL1_SLOTTED_BIT;
  }
  else
  {
    PP_PHY_CTL1 &= ~PP_PHY_CTL1_SLOTTED_BIT;
  }

  PP_PHY_CTL4 |=  PP_PHY_CTL4_TMR3MSK_BIT; // mask TMR3 interrupt
  PP_PHY_CTL3 &= ~PP_PHY_CTL3_SEQ_MSK_BIT; // unmask SEQ interrupt

  // continuous CCA
  if(gContinuousEnMask_c & ccaParam)
  {
    // start the continuous CCA sequence
    // immediately or by TC2', depending on a previous PhyTimeSetEventTrigger() call)
    PP_PHY_CTL1 |=  gCCCA_c;
    // at the end of the scheduled sequence, an interrupt will occur:
    // CCA , SEQ or TMR3
  }
  // normal CCA (not continuous)
  else
  {
    // start the CCA or ED sequence (this depends on CcaType used)
    // immediately or by TC2', depending on a previous PhyTimeSetEventTrigger() call)
    PP_PHY_CTL1 |=  gCCA_c;
    // at the end of the scheduled sequence, an interrupt will occur:
    // CCA , SEQ or TMR3
  }
  return gPhySuccess_c;
}

/***********************************************************************************/

uint8_t PhyPlmeSetCurrentChannelRequest(uint8_t channel)
{
  uint16_t nFreqTemp;

  PHY_DEBUG_LOG(PDBG_PLME_SET_CHAN_REQUEST);

#ifdef PHY_PARAMETERS_VALIDATION
  if((channel < 11) || (channel > 26)) 
  {
    return gPhyInvalidParam_c;
  }
#endif // PHY_PARAMETERS_VALIDATION

  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }
  gPhyCurrentChannel = channel;
  channel  -= 11;                          // start channel numbering from 1, not 0
  channel++;
  nFreqTemp = (channel << 5) + (channel << 3);      // multiply by 40
  PP_PHY_LO1INT   = (uint8_t)(nFreqTemp >> 8) + 74; // 2400/32-1 = 74
  PP_PHY_LO1FRACH = (uint8_t)(nFreqTemp  & 0xFF);
  PP_PHY_LO1FRACL = 1; // add 1 (channel will have 500Hz offset) to avoid spurs
  return gPhySuccess_c;
}

/***********************************************************************************/

uint8_t PhyPlmeGetCurrentChannelRequest(void)
{
  return gPhyCurrentChannel;
}

/***********************************************************************************/

/***********************************************************************************/

uint8_t PhyPlmeSetPwrLevelRequest(uint8_t pwrLevel)
{
  PHY_DEBUG_LOG(PDBG_PLME_SET_POWER_REQUEST);

  if(pwrLevel > (SUPPORTED_POWER_LEVELS - 1))
  {
    return gPhyInvalidParam_c;
  }

  IoIndirectWrite(0x10, pwrOutIAR10[pwrLevel]);
  IoIndirectWrite(0x11, pwrOutIAR11[pwrLevel]);  
  IoIndirectWrite(0x20, pwrOutIAR20[pwrLevel]);
  PP_PHY_PAPWRCTRL = pwrOutCtrl[pwrLevel];
  
  return gPhySuccess_c;
}

/***********************************************************************************/

#pragma CODE_SEG DEFAULT
