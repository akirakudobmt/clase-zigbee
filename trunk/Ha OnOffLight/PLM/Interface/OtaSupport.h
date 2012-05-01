/************************************************************************************
*
*(c) Copyright 2010, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
************************************************************************************/

#ifndef _OTA_SUPPORT_H_
#define _OTA_SUPPORT_H_

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

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
typedef enum
{
  gOtaSucess_c = 0,
  gOtaNoImage_c,
  gOtaUpdated_c,
  gOtaError_c,
  gOtaCrcError_c,
  gOtaInvalidParam_c,
  gOtaInvalidOperation_c,
  gOtaEepromError_c,
  gOtaInternalFlashError_c,
} otaResult_t;

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
*  Starts the process of writing a new image to the external EEPROM.
*
*  Input parameters:
*  - lenght: the lenght of the image to be written in the EEPROM
*  Return:
*  - gOtaInvalidParam_c: the intended lenght is bigger than the FLASH capacity
*  - gOtaInvalidOperation_c: the process is already started (can be cancelled)
*  - gOtaEepromError_c: can not detect extrenal EEPROM
************************************************************************************/
extern otaResult_t OTA_StartImage(uint32_t length);

/************************************************************************************
*  Places the next image chunk into the external FLASH.
*  !!!! WARNING !!!!
*  Due to the EEPROM currently supported, all but the last image chunk have to be of
*  128 bytes.
*
*  Input parameters:
*  - pData: pointer to the data chunk
*  - length: the length of the data chunk
*  - pImageLength: if it is not null and the function call is sucessfull, it will be
*       filled with the current lenght of the image
*  Return:
*  - gOtaInvalidParam_c: pData is NULL or the resulting image would be bigger than the 
*       final image length specified with OTA_StartImage()
*  - gOtaInvalidOperation_c: the process is not started
************************************************************************************/
extern otaResult_t OTA_PushImageChunk(uint8_t* pData, uint8_t length, uint32_t* pImageLength);

/************************************************************************************
*  Finishes the writing of a new image to the external EEPROM.
*  It will write the image header (signature and lenght) and footer (sector copy bitmap)
*  and CRC.
*
*  Input parameters:
*  - bitmap: pointer to a 15 byte byte array indicating the sector erase pattern for the
*       internal FLASH before the image update. The MSB of the first byte controls the
*       erasing of the first sector of the second FLASH area (0x182C - 0xFFFF).
*  Return:
*  - gOtaInvalidOperation_c: the current image lenght is not equal with the 
*       final image length specified with OTA_StartImage(), or the process is not started,
*       or the bitmap bits do not instruct the erasing of enough sectors to contain the image.
*  - gOtaEepromError_c: error while trying to write the EEPROM 
************************************************************************************/
otaResult_t OTA_CommitImage(uint8_t* bitmap);

/************************************************************************************
*  Cancels the process of writing a new image to the external EEPROM.
*
*  Input parameters:
*  - None
*  Return:
*  - None
************************************************************************************/
extern void OTA_CancelImage(void);
/*****************************************************************************
*  OTA_WriteNewImageFlashFlags
*  
*  Input parameters:
*  - None
*  Return:
*  - None
*  It is called in idle task to write flags from flash - new image present
*****************************************************************************/
void OTA_WriteNewImageFlashFlags(void);


#endif /* _OTA_SUPPORT_H_ */
