/************************************************************************************
* This file is owned by the platform/application specific software and controls the
* debug options of the Freescale 802.15.4 MAC/PHY.
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#ifndef _PLATFORMTOMACPHYDBGCONFIG_H_
#define _PLATFORMTOMACPHYDBGCONFIG_H_

/**** Debug configuation options BEGIN ****/

//#define SMAPI_DEBUG_LEVEL_HIGH // Enable tracing with the State Machine API
#define SMAPI_MLME_SM_DEBUG 0
#define SMAPI_MEM_LOWER_DEBUG 0
#define SMAPI_MEM_CTRL_DEBUG 0
#define SMAPI_SEQ_SUB_COMP_DEBUG 0

  // Enables debug info in the Memory Manager.
//#define MM_DEBUG 
  // Enables additional debug counters in the MAC
//#define MAC_DEBUG_COUNTERS 
//#define BEACON_TX_MASK_DEBUG
  // Enables histogram building of MEM-SEQ action setup timing.
//#define MEM_SEQ_HIST_DEBUG_ENABLED 


  // Module specific assert switches. Undefine or set
  // to ASSERT_NONE to disable asserts for a module.
#define ASSERT_NONE   0
#define ASSERT_STOP   1
#define PHY_ASSERT    ASSERT_NONE
#define SEQ_ASSERT    ASSERT_STOP
#define MEM_ASSERT    ASSERT_STOP
#define MAC_ASSERT    ASSERT_STOP
#define ASP_ASSERT    ASSERT_STOP
#define SMAPI_ASSERT  ASSERT_STOP

  // Main switch for enabling asserts on target. 
//#define ENABLE_ASSERTS

  // Enable this to make the Dbg_SetLed() macro set/clr/toggle LEDs.
//#define ENABLE_LEDS

  // Enable this to make the Dbg_SetPin() macro set/clr/toggle port bits.
  // can be used for e.g. measuring the time between events using a scope.
//#define ENABLE_PORT_PINS
  // Code profiling functionality. ENABLE_PORT_PINS must be defined.
  // NOTE:  Define only one profiling scheme at a time!
//#define CODE_PROFILING_ENABLED
//#define RXFILTER_PROFILING_ENABLED
//#define SECURITY_PROFILING_ENABLED


/***** Debug configuation options END *****/


#endif /* _PLATFORMTOMACPHYDBGCONFIG_H_ */
