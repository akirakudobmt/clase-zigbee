/************************************************************************************
* The global include file for the MAC and PHY stack.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _MACPHYGLOBALHDR_H_
#define _MACPHYGLOBALHDR_H_

#include "EmbeddedTypes.h"
#include "AppToMacPhyConfig.h"
#include "PlatformToMacPhyConfig.h"
#include "PlatformToMacPhyDbgConfig.h"
#include "Debug.h"
#include "FunctionalityDefines.h"

/************************************************************************************
*************************************************************************************
* Code profiling functionality
*************************************************************************************
************************************************************************************/

#ifdef RXFILTER_PROFILING_ENABLED
  #define RXFILTER_PROFILING_ENABLE_PINS Dbg_EnablePins()
  #define RXFILTER_PROFILING_SETPIN_0    Dbg_SetPin(gDbgPin0_c, gDbgOn_c)
  #define RXFILTER_PROFILING_SETPIN_1    Dbg_SetPin(gDbgPin1_c, gDbgOn_c)
  #define RXFILTER_PROFILING_SETPIN_2    Dbg_SetPin(gDbgPin2_c, gDbgOn_c)
  #define RXFILTER_PROFILING_SETPIN_3    Dbg_SetPin(gDbgPin3_c, gDbgOn_c)

  #define RXFILTER_PROFILING_CLRPIN_0    Dbg_SetPin(gDbgPin0_c, gDbgOff_c)
  #define RXFILTER_PROFILING_CLRPIN_1    Dbg_SetPin(gDbgPin1_c, gDbgOff_c)
  #define RXFILTER_PROFILING_CLRPIN_2    Dbg_SetPin(gDbgPin2_c, gDbgOff_c)
  #define RXFILTER_PROFILING_CLRPIN_3    Dbg_SetPin(gDbgPin3_c, gDbgOff_c)

  #define RXFILTER_PROFILING_TGLPIN_0    Dbg_SetPin(gDbgPin0_c, gDbgToggle_c)
  #define RXFILTER_PROFILING_TGLPIN_1    Dbg_SetPin(gDbgPin1_c, gDbgToggle_c)
  #define RXFILTER_PROFILING_TGLPIN_2    Dbg_SetPin(gDbgPin2_c, gDbgToggle_c)
  #define RXFILTER_PROFILING_TGLPIN_3    Dbg_SetPin(gDbgPin3_c, gDbgToggle_c)
#else
  #define RXFILTER_PROFILING_ENABLE_PINS
  #define RXFILTER_PROFILING_SETPIN_0
  #define RXFILTER_PROFILING_SETPIN_1
  #define RXFILTER_PROFILING_SETPIN_2
  #define RXFILTER_PROFILING_SETPIN_3

  #define RXFILTER_PROFILING_CLRPIN_0
  #define RXFILTER_PROFILING_CLRPIN_1
  #define RXFILTER_PROFILING_CLRPIN_2
  #define RXFILTER_PROFILING_CLRPIN_3

  #define RXFILTER_PROFILING_TGLPIN_0
  #define RXFILTER_PROFILING_TGLPIN_1
  #define RXFILTER_PROFILING_TGLPIN_2
  #define RXFILTER_PROFILING_TGLPIN_3
#endif /* RXFILTER_PROFILING_ENABLED */


#ifdef CODE_PROFILING_ENABLED
  #define CODE_PROFILING_ENABLE_PINS  Dbg_EnablePins() 
  #define CODE_PROFILING_SETPIN_0     Dbg_SetPin(gDbgPin0_c, gDbgOn_c) 
  #define CODE_PROFILING_SETPIN_1     Dbg_SetPin(gDbgPin1_c, gDbgOn_c) 
  #define CODE_PROFILING_SETPIN_2     Dbg_SetPin(gDbgPin2_c, gDbgOn_c) 
  #define CODE_PROFILING_SETPIN_3     Dbg_SetPin(gDbgPin3_c, gDbgOn_c) 

  #define CODE_PROFILING_CLRPIN_0     Dbg_SetPin(gDbgPin0_c, gDbgOff_c) 
  #define CODE_PROFILING_CLRPIN_1     Dbg_SetPin(gDbgPin1_c, gDbgOff_c) 
  #define CODE_PROFILING_CLRPIN_2     Dbg_SetPin(gDbgPin2_c, gDbgOff_c) 
  #define CODE_PROFILING_CLRPIN_3     Dbg_SetPin(gDbgPin3_c, gDbgOff_c) 

  #define CODE_PROFILING_TGLPIN_0     Dbg_SetPin(gDbgPin0_c, gDbgToggle_c) 
  #define CODE_PROFILING_TGLPIN_1     Dbg_SetPin(gDbgPin1_c, gDbgToggle_c) 
  #define CODE_PROFILING_TGLPIN_2     Dbg_SetPin(gDbgPin2_c, gDbgToggle_c) 
  #define CODE_PROFILING_TGLPIN_3     Dbg_SetPin(gDbgPin3_c, gDbgToggle_c) 
