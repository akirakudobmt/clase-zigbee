/************************************************************************************
* This file holds type definitions that maps the standard c-types into types
* with guaranteed sizes. The types are target/platform specific and must be edited
* for each new target/platform.
*
* The header file also provides definitions for TRUE, FALSE and NULL.
*
* (c) Copyright 2008, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*************************************************************************************/

#ifndef _EMBEDDEDTYPES_H_
#define _EMBEDDEDTYPES_H_

#define ROM_PATCH_TRIGGER(func)
#define gRomBuild_d 0

typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;

/* used for indexing into an array in the most efficient manner for the platform */
typedef uint8_t index_t;
#define gMaxIndex_c 0xfe
typedef signed short intn16_t;
typedef unsigned short uintn16_t;
typedef signed char intn8_t;
typedef unsigned char uintn8_t;
typedef signed long intn32_t;
typedef unsigned long uintn32_t;

typedef uint8_t bool_t;
typedef uintn8_t booln_t;

#define TRUE 1
#define FALSE 0

#ifndef NULL
#define NULL (( void * )( 0 ))
#endif

typedef uint32_t zbClock24_t;
typedef uint16_t zbClock16_t;

/* common macros to reduce code size in S08 */
#define IsEqual2Bytes(aVal1, aVal2) (*((uint16_t *)(aVal1)) == *((uint16_t *)(aVal2)))
#define Copy2Bytes(aVal1, aVal2) (*((uint16_t *)(aVal1)) = *((uint16_t *)(aVal2)))
#define Set2Bytes(aVal1, iVal2) (*((uint16_t *)(aVal1)) = (iVal2))
#define IsEqual2BytesInt(aVal1, iVal2) (*((uint16_t *)(aVal1)) == (iVal2))
#define TwoBytesToUint16(aVal)  ( *((uint16_t *)(aVal)) )
#define FourBytesToUint32(aVal)     ( *((uint32_t *)(aVal)) ) 
#define TwoBytes2Byte(aVal)         ( (uint8_t)( *( ( uint8_t * )(aVal) ) ) )
#define Cmp2BytesToZero(aVal)  (!( TwoBytesToUint16(aVal) ))
#define Inc4Bytes(aVal1, iVal2)  ( *((uint32_t *)(aVal1)) += (iVal2))

/* allow for routines/variables to be static or not at compile-time. */
#define STATIC

/* add assert codes if needed. Calls assert function. */
typedef uint8_t assertCode_t;

void BeeAppAssert(assertCode_t code);

#define gAssertCode_NoTasks_c   0x01  /* not enough tasks. Increase gTsMaxTasks_c */
#define gAssertCode_NoTimers_c  0x02  /* not enough timers. Increase gTmrApplicationTimers_c */
#define gAssertCode_FragPtr_c   0x03  /* a pointer was provided but no fragmentation bit for APSDE-DATA.request */
#define gAssertCode_PageTooBig_c  0x04  /* An NVM page is to big to be stored. */

/* indicate endian-ness of this MCU */
#define gBigEndian_c TRUE

#if ( defined(PROCESSOR_QE128) && defined(PROCESSOR_MC1323X))
 #error "Error: Configuration not supported"
#endif


#if (defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X))
  #ifndef __BANKED__
    #error "Error: Banked memory model not configured. Please use the -Mb compiler switch."
  #else 
    #define MEMORY_MODEL_BANKED
  #endif
#endif

#if (!defined(PROCESSOR_QE128) && !defined(PROCESSOR_MC1323X))
  #define PROCESSOR_HCS08
#endif


#endif /* _EMBEDDEDTYPES_H_ */
