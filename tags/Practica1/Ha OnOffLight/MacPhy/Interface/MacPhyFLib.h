/************************************************************************************
* This header file is provided as part of the interface to the freeScale 802.15.4
* MAC and PHY layer.
*
* The file gives access to the generic function library used by the MAC/PHY.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _MACPHYFLIB_H_
#define _MACPHYFLIB_H_

#include "PlatformToMacPhyConfig.h" // Included for endianess 
#include "FunctionLib.h"

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

#define FLib_Cmp8Bytes(dst, src) FLib_MemCmp((dst), (src), 8);
#define LongToByteArray(uint32, arr4) {arr4[0] = (uint8_t)((uint32)>>0);  arr4[1] = (uint8_t)((uint32)>>8);  \
                                       arr4[2] = (uint8_t)((uint32)>>16); arr4[3] = (uint8_t)((uint32)>>24); }

#define ByteArrayToShort(arr2, uint16) ((uint16) = (uint16_t)((arr2)[0] | ((arr2)[1] << 8)))

#define ByteArrayToLong(arr4, uint32) {              \
  uint16_t uint16a = arr4[0] | (uint16_t)arr4[1]<<8; \
  uint16_t uint16b = arr4[2] | (uint16_t)arr4[3]<<8; \
  uint32 = ((uint32_t)uint16b<<16) | uint16a; }

#define ByteArrayToZbClock24(arr3, zbClock24) {               \
  zbClock16_t zbClock16a = arr3[0] | (zbClock16_t)arr3[1]<<8; \
  zbClock16_t zbClock16b = (zbClock16_t)arr3[2];              \
  zbClock24 = (zbClock24_t)(((zbClock24_t)zbClock16b<<16) | zbClock16a); }

#define ZbClock24ToByteArray(zbClock24, arr3) \
{ \
  arr3[0] = (uint8_t)(zbClock24 >> 0);  \
  arr3[1] = (uint8_t)(zbClock24 >> 8);  \
  arr3[2] = (uint8_t)(zbClock24 >> 16); \
}

#define FLib_AddrModeToLen(addrMode) ((uint8_t)FLib_AddrModeToLength((uint8_t)(addrMode))) 


/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* Convert an 802.15.4 address mode to a length in bytes, Input values must be 0, 2,
* or 3. Other values will return either 0 or 2.
* 
* Interface assumptions:
*   None
*   
* Return value:
*   0, 2, or 8.
* 
************************************************************************************/
uint8_t FLib_AddrModeToLength
  (
  uint8_t addrMode // IN: 802.15.4 address mode (0, 2, or 3)
  );

/************************************************************************************
* Initialize an Array-session. The supplied pointer will be stored in a module
* variable, and used as a base pointer for succeeding Array-operations.
* 
* Interface assumptions:
*   None
*   
* Return value:
*    None
* 
************************************************************************************/
void FLib_ArrayInit
  (
  uint8_t *pArray // IN: Base pointer used for Array-operations
  );

/************************************************************************************
* Get the current length of the array as number of bytes. The 'bias' argument is
* added to the array length.
* 
* Interface assumptions:
*   None
*   
* Return value:
*   Number of continous bytes stored in the Array + bias.
* 
************************************************************************************/
uint8_t FLib_ArrayLength
  (
  uint8_t bias // IN: Value to be added to the returned length
  );

/************************************************************************************
* Get the pointer to the current position in the Array
* 
* Interface assumptions:
*   None
*   
* Return value:
*   Pointer to next empty position in array.
* 
************************************************************************************/
uint8_t *FLib_ArrayGet
  (
  void
  );

/************************************************************************************
* Move the head pointer to a position in the Array relative to the original
* base pointer.
* 
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void FLib_ArraySetOffset
  (
  uint8_t offset // IN: Position relative to the Array-base
  );

/************************************************************************************
* Copy a number of bytes to the Array.
* 
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void FLib_ArrayAddArray
  (
  uint8_t *pArray, // IN: The byte array to be added
  uint8_t length   // IN: Number of bytes to be added
  );

/************************************************************************************
* Add the value of a specific PIB attribute to the Array.
* 
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void FLib_ArrayAddPibAttribute
  (
  uint8_t pibAttribute // IN: The PIB attribute (e.g. gMacPanId_c)
  );

/************************************************************************************
* Add one byte to the Array.
* 
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void FLib_ArrayAddByte
  (
  uint8_t byte // IN: Byte to add to the Array
  );

#endif /* _FUNCTION_LIB_H_ */
