/******************************************************************************
* This is the source file for the bootloader
*
*														 
* (c) Copyright 2010, Freescale Semiconductor inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from FreeScale.
*
******************************************************************************/
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
   The code in this file should be self contained. It should not have any 
   calls to external functions from within the compiler libraries (ansiis, ansibim, etc).   
   When generating an application with bootloader support, please check in the list file
   or the map file of the application that there is no external calls from this module */



#include "Embeddedtypes.h"
#include "Bootloader.h"
#include "BootloaderEeprom.h"
#include "BootloaderFlashUtils.h"
#include "crt0.h"


#ifdef gUseBootloader_d

/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/
/* Assembly written code that executes an illegal instruction */  
#define Boot_ExecuteIllegalInstruction()  __asm ldhx @mIllegalOpcode; \
                                          __asm jmp ,x;

/* Reset the MCU by executing an illegal instruction */
#define Boot_ResetMcu()                   Boot_ExecuteIllegalInstruction()


/* Init value for the mask used to look through each byte in the bitmap that
indicates which Flash sectors are write protected and should not be updated */
#define gBitMaskInit_c                  0x80

/* Size of the buffer to be allocated on the stack for FLASH reads. The function calls
   where this size is allocated should be done at the beginning of the app execution
   when the stack is not utilized. */
#define gEepromBootStackBufferSize_c  	128 

/* On MC1321x, the FLASH is split in two parts, with the
High Page Registers area in the middle, between addresses $1800 and $182B
   A Flash Page can be cleared by erasing any byte inside the page and usually 
first byte in the page is used for this. On MC1321x, erasing the byte at
address $1800 will not result in the FLASH page starting at address $1800 to be
cleared. For this reason, on MC1321x all FLASH pages are cleared by errasing
the byte starting at 0x2C bytes from begining of sector. This will ensure that
FLASH sector containing the High Page Registers will be erased */
#define gS08EraseAddressOffset_c 		0x2C


/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
  static void Boot_Sub8BitValFrom32BitVal(uint8_t val8Bit, uint16_t* pVal32Bit);
#pragma CODE_SEG DEFAULT

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/
#pragma CONST_SEG BOOTLOADER_DATA_ROM
  /* An HCS08 illegal instruction. Used to force the MCU reset */
  static const uint16_t mIllegalOpcode = 0x9e62;                
#pragma CONST_SEG DEFAULT


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

#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM

