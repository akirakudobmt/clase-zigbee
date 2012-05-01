/************************************************************************************
* SecurityLib implements AES and CCM* functionality. The functions are avaliable to
* all higher layers. The AES module is not reentrant.
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _SECURITY_LIB_H_
#define _SECURITY_LIB_H_
/* Place it in NON_BANKED memory */
#ifdef PROCESSOR_QE128
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* PROCESSOR_QE128 */

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

#define gCcmNonceLength_c 13 // Length of nonce used in ccm star.
#define gCcmBlockSize_c 16   // Block size in CCM star (128 bit)

/************************************************************************************
*************************************************************************************
* Public types
*************************************************************************************
************************************************************************************/

enum {
  gCcmEncode_c,
  gCcmDecode_c
};
typedef uintn8_t gCcmDirection_t;


/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************

it will process the result in about 8000 cycles on the current platform. Please note that the function:
1) Always uses 128 bit (16 bytes) inputs/output.
2) Is not re-entrant (and is not re-entrant with execution of the CCMStar function either).
3) It is allowed to set the ReturnData pointer to the same memory as either Data or Key if wanted.

************************************************************************************/

/************************************************************************************
* Perform AES-128 cryptographic operation on a data set. Optimized for 8 bit
* processors. The result will be processed in about 8000 cycles on the HCS08 platform
* Please note that for HCS08 projects the architecture definitions must locate the
* internal buffers (MY_ZEROPAGE) in lower memory space for correct linking.
*
* Interface assumptions:
*   All input/outputs are 16 bytes (128 bit).
*   Please note that the function is not re-entrant. Also, it is of course not re-
*   entrant with other functions calling this function (like SecLib_CcmStar).
*   It is allowed to point the ReturnData pointer to the same memory as either Data or
*   Key if wanted.
*
* Return value:
*   None
* 
************************************************************************************/
void SecLib_Aes
  (
  const uint8_t *pData, // IN:  Data to be en/de-crypted
  const uint8_t *pKey,  // IN:  128 bit key
  uint8_t *pReturnData  // OUT: Result (can be same address as pData or pKey)
  );


/************************************************************************************
* Calculate CCM* as defined for ZigBee
*
* Interface assumptions:
*
* Header, Message and Integrity code have to be located in memory as they appear in
* the packet. That is, as a concatenated string in that order.
*
* For levels with encryption Message is encrypted in-place (overwriting Message).
*
* Depending on the security level the function does:
*
* Level  Action                                   CCM* engine inputs
* 0:	   Nothing.
* 1,2,3: Integrity only based on Header||Message. a is Header||Message, m is empty
* 4:	   Encryption of Message only.              a is empty, m is Message
* 5,6,7: "Normal" CCM on Header and Message.      a is Header, m is Message
*
* Please note that the function is not re-entrant.
*
* Return value:
*   Return status of the operation (always ok = 0 for encoding)
* 
************************************************************************************/
uint8_t SecLib_CcmStar
  (
  uint8_t * pHeader,           // IN/OUT: start of data to perform CCM-star on
  uint8_t headerLength,        // IN: Length of header field
  uint8_t messageLength,       // IN: Length of data field
  const uint8_t key[16],       // IN: 128 bit key
  const uint8_t nonce[13],     // IN: 802.15.4/Zigbee specific nonce
  const uint8_t securityLevel, // IN: Security level 0-7
  gCcmDirection_t direction    // IN: Direction of CCM: gCcmEncode_c, gCcmDecode_c
  );


/************************************************************************************
* Calculate XOR of individual byte pairs in two uint8_t arrays. I.e.
* pDst[i] := pDst[i] ^ pSrc[i] for i=0 to n-1
*
* Interface assumptions:
*   None
*
* Return value:
*   None
* 
************************************************************************************/
void CCM_XorN
  (
  uint8_t *pDst, // First operand and result of XOR operation
  uint8_t *pSrc, // Second operand. Not modified.
  uint8_t n      // Number of bytes in input arrays.
  );

/* Place it in NON_BANKED memory */
#ifdef PROCESSOR_QE128
#pragma CODE_SEG DEFAULT
#endif /* PROCESSOR_QE128 */

#endif // _SECURITY_LIB_H_
