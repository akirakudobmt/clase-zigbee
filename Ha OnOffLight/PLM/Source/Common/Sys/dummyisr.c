/************************************************************************************
* This module contains the dummy ISR function
* 
*
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#include "IrqControlLib.h"
#include "EmbeddedTypes.h"

// **************************************************************************
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
INTERRUPT_KEYWORD void Default_Dummy_ISR(void)
{
   // Unimplemented ISRs trap.
   asm BGND;
}
#pragma CODE_SEG DEFAULT
// **************************************************************************
