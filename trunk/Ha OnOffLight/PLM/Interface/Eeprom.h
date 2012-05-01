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
#ifndef _EEPROM_H_
#define _EEPROM_H_

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
  ee_ok,
  ee_too_big,
  ee_not_aligned,
  ee_error
} ee_err_t;

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


/******************************************************************************
*  This function initialises the port pins and the EEPROM 
*
* Interface assumptions:   
*   													  
*
* Return value: ee_err_t
*   
******************************************************************************/
ee_err_t EEPROM_Init
(
  void /*IN: No Input Parameters*/
);


/******************************************************************************
*  This function Reads x bytes from the EEPROM
*
* Interface assumptions:   
*   													  
*
* Return value: ee_err_t  
*   
******************************************************************************/
ee_err_t EEPROM_ReadData
(
  uint16_t NoOfBytes,/* IN: No of bytes to read */
  uint32_t Addr,		 /* IN: EEPROM address to start reading from */
  uint8_t  *inbuf		 /* OUT:Pointer to read buffer */ 
);

/******************************************************************************
*  This function writes x bytes to the EEPROM
*
* Interface assumptions:   
*   													  
*
* Return value: ee_err_t  
* 
******************************************************************************/
ee_err_t EEPROM_WriteData
(
  uint8_t  NoOfBytes,/* IN: No of bytes to write */
  uint32_t Addr,		 /* IN: EEPROM address to start writing at. */
  uint8_t  *Outbuf	 /* IN: Pointer to data to write to EEPROM  */
);


#endif /* _EEPROM_H_ */


