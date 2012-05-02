/******************************************************************************
* Interface definition for EEPROM driver
*
* (c) Copyright 2010, Freescale Semiconductor, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
******************************************************************************/
#ifndef _EEPROM_BOOT_H_
#define _EEPROM_BOOT_H_

#include "AppToPlatformConfig.h"

/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Public macros
*******************************************************************************
******************************************************************************/

/* List of the EEPROM devices used on each of the FSL development boards */
#if (gTargetMC13213SRB_d) || (gTargetMC13213NCB_d)
  #define gEepromDevice_AT25HP512C1_c
#elif (gTargetQE128EVB_d)
  #define gEepromDevice_AT24C1024BW_c
#elif (gTargetMC1323xRCM_d) || (gTargetMC1323xREM_d)
  #define gEepromDevice_AT45DB021D_c
#endif
  

/* Characteristics of the EEPROM device */
#if defined(gEepromDevice_AT25HP512C1_c)
  #define gEepromParams_TotalSize_c            0x10000                       
#elif defined(gEepromDevice_AT24C1024BW_c)
  #define gEepromParams_TotalSize_c            0x20000                       
#elif defined(gEepromDevice_AT45DB021D_c)
  #define gEepromParams_TotalSize_c            0x40000  
#elif !defined(gEepromParams_TotalSize_c)
  #define gEepromParams_TotalSize_c            0x20000  
#endif


/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/

typedef enum
{
  boot_ee_ok,
  boot_ee_error
}boot_ee_err_t;


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

/* If the bootloader is enabled, the EEPROM_BOOT_Init and EEPROM_BOOT_ReadData functions
should be placed in the Bootloader's segment as they will be used by the 
bootloader */
#ifdef gUseBootloader_d
  #pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
#endif

/******************************************************************************
* This function initialises the port pins and the EEPROM 
*
* Interface assumptions:   
*   													  
*
* Return value: ee_err_t
*   
******************************************************************************/
boot_ee_err_t EEPROM_BOOT_Init
(
  void /*IN: No Input Parameters */
);


/******************************************************************************
* This function Reads x bytes from the EEPROM
*
* Interface assumptions:   
*   													  
*
* Return value: ee_err_t  
*   
******************************************************************************/
boot_ee_err_t EEPROM_BOOT_ReadData
(
  uint16_t NoOfBytes,/* IN: No of bytes to read */
  uint32_t Addr,		 /* IN: Start reading EEPROM address */
  uint8_t  *inbuf		 /* OUT:Pointer to read buffer */ 
);

#ifdef gUseBootloader_d
  #pragma CODE_SEG DEFAULT
#endif

#endif