#else
  #define CODE_PROFILING_ENABLE_PINS 
  #define CODE_PROFILING_SETPIN_0 
  #define CODE_PROFILING_SETPIN_1 
  #define CODE_PROFILING_SETPIN_2 
  #define CODE_PROFILING_SETPIN_3 

  #define CODE_PROFILING_CLRPIN_0 
  #define CODE_PROFILING_CLRPIN_1 
  #define CODE_PROFILING_CLRPIN_2 
  #define CODE_PROFILING_CLRPIN_3 

  #define CODE_PROFILING_TGLPIN_0 
  #define CODE_PROFILING_TGLPIN_1 
  #define CODE_PROFILING_TGLPIN_2 
  #define CODE_PROFILING_TGLPIN_3 
#endif /* CODE_PROFILING_ENABLED */

  // Used for measuring timing for security
#ifdef SECURITY_PROFILING_ENABLED
  #define SECURITY_PROFILING_ENABLE_PINS  Dbg_EnablePins() 
  #define SECURITY_PROFILING_SETPIN_0     Dbg_SetPin(gDbgPin0_c, gDbgOn_c) 
  #define SECURITY_PROFILING_SETPIN_1     Dbg_SetPin(gDbgPin1_c, gDbgOn_c) 
  #define SECURITY_PROFILING_SETPIN_2     Dbg_SetPin(gDbgPin2_c, gDbgOn_c) 
  #define SECURITY_PROFILING_SETPIN_3     Dbg_SetPin(gDbgPin3_c, gDbgOn_c) 

  #define SECURITY_PROFILING_CLRPIN_0     Dbg_SetPin(gDbgPin0_c, gDbgOff_c) 
  #define SECURITY_PROFILING_CLRPIN_1     Dbg_SetPin(gDbgPin1_c, gDbgOff_c) 
  #define SECURITY_PROFILING_CLRPIN_2     Dbg_SetPin(gDbgPin2_c, gDbgOff_c) 
  #define SECURITY_PROFILING_CLRPIN_3     Dbg_SetPin(gDbgPin3_c, gDbgOff_c) 

  #define SECURITY_PROFILING_TGLPIN_0     Dbg_SetPin(gDbgPin0_c, gDbgToggle_c) 
  #define SECURITY_PROFILING_TGLPIN_1     Dbg_SetPin(gDbgPin1_c, gDbgToggle_c) 
  #define SECURITY_PROFILING_TGLPIN_2     Dbg_SetPin(gDbgPin2_c, gDbgToggle_c) 
  #define SECURITY_PROFILING_TGLPIN_3     Dbg_SetPin(gDbgPin3_c, gDbgToggle_c) 
#else
  #define SECURITY_PROFILING_ENABLE_PINS
  #define SECURITY_PROFILING_SETPIN_0
  #define SECURITY_PROFILING_SETPIN_1
  #define SECURITY_PROFILING_SETPIN_2
  #define SECURITY_PROFILING_SETPIN_3

  #define SECURITY_PROFILING_CLRPIN_0
  #define SECURITY_PROFILING_CLRPIN_1
  #define SECURITY_PROFILING_CLRPIN_2
  #define SECURITY_PROFILING_CLRPIN_3

  #define SECURITY_PROFILING_TGLPIN_0
  #define SECURITY_PROFILING_TGLPIN_1
  #define SECURITY_PROFILING_TGLPIN_2
  #define SECURITY_PROFILING_TGLPIN_3
#endif /* SECURITY_PROFILING_ENABLED */

#if PHY_ASSERT == ASSERT_STOP
  #define PhyAssert(regexp) Dbg_Assert(regexp)
#else
  #define PhyAssert(regexp)
#endif // PHY_ASSERT == ASSERT_STOP

#if MAC_ASSERT == ASSERT_STOP
  #define MacAssert(regexp) Dbg_Assert(regexp)
#else
  #define MacAssert(regexp)
#endif // MAC_ASSERT == ASSERT_STOP

#if SEQ_ASSERT == ASSERT_STOP
  #define SeqAssert(regexp) Dbg_Assert(regexp)
#else
  #define SeqAssert(regexp)
#endif // Seq_ASSERT == ASSERT_STOP

#if MEM_ASSERT == ASSERT_STOP
  #define MemAssert(regexp) Dbg_Assert(regexp)
#else
  #define MemAssert(regexp)
#endif // MEM_ASSERT == ASSERT_STOP

#if ASP_ASSERT == ASSERT_STOP
  #define AspAssert(regexp) Dbg_Assert(regexp)
#else
  #define AspAssert(regexp)
#endif // ASP_ASSERT == ASSERT_STOP

#if SMAPI_ASSERT == ASSERT_STOP
  #define SmApiAssert(regexp) Dbg_Assert(regexp)
#else
  #define SmApiAssert(regexp)
#endif // SMAPI_ASSERT == ASSERT_STOP

#endif /* _MACPHYGLOBALHDR_H_ */
