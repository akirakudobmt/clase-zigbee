/*****************************************************************************
* This contains Interface information for the RST module.          
*
*             
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*****************************************************************************/

#ifndef _RST_INTERFACE_H_
#define _RST_INTERFACE_H_

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/
/*****************************************************************************
* gBeeStackProject_c valid range:
* gBeeStackProject_c 0 reset management disabled
* gBeeStackProject_c 1 reset management enabled
******************************************************************************/
#ifndef gBeeStackProject_c
  #define gBeeStackProject_c 1
#endif

#if (gBeeStackProject_c == 1)

#if defined (PROCESSOR_QE128)
  /* These masks are specific for QE128 SRS register.SRS register contains the
     status of previous reset. */
     
  #define gPowerOnResetMask_c     0x82
  #define gExternalPinResetMask_c 0x40
  #define gCopResetMask_c         0x20
  #define gIllegalOpcodeMask_c    0x10  
  #define gLVDResetMask_c         0x02

  #define gResetNotOccured_c      0x00  
#elif defined PROCESSOR_MC1323X
  /* These masks are specific for MC1323x SRS register.SRS register contains the
     status of previous reset. */
  
  #define gPowerOnResetMask_c     0x82
  #define gExternalPinResetMask_c 0x40
  #define gCopResetMask_c         0x20
  #define gIllegalOpcodeMask_c    0x10  
  #define gIllegalAddressMask_c   0x08
  #define gLVDResetMask_c         0x02

  #define gResetNotOccured_c      0x00  
#else
  /* These masks are specific for HCS08 SRS register.SRS register contains the
     status of previous reset. */
  #define gPowerOnResetMask_c     0x82
  #define gExternalPinResetMask_c 0x40
  #define gCopResetMask_c         0x20
  #define gIllegalOpcodeMask_c    0x10  
  #define gICGRestMask_c          0x04
  #define gLVDResetMask_c         0x02

  #define gResetNotOccured_c      0x00      
#endif
  


/*Gives accesss to the global variable whose bits are set according to the
   reset reason detected*/
#define RST_GetResetFlag  gRSTResetStatus

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/
/*global variable whose bits are set according to the reset reason detected*/
extern uint8_t gRSTResetStatus;

/*variable in NO_INIT memory,which can be used to distinguish between the
   accidental and deliberate  illegal opcode instruction execution */
extern uint8_t gNoInitResetFlag; 


/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/
/*None*/

/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/
/*None*/

/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/
void RST_GetResetStatus(void);

#endif /*gBeeStackProject_c*/

#endif _RST_INTERFACE_H_


 
 
 
 
 