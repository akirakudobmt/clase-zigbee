/******************************************************************************
* This is the Source file for Flash Utilities for HCS08,QE128 and MC1323X cores
*												 
* (c) Copyright 2010, Freescale Semiconductor inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from FreeScale.
*
******************************************************************************/
/*      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
      Check compiler in order not to call functions from ANSI*.LIB  
                Check disasamble file or map file 
*/
#include "Embeddedtypes.h"
#include "IoConfig.h"
#include "BootloaderFlashUtils.h"
#include "Bootloader.h"


#ifdef gUseBootloader_d

/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/

/* If a flash command fails, retry it a few times. If it still fails, an 
embedded device generally doesn't have any one to tell about the problem,
so this number can be large */
#define mNvFlashCmdRetries_c        32

/* Size of the function responsible for erasing or writting the FLASH
and which needs to be executed from RAM. Size is needed in order to
know how much code to copy in RAM
   This value must be updated in case the compiler changes, as the 
code of the function could increase */
#define mSizeofNvCodeInRam_c        ((uint16_t)97)

/* Bits in the HCS08 flash status register (FSTAT) */
/* Flash command buffer is empty */
#define FCBEF   (( uint8_t ) 0x80 )
/* Flash command is complete */     
#define FCCF    (( uint8_t ) 0x40 )
/* Flash protection violation */     
#define FPVIOL  (( uint8_t ) 0x20 ) 
/* Flash access error */    
#define FACCERR (( uint8_t ) 0x10 )
/* Flash is entirely blank */     
#define FBLANK  (( uint8_t ) 0x04 )     
/* Flash security error */
#ifdef  PROCESSOR_MC1323X
#define FSECERR (( uint8_t ) 0x08 )     
#else
#define FSECERR (( uint8_t ) 0x00 )
#endif

/* Program a single byte to a location */
#define mNvFlashByteProgramCmd_c        (( uint8_t ) 0x20 )
/* Program multiple bytes to flash */
#define mNvFlashBurstProgramCmd_c       (( uint8_t ) 0x25 )
/* Erase a single flash sector */
#define mNvFlashSectorEraseCmd_c        (( uint8_t ) 0x40 )
/* Erase all of flash memory */
#define mNvFlashMassEraseCmd_c          (( uint8_t ) 0x41 )
/* Writes to 0xFFB0-0xFFB7 are interpreted as the start of a Flash
   programming or erase command */
#define FCNFGValue_c            0

/* FLASH options */
#define FLASH_NVPROT    0xFFBD
#define FLASH_ICG_TRIM  0xFFBE
#define FLASH_NVOPT     0xFFBF


/* Define call function from RAM for flash operation */ 
#define CALL_FLASH_RAM_FUNC     __asm LDHX  @aNvCodeInRAM;\
                                __asm JSR   ,X;



/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
  static void NvCopyCodeInRam(void);
  /* Low level function to send a command to the flash hardware controller
     Returns TRUE if the controller does not report an error
     Place it in NON_BANKED memory */
  static void NvExecuteFlashCmd( void );
#pragma CODE_SEG DEFAULT

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/* typedef for flash write / erase function */
typedef void(*pfNvExecuteFlashCmd_t)( void );


/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/

#pragma DATA_SEG BOOTLOADER_DATA_RAM
  /* Buffer used in flash write process */ 
  static uint8_t*  mpNvSectorBufferOTA;
  /* IN: Flash command to be executed */    
  static uint8_t  flashCommand;
  /* OUT: flash operation return status */     
  static volatile bool_t   bReturnFlashCmdAnswer;
  /* Flash write length */
  static uint16_t nvWriteLen;
  
  #if defined (PROCESSOR_HCS08)
    /* SO8 pointer to flash area to be write */
    static uint8_t*  mpNvAddressFlashWrite;
  #endif
  
#pragma DATA_SEG DEFAULT

#pragma DATA_SEG BOOTLOADER_CODE_RAM
  /* Buffer in RAM where the code that is responsible for erasing or writting FLASH
  will be copied and executed from.
     The linker command file overlays this buffer with the end of the stack.
     Be careful with this one. A new version of the compiler, 
  or changes to compiler options, could change the size needed. Check
  the link map to ensure that there's enough space. */
  static unsigned char aNvCodeInRAM[mSizeofNvCodeInRam_c];
#pragma DATA_SEG DEFAULT

/******************************************************************************
*******************************************************************************
* Public Memory
*******************************************************************************
******************************************************************************/
 
extern char __SEG_START_SSTACK[];


/******************************************************************************
*******************************************************************************
* Public Functions
*******************************************************************************
******************************************************************************/

/* The NvInit, NvEraseSector and NvWrite functions are used by the bootloader
so they should reside in the bootloader code section */
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM

/*****************************************************************************
*  NvInit
*
*  Initializes the flash controller
*
*****************************************************************************/                
void NvInit(void)
{
	/* Initialize the flash controller. */
	/* Clear errors. */
	FSTAT = FPVIOL | FACCERR | FSECERR;  
	
	FCDIV = ( 0x40 | (( uint8_t )( 0x10 / 2 ) + 2 ));	
	
	FCNFG = FCNFGValue_c;
	
  /* Copy to RAM the function that work directly to flash. */
  NvCopyCodeInRam();
}