#pragma NO_FRAME
#pragma NO_EXIT
/*****************************************************************************
*  Boot_LoadImage
*
*  This function is called when the startup code detects that a new image is 
*  available. Right now, the bootloader only supports loading the image
*  from an external EEPROM. The image from EEPROM will be loaded
*  into the internal MCU FLASH, after which the MCU will be reset
*
*****************************************************************************/
void Boot_LoadImage(void) 
{
  
  uint8_t  bitmapBuffer[gBootData_SectorsBitmap_Size_c + sizeof(uint32_t)];
  uint8_t  dataBuffer[gEepromBootStackBufferSize_c];
  uint32_t remainedImgLen;
  uint32_t eeIndex                = gBootData_Image_Offset_c;         
  uint32_t flashAddress           = gFlashParams_ErasableStartOffset_c;
  uint8_t* pBitmap                = &bitmapBuffer[4];
  uint8_t  bitMask                = gBitMaskInit_c;
  uint8_t  bitMaskOld             = 0;
  uint8_t  status                 = boot_ee_ok;
  volatile uint8_t  imageChunkLen = gEepromBootStackBufferSize_c;
  
 
  /* Try to initialize the EEPROM */
  /* EEPROM functions for QE128 board do not return failure statuses. In order
to gain some code in the bootloader, do not check the status of these functions
on QE128 */
  #ifdef PROCESSOR_QE128 
    (void)EEPROM_BOOT_Init();
  #else
    if(EEPROM_BOOT_Init())
    {
      /* If EEPROM could not be successfully initialized, reset MCU and retry the whole process again */
      Boot_ResetMcu();
    }
  #endif
   
  /* Read first 2 fields of the eeprom image (lentgh and protected sectors bitmap) into a common buffer in order 
  to save some code in the bootloader */
  #ifdef PROCESSOR_QE128
    (void)EEPROM_BOOT_ReadData(sizeof(remainedImgLen) + gBootData_SectorsBitmap_Size_c, gBootData_ImageLength_Offset_c, bitmapBuffer);
  #else
    if(EEPROM_BOOT_ReadData(sizeof(remainedImgLen) + gBootData_SectorsBitmap_Size_c, gBootData_ImageLength_Offset_c, bitmapBuffer))
    {
      /* If EEPROM could not be successfully read, reset MCU and retry the whole process again */
      Boot_ResetMcu();
    }
  #endif
  
  /* Initialize the local variable storing the length of image remained to be loaded in internal Flash
  from external EEPROM */  
  remainedImgLen = *(uint32_t *)(&bitmapBuffer[0]); 
  
  
  /* Initialize the module dealing with the non volatile memory (FLASH in our case) */
  NvInit();
  
  /* Start the loading of information from external EEPROM to internal FLASH. Order of the operations
  will be:
	1. Check the bitmap to see if current Flash sector should be preserved. If yes, skip to next
  sector and repeat step 1. If no, go to step 2
	2. Erase current sector in the internal Flash
	3. Load the current sector in internal Flash with the information from external EEPROM. If the 
  current sector contains the flag used by bootloader to know that an external image is available 
  to be booted, skip the flag and leave it set to 0xFF.  
	4. Check if whole image has been loaded from external EEPROM to internal Flash. If not, go
  to step 1. If yes, go to step 6.
    6. Set the flag informing that an external image is available to be booted to 0x00, meaning
  no external image is available.
    7. Reset the MCU by executing an illegal opcode */ 

  while(remainedImgLen != 0)
  {

    
    /* Check if bitmap indicates that this sector is write protected and shouldn't be updated */
    if(*pBitmap & bitMask)
    { 
      /* Check if we got to the last chunk of data from EEPROM */
      if(! (((uint16_t)remainedImgLen > gEepromBootStackBufferSize_c) || ((uint16_t)(remainedImgLen >> 16) != 0))) 
      {
        imageChunkLen = (uint8_t)remainedImgLen;
      }
      /* Read from eeprom */
      #ifdef PROCESSOR_QE128
        (void)EEPROM_BOOT_ReadData(imageChunkLen, eeIndex, dataBuffer);
      #else 
	    if(EEPROM_BOOT_ReadData(imageChunkLen, eeIndex, dataBuffer))
	    {
	      /* If EEPROM could not be successfully read, reset MCU and retry the whole process again */
	      Boot_ResetMcu();
	    }
      #endif

      /* Sector is not write protected. As we read chunks of data of 128 bytes from EEPROM but the
      Flash sector page is larger, we only erase the Flash sector when we put first chunk
      of data in it */
      if(bitMaskOld != bitMask)
      {
        /* Avoid HCS08 High Page Registers */          
        #if defined (PROCESSOR_HCS08)
          NvEraseSector((uint16_t)flashAddress + gS08EraseAddressOffset_c);
        #else
          NvEraseSector(flashAddress);
        #endif
        bitMaskOld = bitMask;
      }
      
      if(NvWrite(flashAddress, dataBuffer, imageChunkLen ) == FALSE) 
      {
        /* If FLASH could not be successfully written, reset MCU and retry the whole process again */
        Boot_ResetMcu();
      }
    }
    
    /* eeIndex += imageChunkLen */
    Boot_Add8BitValTo32BitVal(imageChunkLen, (uint16_t *)&eeIndex);
    
    /* flashAddress += imageChunkLen */
    Boot_Add8BitValTo32BitVal(imageChunkLen, (uint16_t *)&flashAddress);
    
    /* RemainedImgLen -= imageChunkLen  */
    Boot_Sub8BitValFrom32BitVal(imageChunkLen, (uint16_t*)&remainedImgLen);
    
    /* Check if we got to the end of one Flash sector */
    if( 0 ==(((uint16_t)flashAddress) % gFlashParams_PageLength_c ))
    {
      /* We got to the end of one Flash sector. Move to next bit in the current
      bitmap byte */
      bitMask >>=1;
      if(bitMask == 0)
      {
		/* This was last bit in the current bitmap byte. Move to next bitmap byte */	  
        bitMask = gBitMaskInit_c;
        pBitmap++;
      }  
    }  
  }
  
  /* Whole image was successfully loaded from EEPROM to internal Flash.
  Set the flag that indicates that the loading has been completed. Temporary use one 
  one the local variables */
  dataBuffer[0] = gBootValueForTRUE_c;
  
  /* Set image upload finish */
  (void)NvWrite((uint32_t)&bBootProcessCompleted, dataBuffer, sizeof(uint8_t));
  
  /* External image successfully loaded in Flash. Reset MCU */
  Boot_ResetMcu();
}
/*****************************************************************************
*  Boot_Add8BitValTo32BitVal
*
*  This is a utility function that adds a 8 bit value (val8Bit) to a 32 bit 
*  value (val32Bit). 
*  The code is written such that the compiler does not include any calls to 
*  32 bit functions from the compiler library (ansiis or ansibim)
*
*****************************************************************************/
void Boot_Add8BitValTo32BitVal(uint8_t val8Bit, uint16_t* pVal32Bit)
{ 
  
  /* Point to LSB */
  pVal32Bit++;
  *pVal32Bit += val8Bit;
  /* Check if it is something to add to MSB */
  if(val8Bit > *pVal32Bit)
  {
    /* Point to MSB */
    pVal32Bit--;
    (*pVal32Bit)++;  
  }
}

#pragma CODE_SEG DEFAULT


/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
/*****************************************************************************
*  Boot_Sub8BitValFrom32BitVal
*
*  This is a utility function that substracts a 8 bit value (val8Bit) from a 32 bit 
*  value (val32Bit). 
*  The code is written such that the compiler does not include any calls to 
*  32 bit functions from the compiler library (ansiis or ansibim)
*
*****************************************************************************/
static void Boot_Sub8BitValFrom32BitVal(uint8_t val8Bit, uint16_t* pVal32Bit)
{
  /* Point to LSB */
  pVal32Bit++;
  if(*pVal32Bit >= val8Bit)
  {
    *pVal32Bit -= val8Bit;
  }
  else
  {
    *pVal32Bit -= val8Bit;
    /* Point to MSB */
    pVal32Bit--;
    (*pVal32Bit)--;
  }
}

#pragma CODE_SEG DEFAULT


#endif /* gUseBootloader_d */
