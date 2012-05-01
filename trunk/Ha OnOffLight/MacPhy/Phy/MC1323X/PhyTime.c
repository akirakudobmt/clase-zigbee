/************************************************************************************
* MC1323X: Implements PHY timing functionalities
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


/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

/***********************************************************************************/

void PhyTimeSetEventTrigger(uint16_t startTime)
{
  PP_PHY_CTL4    &= ~PP_PHY_CTL4_TMR2CMP_EN_BIT; // disable TMR2 compare
  PP_PHY_CTL1    |=  PP_PHY_CTL1_TMRTRIGEN_BIT;  // enable autosequence start by TC2 match
  PP_PHY_TC2PRIME =  startTime;
  PP_PHY_CTL4    |=  PP_PHY_CTL4_TMR2CMP_EN_BIT; // enable TMR2 compare
  PP_PHY_CTL4    &= ~PP_PHY_CTL4_TMR2MSK_BIT;    // unmask TMR2 interrupt
  PHY_DEBUG_LOG2(PDBG_SET_EVENT_TRIGGER, PP_PHY_TC2PRIMEH, PP_PHY_TC2PRIMEL);
}

/***********************************************************************************/

void PhyTimeSetEventTimeout(zbClock24_t *pEndTime)
{
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pEndTime)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION
  PP_PHY_CTL4   &= ~PP_PHY_CTL4_TMR3CMP_EN_BIT; // disable TMR3 compare
  PP_PHY_CTL2   |=  PP_PHY_CTL2_TC3TMOUT_BIT;   // enable autosequence stop by TC3 match
  PP_PHY_T3CMP0  =  *(((uint8_t *)pEndTime)+3);
  PP_PHY_T3CMP8  =  *(((uint8_t *)pEndTime)+2);
  PP_PHY_T3CMP16 =  *(((uint8_t *)pEndTime)+1);
  PP_PHY_CTL4   |=  PP_PHY_CTL4_TMR3CMP_EN_BIT; // enable TMR3 compare
  PP_PHY_CTL4   &= ~PP_PHY_CTL4_TMR3MSK_BIT;    // unmask TMR3 interrupt
  PHY_DEBUG_LOG3(PDBG_SET_EVENT_TIMEOUT, PP_PHY_T3CMP16, PP_PHY_T3CMP8, PP_PHY_T3CMP0);
}

/***********************************************************************************/

void PhyTimeDisableEventTimeout(void)
{
  PHY_DEBUG_LOG(PDBG_DISABLE_EVENT_TIMEOUT);
  PP_PHY_CTL2   &= ~PP_PHY_CTL2_TC3TMOUT_BIT;
  PP_PHY_CTL4   &= ~PP_PHY_CTL4_TMR3CMP_EN_BIT;
  PP_PHY_STATUS1 =  PP_PHY_STATUS1_TMR3_IRQ;
}

/***********************************************************************************/

void PhyTimeReadClock(zbClock24_t *pRetClk)
{
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pRetClk)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION
  PHY_DEBUG_LOG(PDBG_READ_CLOCK);
  *(((uint8_t *)pRetClk)+3) = PP_PHY_EVENTTMR0;
  *(((uint8_t *)pRetClk)+2) = PP_PHY_EVENTTMR8;
  *(((uint8_t *)pRetClk)+1) = PP_PHY_EVENTTMR16;
  *(((uint8_t *)pRetClk)+0) = 0;
}

/***********************************************************************************/

void PhyTimeInitEventTimer(zbClock24_t *pAbsTime)
{
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pAbsTime)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION
  PHY_DEBUG_LOG(PDBG_INIT_EVENT_TIMER);
  PP_PHY_T1CMP0  = *(((uint8_t *)pAbsTime)+3);
  PP_PHY_T1CMP8  = *(((uint8_t *)pAbsTime)+2);
  PP_PHY_T1CMP16 = *(((uint8_t *)pAbsTime)+1);
  PP_PHY_CTL2   |= PP_PHY_CTL2_TMRLOAD_BIT;   // self clearing bit
}

/***********************************************************************************/

#pragma CODE_SEG DEFAULT


