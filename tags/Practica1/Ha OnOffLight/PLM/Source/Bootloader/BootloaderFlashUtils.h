/******************************************************************************
* Interface definition for NV utilities
*
*
* (c) Copyright 2010, Freescale Semiconductor, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
******************************************************************************/
#ifndef _NV_UTILS_H_
#define _NV_UTILS_H_

/******************************************************************************
*******************************************************************************
* Public macro definitions
*******************************************************************************
******************************************************************************/

/* The defines below contain information about the internal FLASH of the MCU that
   is needed by the bootloader */
#if defined (PROCESSOR_HCS08)
  #define gFlashParams_MaxImageLength_c           0x10000
  #define gFlashParams_ErasableStartOffset_c      0x1080
#elif defined (PROCESSOR_QE128)
  #define gFlashParams_MaxImageLength_c           0x20000
  #define gFlashParams_ErasableStartOffset_c      0
#elif defined (PROCESSOR_MC1323X)
  #define gFlashParams_MaxImageLength_c           0x14000
  #define gFlashParams_ErasableStartOffset_c      0
#endif

#define gFlashParams_PageLength_c               gNvRawPageSize_c


/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
  void   NvInit(void);
  void   NvEraseSector(uint32_t pageAddress);
  bool_t NvWrite(uint32_t address, unsigned char *pSource, uint16_t sourceLen);
#pragma CODE_SEG DEFAULT

void NvReadData(uint32_t  address,uint8_t * pBuffer, uint8_t len);


#endif _NV_UTILS_H_