/*****************************************************************************
*  NvEraseSector
*
*  Erase the sector that contains the given address 
*
*****************************************************************************/                
void NvEraseSector(uint32_t pageAddress)
{
	index_t retries = mNvFlashCmdRetries_c;
	uint8_t eraseValue = 0x33;

	/* Register used for liniar access to memory */
	#if defined (PROCESSOR_HCS08)
    mpNvAddressFlashWrite = (uint8_t *)((uint16_t)(pageAddress));
  #else
	  LAP2 = (uint8_t)(pageAddress>>16);
    LAP1 = (uint8_t)(pageAddress>>8);
    LAP0 = (uint8_t)(pageAddress>>0);
  #endif  

	/* Keep trying a fixed amount of times or until success. */
	while ( retries-- )
	{
	  /* Erase the page. */
		flashCommand = mNvFlashSectorEraseCmd_c;
		/* Just init the write pointer with a known value */
		mpNvSectorBufferOTA = __SEG_START_SSTACK;
		/* eraseValue must be different from 0xFF */
		mpNvSectorBufferOTA = &eraseValue;
		nvWriteLen=1; 
		CALL_FLASH_RAM_FUNC 
		if(bReturnFlashCmdAnswer) 
		{
			break;
		}
	}
	return;
}


/*****************************************************************************
*  NvWrite
*
*  Writes a chunk of data in the FLASH, starting from a given address
*
*****************************************************************************/                
bool_t NvWrite
(
	uint32_t  address,
	uint8_t*  pSource,
	uint16_t  sourceLen
)
{
	/* Status returned for the write action. */
	bReturnFlashCmdAnswer = TRUE;

	/* While we haven't done the full set of bytes keep going. */
	#if defined (PROCESSOR_MC1323X)
	  flashCommand = mNvFlashBurstProgramCmd_c;
  #else
    flashCommand = mNvFlashByteProgramCmd_c;
  #endif

  #if defined (PROCESSOR_HCS08)
    mpNvAddressFlashWrite = (uint8_t *)((uint16_t)(address));
  #else
	  LAP2 = (uint8_t)(address>>16);
    LAP1 = (uint8_t)(address>>8);
    LAP0 = (uint8_t)(address>>0);
  #endif  

	/* Copy the routine into RAM to execute it form there instead that NVM. */
  /* set source len */
	nvWriteLen = sourceLen;
  /* Provide pointer to the source */
	mpNvSectorBufferOTA = pSource;   
  /* Jump to the function from RAM */
  CALL_FLASH_RAM_FUNC;
	/* Return value which was filled in RAM */ 
	return bReturnFlashCmdAnswer;
}

#pragma CODE_SEG DEFAULT


/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

/* The static NvCopyCodeInRam, NvClearCodeFromRam and NvExecuteFlashCmd functions are called
by the public NV_Utils functions used by the bootloader, so they should reside in
the bootloader code section */
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM


/*****************************************************************************
*  NvCopyCodeInRam
*
*  Copies the code of the function responsible for erasing or writing the FLASH
*  into RAM
*
*****************************************************************************/                
static void NvCopyCodeInRam(void)
{
	index_t   i;

  /* Copy the function code into RAM, avoiding the default convetion. */
  #pragma MESSAGE DISABLE C1805  /* Non standard conversion used */

	/* Starting copying the function into RAM */
	for ( i = 0; i < mSizeofNvCodeInRam_c; ++i)
	{
		aNvCodeInRAM[i] = (( uint8_t * )NvExecuteFlashCmd)[i];
	}
}

/*****************************************************************************
*  NvExecuteFlashCmd
*
*  This is the function responsible for erasing or writting the FLASH. It must
*  be executed from RAM 
*
*****************************************************************************/                
static void NvExecuteFlashCmd(void)
{
	
  /* Clear all errors. */
	FSTAT = FPVIOL | FACCERR | FSECERR;
	
	for (;/* Do it until a break point is reached. */;)
	{
		/* Write data to flash. This is required by the flash controller, even
		for commands that don't modify data. */
		
		/* 0xFF values from flash are skipped, they already are deleted*/
		/* Feature impllemented in order to skip high page register area of MC1321x */ 
		#ifndef PROCESSOR_MC1323X                                                   
		  if(0xFF != *mpNvSectorBufferOTA)
		#endif
		{
		  
		  #if defined (PROCESSOR_HCS08)
		    *mpNvAddressFlashWrite = *mpNvSectorBufferOTA;
		  #else
		    LB = *mpNvSectorBufferOTA; 
		  #endif 

		  /* Load command register with flash command. */
		  FCMD = flashCommand;

		  /* Launch the command. */
		  FSTAT = FCBEF;
		  
		  /* Wait for the command buffer to clear. */
		  while ( !( FSTAT & FCBEF ))
		  {/* Do nothing for a while. */}
		}
		mpNvSectorBufferOTA++;
		#if defined (PROCESSOR_HCS08)
		  mpNvAddressFlashWrite++;
		#else
		  LAPAB=1;
		#endif
		/* Check if write is finish */
		if(!(--nvWriteLen))
		{
			/* If we are done get out */
			break;
		}
	}/* end while */

	/* Wait for the command to complete. */
	while ( !( FSTAT & FCCF ))
	{/* Do nothing for a while. */}

	if ( FSTAT & ( FACCERR | FSECERR))
	{
		/* If somethig got wrong, the set and return error. */
		/* An error bit is set. */
		bReturnFlashCmdAnswer=FALSE;  
	}
}

#pragma CODE_SEG DEFAULT

/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

#endif /* gUseBootloader_d */