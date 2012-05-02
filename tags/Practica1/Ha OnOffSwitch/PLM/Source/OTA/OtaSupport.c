/******************************************************************************
* This is the source file containing the code that enables the OTAP protocol
* to load an image received over the air into an external EEPROM memory, using
* the format that the bootloader will understand
*
*														 
* (c) Copyright 2010, Freescale Semiconductor inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from FreeScale.
*
******************************************************************************/
#include "Embeddedtypes.h"
#include "OtaSupport.h"
#include "Eeprom.h"
#include "Bootloader.h"
#include "BootloaderFlashUtils.h"
#include "crt0.h"
#include "IrqControlLib.h"


/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/
#if defined (PROCESSOR_HCS08)
  #define gStartFlashAddress_c        0x1080
#else
  #define gStartFlashAddress_c        0x00  
#endif

/* There are 2 flags stored in the internal Flash of the MCU that tells
  1. whether there is a bootable image present in the external EEPROM
  2. whether the load of a bootable image from external EEPROM to internal
Flash has been completed. This second flag is useful in case the MCU is reset
while the loading of image from external EEPROM to internal Flash is in progress 
  No matter the platform (MC1320x, MC1321x, MC1323x), these 2 flags are always
located at a fixed address in the internal FLASH */
#define gBootImageFlagsAddress_c       0xFB9E

/* Need to compute the offset relative to the start of the image loaded in EEPROM
where the 2 flags will actually reside in EEPROM */
#define gBootImageFlagEepromOffset_c  (gBootData_Image_Offset_c + gBootImageFlagsAddress_c - gStartFlashAddress_c)
 
/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/* Structure containing the 2 boot flags */
typedef struct
{
  uint8_t bNewBootImageAvailable;
  uint8_t bBootProcessCompleted;
}bootFlags_t;

  

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/

/* Flag storing we are already in the process of writing an image received
OTA in the EEPROM or not */
static  bool_t    mLoadOtaImageInEepromInProgress = FALSE;    
/* Total length of the OTA image that is currently being written in EEPROM */
static  uint32_t  mOtaImageTotalLength = 0;           
/* The length of the OTA image that has being written in EEPROM so far */
static  uint32_t  mOtaImageCurrentLength = 0;    
/* Current write address in the EEPROM */
static  uint32_t  mCurrentEepromAddress = 0;        
/* When a new image is ready the flash flags will be write in idle task */
static  bool_t    mNewImageReady=FALSE;


/******************************************************************************
*******************************************************************************
* Public Memory
*******************************************************************************
******************************************************************************/
 

/******************************************************************************
*******************************************************************************
* Public Functions
*******************************************************************************
******************************************************************************/
extern bool_t NvHalUnbufferedWrite(uint8_t const *pPageAddress,NvSize_t dstPageOffset,uint8_t *pSource,NvSize_t sourceLen);


/*****************************************************************************
*  OTA_StartImage
*
*  This function is called in order to start a session of writing a OTA image
*  in an external EEPROM. 
*
*****************************************************************************/
otaResult_t OTA_StartImage(uint32_t length)
{
  uint8_t  flashBuffer[5]={0};
  
  /* Check if we already have an operation of writing an OTA image in the EEPROM
  in progess and if yes, deny the current request */
  if(mLoadOtaImageInEepromInProgress) 
  {
    return gOtaInvalidOperation_c;
  }
  /* Check if the internal FLASH and the EEPROM have enough room to store
  the image */
  if((length > gFlashParams_MaxImageLength_c) ||
     (length > (gEepromParams_TotalSize_c - gBootData_Image_Offset_c)))
  {
    return gOtaInvalidParam_c;
  }
  /* Try to initialize the EEPROM */
  if(EEPROM_Init() != ee_ok) 
  {
    return gOtaEepromError_c;
  }
  /* Save the total length of the OTA image */
  mOtaImageTotalLength = length;
  /* Init the length of the OTA image currently written */
  mOtaImageCurrentLength = 0;
  /* Init the current EEPROM write address */
  mCurrentEepromAddress = gBootData_Image_Offset_c;
  /* Mark that we have started loading an OTA image in EEPROM */
  mLoadOtaImageInEepromInProgress = TRUE;

  return gOtaSucess_c;
}


