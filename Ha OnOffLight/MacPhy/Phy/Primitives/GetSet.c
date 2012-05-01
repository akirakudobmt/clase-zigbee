/************************************************************************************
* Get and Set PIB attribute values in physical layer.
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
#include "PhyMacMsg.h"

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
* Private type definitions
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
/************************************************************************************
* Configure frequency dividers in Abel from PHY layer channel. 
*
* Channels 11-26
* Abel divider pseudo table
*  0x95, 0x95, 0x95, 0x96,
*  0x96, 0x96, 0x97, 0x97,
*  0x97, 0x98, 0x98, 0x98,
*  0x99, 0x99, 0x99, 0x9a
*
* Abel numerator pseudo table
*  0x5000, 0xa000, 0xf000, 0x4000,
*  0x9000, 0xe000, 0x3000, 0x8000,
*  0xd000, 0x2000, 0x7000, 0xc000,
*  0x1000, 0x6000, 0xb000, 0x0000
*
* Conversion pseudo function:
*
*   channel   = (channel - 10)*5;
*   divider   = 0x95 + channel>>4;
*   numerator = channel << 12;
*   
* Interface assumptions:
*   Channels range from 11-26 - no range check!
*
* NOTE: When changing the channel the maximum allowed (from NWM) output level is used.
*       The application needs to explicitely call Asp_SetPowerLevel to change it again. 
*
* Return value:
*   NONE
*
************************************************************************************/

// PA level for all channels is found in NVM RAM
// Bit 4-7, is the PA level. Bit 4-5 is fine adjustment and bit 6-7 is coarse adjustment.
// (Look in MC1319x reference manual for details)
// Set default powerlevel to MAX
void PhyPlmeSetCurrentChannelRequest(uint8_t channel)
{
  uint16_t iDiv, Num, PowerLevel;
  uint8_t tmpLevel;

    // Limit channel power level if value in gCurrentPALevel is lower
  if (gpaPowerLevelLimits != NULL) {
    tmpLevel = gpaPowerLevelLimits[channel - 11];
    if (tmpLevel > gCurrentPALevel) tmpLevel = gCurrentPALevel;
  }
  else {
      // If gpaPowerLevelLimits == NULL then the limiting is disabled.
    tmpLevel = gCurrentPALevel;
  }

    // Write PA level for the channel.
  MC1319xDrv_ReadSpiSync(ABEL_reg12, &PowerLevel);
  PowerLevel &= 0xFF00;
  PowerLevel |= tmpLevel;
  MC1319xDrv_WriteSpiSync(ABEL_reg12, PowerLevel);

    // Convert from channel into divider values in Abel
  channel = channel - 10;
  channel=channel*5;
  MC1319xDrv_ReadSpiSync(ABEL_LO1DIV_REG, &iDiv);
    // Set lowest 8 bit, which are iDiv
  iDiv = (iDiv & ~cIDIV8mask);
  iDiv = (iDiv | (uint8_t)(0x95+(channel>>4)));
  MC1319xDrv_WriteSpiSync(ABEL_LO1DIV_REG, iDiv);

  channel = channel << 4;   // Shift by 4 for optimal asm 
  Num = channel << 8;
  MC1319xDrv_WriteSpiSync(ABEL_LO1NUM_REG, Num);
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
#pragma CODE_SEG DEFAULT
