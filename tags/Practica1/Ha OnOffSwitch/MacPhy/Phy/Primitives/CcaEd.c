/************************************************************************************
* Handle channel assesment and energy detection
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
/************************************************************************************
* Get and convert energyLevel
*
* ED returns values between -95 dBm to about -18 dBm which are represented by decimal
* values 190 (0xbe) and 36 (0x24) respectively.
*
************************************************************************************/
uint8_t GetEnergyLevel(void){
  uint16_t tmpreg;
  uint8_t energyLevel;

  MC1319xDrv_ReadSpiSync(ABEL_ENERGYLVL_REG, &tmpreg);
  energyLevel = ((tmpreg >> cRX_LEVEL8shift) & cRX_LEVEL8mask);
  // Make dynamics of the energy level vary from 0x00-0xff
  if (energyLevel > 150) {
    energyLevel = 150; //150 = -75dBm as floor (translates to 0x00)
  }
  if (energyLevel < 23) {
    energyLevel = 23; //23 = -11.5 dBm as top (saturation)
  }
  energyLevel = 150 - energyLevel;  // 150 = -75dBm as floor (translates to 0xFF)
  /* Now double the value to "stretch" the dynamics up to 255 */
  energyLevel = energyLevel << 1;
  return energyLevel;
}



/************************************************************************************
* Start channel assesment. he action may be timer-triggered, and this is previously
* set by SetRxTxState. This is 'remembered' by the module variable 'mEnableT2Triggering'
*
* Checking on phy-state is done through asserts, therefore the return value will
* always be success!
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void PhyPlmeCcaRequest(void)
{
  mpfPendingSetup = SetupPendingCca;
  if (mPhyTxRxState==cIdle){  // Is Abel running, so that command must be pended?
      // Setup action immediately 
    SetupPendingProtected(); // Not really pending at this point, but this saves code! (clears mpfPendingSetup)
  }
}

/************************************************************************************
* Start energy detection. The action may be timer-triggered,
*
* Interface assumptions:
*
* Return value:
*   NONE
*
************************************************************************************/
void PhyPlmeEdRequest(void)
{
  mpfPendingSetup = SetupPendingEd;
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