/*****************************************************************************
*  OTA_PushImageChunk
*
*  Put image chunck on eeprom 
*  Make a read from eeprom in order to check this data
*****************************************************************************/
otaResult_t OTA_PushImageChunk(uint8_t* pData, uint8_t length, uint32_t* pImageLength)
{
  
  /* Cannot add a chunk without a prior call to OTA_StartImage() */
  if(mLoadOtaImageInEepromInProgress == FALSE) 
  {
    return gOtaInvalidOperation_c;
  }
  
  /* Validate parameters */
  if((length == 0) || (pData == NULL)) 
  {
    return gOtaInvalidParam_c;
  }
  
  /* Check if the chunck does not extend over the boundaries of the image */
  if(mOtaImageCurrentLength + length > mOtaImageTotalLength) 
  {
    return gOtaInvalidParam_c;
  }
  
  /* For sefty disable irq */    
  IrqControlLib_DisableAllIrqs();
  
  /* Try to write the data chunk into the external EEPROM */
  if(EEPROM_WriteData((uint16_t)length, mCurrentEepromAddress, pData) != ee_ok) 
  {
    return gOtaEepromError_c;
  }

  /* Enable Interrupts */
  IrqControlLib_EnableAllIrqs();
  
  /* Try to read the previous data chunk from external EEPROM */
  if(EEPROM_ReadData((uint16_t)length, mCurrentEepromAddress, pData) != ee_ok) 
  {
    return gOtaEepromError_c;
  }
  /* Data chunck successfully writtem into EEPROM
     Update operation parameters */
  mCurrentEepromAddress   += length;
  mOtaImageCurrentLength += length;
  
  /* Return the currenlty written length of the OTA image to the caller */
  if(pImageLength != NULL) 
  {
    *pImageLength = mOtaImageCurrentLength;
  }
  
  
  return gOtaSucess_c;
}


/*****************************************************************************
*  OTA_CommitImage
*
*
*****************************************************************************/
otaResult_t OTA_CommitImage(uint8_t* pBitmap)
{
  bootFlags_t bootFlags =
  {
    gBootValueForFALSE_c,   /* bNewBootImageAvailable */
    gBootValueForFALSE_c    /* bBootProcessCompleted  */ 
  };
  
  /* Cannot commit a image without a prior call to OTA_StartImage() */
  if(mLoadOtaImageInEepromInProgress == FALSE) 
  {
    return gOtaInvalidOperation_c;
  }
  /* If the currently written image length in EEPROM is not the same with 
  the one initially set, commit operation fails */
  if(mOtaImageCurrentLength != mOtaImageTotalLength) 
  {
    return gOtaInvalidOperation_c;
  }
  
  /* To write image length into the EEPROM */
  if(EEPROM_WriteData(sizeof(uint32_t), gBootData_ImageLength_Offset_c,(uint8_t *)&mOtaImageCurrentLength) != ee_ok) 
  {
    return gOtaEepromError_c;
  }
  
  /* To write the sector bitmap into the EEPROM */
  if(EEPROM_WriteData(gBootData_SectorsBitmap_Size_c, gBootData_SectorsBitmap_Offset_c, pBitmap) != ee_ok) 
  {
    return gOtaEepromError_c;
  }
  
  /* Set bootImage flags in the image loaded in EEPROM to FALSE, no matter what value they actually have.
     This is for protection in case of reset */
  if(EEPROM_WriteData(sizeof(bootFlags_t),gBootImageFlagEepromOffset_c,(uint8_t*)&bootFlags) != ee_ok) 
  {
    return gOtaEepromError_c;
  }
  
  /* Flash flags will be write in next instance of idle task */
  mNewImageReady = TRUE;
  
  /* End the load of OTA in EEPROM process */
  mLoadOtaImageInEepromInProgress = FALSE;
  
  return gOtaSucess_c;
}
/*****************************************************************************
*  OTA_WriteNewImageFlashFlags
*
*  It is called in idle task to write flags from flash
*****************************************************************************/
void OTA_WriteNewImageFlashFlags(void)
{
  /* OTAP image successfully writen in EEPROM. Set the flag that indicates that at the next
  boot, the image from FLASH will be updated with the one from EEPROM */
#ifdef gUseBootloader_d
  uint8_t  newImageFlag = gBootValueForTRUE_c;
  if(TRUE == mNewImageReady)
  {
    
    if(!NvHalUnbufferedWrite((const uint8_t *)&bNewBootImageAvailable,0,&newImageFlag,sizeof(newImageFlag))) 
    {
      return ;
    }
    mNewImageReady = FALSE;
  }
#endif
}

/*****************************************************************************
*  OTA_CancelImage
*
*
*****************************************************************************/
void OTA_CancelImage()
{
  mLoadOtaImageInEepromInProgress = FALSE;
}

/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

