/************************************************************************************
* The MAC/PHY Platform Library contains MAC/PHY specific functionality that needs 
* very efficient execution.
*
* The primitives in the library must all be implemented as macros.
* If no processor speed/timing problems are seen with the MAC/PHY execution then
* these macros can just contain the default C-expressions.
* However, if timing problems are seen, then the default C-expressions should be
* replaced with assembly language instructions.
*
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/
#ifndef _MACPHYPLATFORMLIB_H_
#define _MACPHYPLATFORMLIB_H_

#ifndef aUnitBackoffPeriod_s
#define aUnitBackoffPeriod_s    20  // [symbols]
#endif

/************************************************************************************
* This primitive must be implemented as a macro.
*
* The macro must perform the following operation on the uint8_t variable "Length":
*
*   Length = (Length / UnitBackoff)*UnitBackoff + UnitBackoff
*
* The UnitBackoff parameter is a uint8_t variable.
*
* If no processor speed/timing problems are seen with the MAC/PHY execution then
* this macro can just contain the default C-expression:
*  #define MPPLib_RoundLengthToBackoff(Length, UnitBackoff) \ 
*    (Length=((Length / UnitBackoff)*UnitBackoff + UnitBackoff))
*
* However, if timing problems are seen, then the default C-expression should be
* replaced with assembly language.
* 
* Interface assumptions:
*   None
*   
* Return value:
*   The result of the operation must be stored back into Length.
* 
************************************************************************************/
#define MPPLib_RoundLengthToBackoff(Length, UnitBackoff) { \
          __asm CLRH; \
          __asm LDA Length; \
          __asm LDX UnitBackoff; \
          __asm DIV; \
          __asm LDX UnitBackoff; \
          __asm MUL; \
          __asm ADD UnitBackoff; \
          __asm STA Length; \
          }

/************************************************************************************
* This primitive must be implemented as a macro.
*
* The macro must perform the following operation on "Clock24" which is a pointer to
* a uint32_t variable:
*
*   remainder = (uint8_t)((0xFFFFFF & *pClock24)%aUnitBackoffPeriod_s)
*
* The aUnitBackoffPeriod_s is the 20 symbol unit backoff period in the 802.15.4
* specification.
*
* If no processor speed/timing problems are seen with the MAC/PHY execution then
* this macro can just contain the default C-expression:
*   #define MPPLib_Uint24ModUnitBackoffPeriod() \
*     (remainder = (uint8_t)((0xFFFFFF & *pClock24)%aUnitBackoffPeriod_s))
*
* However, if timing problems are seen, then the default C-expression should be
* replaced with assembly language instructions.
* 
* Interface assumptions:
*   This macro assumes that remainder and pClock24 exist.
*   
* Return value:
*   The result of the operation must be stored into remainder.
* 
************************************************************************************/
#define MPPLib_Uint24ModUnitBackoffPeriod(){\
          __asm LDA 3,X;                    \
          __asm PSHA;                       \
          __asm LDA 2,X;                    \
          __asm PSHA;                       \
          __asm LDA 1,X;                    \
          __asm LDX #aUnitBackoffPeriod_s;  \
          __asm CLRH;                       \
          __asm DIV;                        \
          __asm PULA;                       \
          __asm DIV;                        \
          __asm PULA;                       \
          __asm DIV;                        \
          __asm PSHH;                       \
          __asm PULA;                       \
          __asm STA remainder;              \
          }



/************************************************************************************
* This primitive must be implemented as a macro.
*
* The macro must perform the following operation on the uint8_t variable 
* "timeToNextXcv":
*
*   timeToNextXcv = ((timeToNextXcv-1) / aUnitBackoffPeriod_b)*aUnitBackoffPeriod_b
*                     + aUnitBackoffPeriod_b;
*
* The aUnitBackoffPeriod_s is the 20 symbol unit backoff period in the 802.15.4
* specification.
*
* If no processor speed/timing problems are seen with the MAC/PHY execution then
* this macro can just contain the default C-expression:
*   #define MPPLib_ComputeSlottedBoundary() \
*    (timeToNextXcv = ((timeToNextXcv-1) / aUnitBackoffPeriod_b)*aUnitBackoffPeriod_b\
*    + aUnitBackoffPeriod_b)
*
* However, if timing problems are seen, then the default C-expression should be
* replaced with assembly language instructions.
* 
* Interface assumptions:
*   This macro assumes that timeToNextXcv exist.
*   
* Return value:
*   The result of the operation must be stored into timeToNextXcv.
* 
************************************************************************************/
#define MPPLib_ComputeSlottedBoundary()      \
    { asm CLRH ;                       \
      asm LDA timeToNextXcv ;          \
      asm DECA ;                       \
      asm LDX #aUnitBackoffPeriod_b ;  \
      asm DIV ;                        \
      asm LDX #aUnitBackoffPeriod_b ;  \
      asm MUL ;                        \
      asm ADD #(aUnitBackoffPeriod_b); \
      asm STA timeToNextXcv ;          \
    }


#endif //_MACPHYPLATFORMLIB_H_