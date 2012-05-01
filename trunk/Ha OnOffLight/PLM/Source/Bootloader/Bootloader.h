/************************************************************************************
*
*(c) Copyright 2011, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/

#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include "EmbeddedTypes.h"
#include "NV_FlashHal.h"

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

/* Defines used for implementing the boolean types when working with Flash */
#define gBootValueForTRUE_c		0x00
#define gBootValueForFALSE_c	0xFF


/* In order for the bootloader to be able to successfully process an external image and 
load it into the internal MCU FLASH, it must be aware of the map of the sectors in the
internal FLASH which must not be touched while loading an external image,
because they contain information that needs to be preserved 
   A bootable image stored in the external EEPROM should have the following
format in order to be valid:
	- length field: 4 bytes
	- protected sectors bitmap: 15 or 32 bytes, depending on the architecture
	- actual image */
		

#define gBootData_ImageLength_Offset_c       0x00
#define gBootData_ImageLength_Size_c       	 0x04

#define gBootData_SectorsBitmap_Offset_c     (gBootData_ImageLength_Offset_c + \
										      gBootData_ImageLength_Size_c)

#if defined (PROCESSOR_HCS08)
  #define gBootData_SectorsBitmap_Size_c     15
#elif defined (PROCESSOR_QE128)
  #define gBootData_SectorsBitmap_Size_c     32
#elif defined (PROCESSOR_MC1323X)
  #define gBootData_SectorsBitmap_Size_c     32
#endif

#define gBootData_Image_Offset_c             (gBootData_SectorsBitmap_Offset_c + \
                                              gBootData_SectorsBitmap_Size_c)


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

#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
  extern void Boot_LoadImage(void);
  extern void Boot_Add8BitValTo32BitVal(uint8_t val8Bit, uint16_t* pVal32Bit);
#pragma CODE_SEG DEFAULT





#endif /* _BOOTLOADER_H_ */
