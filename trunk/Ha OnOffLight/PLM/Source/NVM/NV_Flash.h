/*****************************************************************************
* Declarations for the NV storage module.
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _NV_FLASH_H_
#define _NV_FLASH_H_

#include "icg.h"
#include "NVM_Interface.h"
#include "NV_FlashHAL.h"

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* No backdoor key allowed, vector redirect disable, unsecure. */
/* Used in crt0.h */
#define NVFOPT_VALUE	  0x02

/* Any flash location (not protected) can be erased or programmed. Flash is */
/* protected as specified in FPS2:FPS1:FPS0 (0xF000-0xFFFF)(=4KB). */
/* Used in crt0.h */
#define NVFPROT_VALUE	  0x98

/*****************************************************************************
******************************************************************************
* Public type declarations
******************************************************************************
*****************************************************************************/

/* It is possible for there to be more than one copy of a given data set */
/* stored in NV. A sequence number is used to recognize the most recent */
/* version. */
typedef uint16_t NvDataSetSequenceNumber_t;

/* A magic number is used to recognize NV pages that have been written to */
/* by this code vs. pages that have been never been written or that have */
/* been written as part of initial flash programming during manufacture. */
/* This should be something improbable; not 0x00, 0xFF, or a simple */
/* repeating bit pattern. */
/* Note that this magic number can also serve to identify versions of */
/* the NV storage code. If the NvStructuredPage_t typedef changes, change */
/* this number. */
typedef uint8_t NvMagicNumber_t;
#define mNvMagicNumber_c    ((NvMagicNumber_t) 0x28)  /* Before was 0x27, now is
                                                         different because the
                                                         HAL work with a physical
                                                         address instead of a
                                                         logical */

/* A small struct is written as both the first and last data in each page, */
/* to identify the contents of the page, and to ensure that the entire */
/* contents of the page have been written correctly. */
typedef struct NvStructuredPageHeader_tag {
  NvMagicNumber_t magicNumber;
  NvDataSetSequenceNumber_t sequenceNumber;
  NvDataSetID_t dataSetID;
} NvStructuredPageHeader_t;

/* One page in flash. */
/* NVM_Interface.h defines the sizeof() the clientData field as a magic */
/* integer (gNvMaxDataSetSize_c). If it changes here, be sure to change it */
/* there, too. */
typedef struct NvStructuredPage_tag {
  NvStructuredPageHeader_t header;
  unsigned char clientData[   sizeof(NvRawPage_t)
                            - sizeof(NvStructuredPageHeader_t)
                            - sizeof(NvStructuredPageHeader_t)];
  NvStructuredPageHeader_t trailer;
} NvStructuredPage_t;

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/



#endif _NV_FLASH_H_
