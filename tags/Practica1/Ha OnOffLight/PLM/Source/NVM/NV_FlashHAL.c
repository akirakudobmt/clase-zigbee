/*****************************************************************************
* Implementation of the non-volatile storage module flash controller HAL.
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

/* Unsigned char is sometimes used in this module instead of uint8_t. The
 * C standard guarantees that sizeof(char) == 1, and that this is the most
 * basic unit of memory. That guarantee is sometimes valuable when working
 * with memory hardware.
 */

/* Power supply variations can make both reading an writing flash unreliable,
 * but there is no means available to this code to monitor power.
 */

#include "EmbeddedTypes.h"
#include "IrqControlLib.h"

#ifndef gBeeStackIncluded_d
#define gBeeStackIncluded_d 0
#endif


#if gBeeStackIncluded_d
#include "Beestack_Globals.h"
#endif /* gBeeStackIncluded_d */

#include "NV_FlashHAL.h"

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/
#define mIllegalOpCode_c  0x82
/*****************************************************************************
******************************************************************************
* Private prototypes
******************************************************************************
*****************************************************************************/
#if gNvHalIncluded_d
static void NvHalCopyCode(void);
static void NvHalClearCode(void);
/* Low level function to send a command to the flash hardware controller. */
/* Returns TRUE if the controller does not report an error. */

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
#if gNvByteAtATimeProgramming_d == 1
static bool_t NvHalExecuteFlashCmd
  (
  uint8_t data,             /* IN: Data to be written */
  void *pDestination,       /* IN: Address where data has to be written */
  uint8_t flashCommand      /* IN: Flash command to be executed */
  );
#else
static bool_t NvHalExecuteFlashCmd
  (
  uint8_t data,             /* IN: Data to be written */
  void *pDestination,       /* IN: Address where data has to be written */
  uint8_t flashCommand,     /* IN: Flash command to be executed */
  uint8_t loopIndex
  );
#endif
#pragma CODE_SEG DEFAULT

#else
#define NvHalCopyCode()
#define NvHalClearCode()
#define NvHalExecuteFlashCmd(data, pDestination, flashCommand) FALSE
#endif /* gNvHalIncluded_d */
/*****************************************************************************
******************************************************************************
* Private type definitions
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Private memory declarations
******************************************************************************
*****************************************************************************/

#if gNvHalIncluded_d
#if !gNvByteAtATimeProgramming_d
#pragma DATA_SEG NV_SECTOR_BUFFER
/* The linker command file overlays this buffer with the end of the stack. */
/* The size of this buffer is determined by the flash controller hardware. */
static unsigned char maNvSectorBuffer[ 64 ];
#pragma DATA_SEG DEFAULT
#endif                                  /* #if !gNvByteAtATimeProgramming_d */
#endif /* gNvHalIncluded_d */

#if gNvHalIncluded_d
#pragma DATA_SEG NV_CODE_BUFFER
/* The linker command file overlays this buffer with the end of the stack. */
/* ***WARNING*** Be careful with this one. A new version of the compiler, */
/* or changes to compiler options, could change the size needed. Check */
/* the link map to ensure that there's enough space. */
static unsigned char NvRAMCodeSpace[ mNvSizeof_NV_CODE_BUFFER_c ];
#pragma DATA_SEG DEFAULT
#endif /* gNvHalIncluded_d */

/* Define the NV storage pages. */
/* The addresses are defined in the project *.prm (linker command) file. */
/* To support one data set, there must be at least two pages available. */
#if gNvHalIncluded_d
#pragma CONST_SEG NV_STORAGE_PAGE_0
NvRawPage_t const mNvPage0 = gNvmPageDefaultInit_c;
#pragma CONST_SEG DEFAULT

#pragma CONST_SEG NV_STORAGE_PAGE_1
NvRawPage_t const mNvPage1 = gNvmPageDefaultInit_c;
#pragma CONST_SEG DEFAULT

#if gNvNumberOfRawPages_c >= 3
#pragma CONST_SEG NV_STORAGE_PAGE_2
NvRawPage_t const mNvPage2 = gNvmPageDefaultInit_c;
#pragma CONST_SEG DEFAULT
#endif

#if gNvNumberOfRawPages_c >= 4
#pragma CONST_SEG NV_STORAGE_PAGE_3
NvRawPage_t const mNvPage3 = gNvmPageDefaultInit_c;
#pragma CONST_SEG DEFAULT
#endif

#if gNvNumberOfRawPages_c >= 5
#pragma CONST_SEG NV_STORAGE_PAGE_4
NvRawPage_t const mNvPage4 = gNvmPageDefaultInit_c;
#pragma CONST_SEG DEFAULT
#endif

#endif /* gNvHalIncluded_d */

/* Pointer to a utility function, copied to RAM, to execute flash commands. */
#if gNvHalIncluded_d
pfNvHalExecuteFlashCmd_t pfNvHalExecuteFlashCmd;
#endif /* gNvHalIncluded_d */

/*****************************************************************************
******************************************************************************
* Public memory definitions
******************************************************************************
*****************************************************************************/

/* Array of NV storage page addresses. */
/* *warning* There should be one more flash page than there are NV data sets. */
/* IMPORTANT: This table must be in ROM. The startup code (crt0 and */
/* PlatformInit) needs to search NV storage for a data structure that */
/* contains setup parameters for the CPU, and needs to know where the NV */
/* storage pages are located. */
#if gNvHalIncluded_d
unsigned char *const maNvRawPageAddressTable[ gNvNumberOfRawPages_c ] = {
  mNvPage0,
  mNvPage1,
#if gNvNumberOfRawPages_c >= 3
  mNvPage2,
#endif
#if gNvNumberOfRawPages_c >= 4
  mNvPage3,
#endif
#if gNvNumberOfRawPages_c >= 5
  mNvPage4
#endif
};
#endif /* gNvHalIncluded_d */

/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/* Erase a flash page, given a page address in the NVM and an erase command
 * (a page or the whole flash).

 * The HCS08 flash controller can get confused if two successive writes are 
 * done to the same location in flash without an erase between them.

 * Unfortunately, the HCS08 flash controller does not provide any completely
 * reliable way to know if a page has been erased. An erased page will contain
 * 0xFF in every byte. It is possible to write 0xFF to every byte of a page,
 * which would be indistinguishable from an erased page, except that the
 * flash controller would get confused if the same bytes were written to again.

 * Looking for all 0xFF's is the best that can be done. The NV storage code
 * must never write such a page, and it is only possible to hope that nobody
 * else does, either.

 * If this page looks like it has already been erased, don't erase it again.
 * This costs time and code space, but saves erase cycles and extends the
 * life of the flash memory.

 * If erasing fails after retries, there isn't anything that can be done
 * about it. An error return would not be useful.
 */
#if gNvHalIncluded_d
void NvHalErasePage
(
	unsigned char const *pPageAddress,
	uint8_t eraseCommand
)
{
	uint8_t ccr;
	NvSize_t i;
	index_t retries = mNvFlashCmdRetries_c;

	/* Erase the complete page starting in the given address */
	for ( i = 0; i < gNvRawPageSize_c; ++i )
	{
		/* If the info is equal to 0xFF it is possible that the page has been
			erased before, to avoid confusing the NV driver lets no write the same in
			the same byte twice. */
		if ( pPageAddress[ i ] == 0xFF )
			continue;

		/* Avoid Iterrumptions */
		IrqControlLib_BackupIrqStatus( ccr );
		IrqControlLib_DisableAllIrqs();

		/* Copy to RAM the function that work directly to flash. */
		NvHalCopyCode();

		/* Keep trying a fixed amount of times or until success. */
		while ( retries-- )
		{
			/* Erase the page. */
#if gNvByteAtATimeProgramming_d
			if ( pfNvHalExecuteFlashCmd( 0, ( void * ) pPageAddress, eraseCommand ))
#else
			if ( pfNvHalExecuteFlashCmd( 0, ( void * ) pPageAddress, eraseCommand, sizeof( maNvSectorBuffer)))
#endif
				break;
		}/* while ( retries-- ) */
		NvHalClearCode();

		/* Enalbe interruptions again */
		IrqControlLib_RestoreIrqStatus( ccr );
		return;

	}/* for ( i = 0; i < gNvRawPageSize_c; ++i ) */
}/* NvHalErasePage() */
#endif /* gNvHalIncluded_d */

/*****************************************************************************/
#if gNvHalIncluded_d
void NvHalInit
(
	void
)
{
	/* Initialize the flash controller. */
	FSTAT = FPVIOL | FACCERR | FSECERR;  /* Clear errors. */
#ifndef PROCESSOR_MC1323X
	FCDIV = ( 0x40 | (( uint8_t )( gHardwareParameters.Bus_Frequency_In_MHz / 2 ) + 2 ));
#endif
	FCNFG = FCNFGValue_c;
}/* NvHalInit() */
#endif /* gNvHalIncluded_d */

/*****************************************************************************/
/* Read data from a specified offset within a given address of a page of NV storage.

 * IN: The address of the NVM page to read.
 * IN: The offset in the page where to star reading the data.
 * IN: The pointer in memory where to copy the data.
 * IN: The amount of bytes to read.

 * OUT: NONE.
*/
#if gNvHalIncluded_d
void NvHalRead
(
	unsigned char const *pPageAddress,  /* IN: The address of the NVM page to read. */
	NvSize_t srcPageOffset,             /* IN: The offset in the page where to star
	                                           reading the data. */
	unsigned char *pDst,                /* IN: The pointer in memory where to copy
	                                           the data. */
	NvSize_t dstLen                     /* IN: The amount of bytes to read. */
)
{
	/* Point in to a specific address of the given page address. */
	unsigned char *pSrc = (unsigned char *)(pPageAddress + srcPageOffset);

	/* FLib_MemCpy() can't handle a 16 bit size. */
	while ( dstLen-- )
	{
		/* Copy one byte a the time of the given byte Length. */
		*pDst++ = *pSrc++;
	}
}/* NvHalRead() */
#endif /* gNvHalIncluded_d */
/****************************************************************************/

/* Write data to a specified offset within a given address of a NV storage page.
 * The caller must insure that a full page of data is written, across more than
 * one call to NvWrite() if necessary.
 * Since this routine only writes partial pages, it does not do retries. Return
 * TRUE if the write succeeded; FALSE if the flash controller reported an error.

 * IN: The address of the memory page where to write.
 * IN: The offset inside the given page address.
 * IN: The address from where the data will be copied.
 * IN: The amount of byte to copy in.

 * OUT: TRUE if succes and FALSE in any other case.
*/
#if gNvHalIncluded_d
bool_t NvHalWrite
(
	unsigned char const *pPageAddress,
	NvSize_t dstPageOffset,
	unsigned char *pSource,
	NvSize_t sourceLen
)
{
	uint8_t ccr;
	unsigned char *pDst;
	bool_t status;
#if !gNvByteAtATimeProgramming_d
	/* This variable is only used in block-mode writing. */
	index_t sectorOffset;
#endif

	/* Status returned for the write action. */
	status = TRUE;

#if gNvByteAtATimeProgramming_d
	/* Using the Byte-at-the-time mode the pointer goes directly to the offset in
		the NVM page. */
	pDst = pPageAddress + dstPageOffset;

	/* Unable Interrumptions. */
	IrqControlLib_BackupIrqStatus( ccr );
	IrqControlLib_DisableAllIrqs();

	/* Copy the function into RAM, so it canbe executed form there. */
	NvHalCopyCode();

	while ( status && sourceLen-- )
	{
		/* Copy one byte at the time into NVM. */
		status = pfNvHalExecuteFlashCmd( *pSource++, ( void * ) pDst++, mNvFlashByteProgramCmd_c );
	}
	
  NvHalClearCode();
	/* Return the previously backup IRQ status for normal execution. */
	IrqControlLib_RestoreIrqStatus( ccr );

#else  /* Use the block mode instead. */
	/* Move a byte to the static sector buffer. If the buffer is now full, send
		it's contents to the controller command function. Map the offset to the
		sector buffer. */
	sectorOffset = ( index_t ) ( dstPageOffset & ( sizeof( maNvSectorBuffer ) - 1 ));

	/* While we haven't done the full set of bytes keep going. */
	while ( status && sourceLen-- )
	{
		/* Fill up the logical sector buffer. */
		maNvSectorBuffer[ sectorOffset++ ] = *pSource++;

		/* If the logical sector buffer is full, write it into NVM. */
		if ( sectorOffset == sizeof( maNvSectorBuffer ))
		{
			/* The given offset of the logical sector buffer needs to be maped into
				the NVM physical offset.*/
			pDst = (unsigned char *)( pPageAddress + (dstPageOffset & ~( sizeof( maNvSectorBuffer ) -1 )) );

			/* Backup and disable the interruptions. */
			IrqControlLib_BackupIrqStatus( ccr );
			IrqControlLib_DisableAllIrqs();

			/* Copy the routine into RAM to execute it form there instead that NVM. */
			NvHalCopyCode();

			/* Execute the NVM command and catch the status of it */
			status = pfNvHalExecuteFlashCmd( maNvSectorBuffer[ 0 ], ( void * ) pDst, mNvFlashBurstProgramCmd_c, 0 );
			
      NvHalClearCode();
			/* Restore previous IRQ information back to normal. */
			IrqControlLib_RestoreIrqStatus( ccr );

			/* Set the new offset in the physical NVM page for the next write. */
			dstPageOffset += sizeof( maNvSectorBuffer );

			/* We already write into NVM, so the next time the logical sector buffer
				must be filled form address Zero and on. */
			sectorOffset = 0;
		}/* if ( sectorOffset == ... */
	}/* while ( status && sourceLen-- ) */
#endif

	return status;
}/* NvHalWrite() */
#endif /* gNvHalIncluded_d */

/****************************************************************************/

/* Write data to a specified offset within a given address of a NV storage page.
 * Return
 * TRUE if the write succeeded; FALSE if the flash controller reported an error.

 * IN: The address of the memory page where to write.
 * IN: The offset inside the given page address.
 * IN: The address from where the data will be copied.
 * IN: The amount of byte to copy in.

 * OUT: TRUE if succes and FALSE in any other case.
*/
#if (gNvHalIncluded_d && gNvMultiStorageIncluded_d )
bool_t NvHalUnbufferedWrite
(
	uint8_t const *pPageAddress,
	NvSize_t dstPageOffset,
	uint8_t *pSource,
	NvSize_t sourceLen
)
{
	uint8_t ccr;
	uint8_t const *pDst;
	bool_t status;
#if !gNvByteAtATimeProgramming_d
	/* This variable is only used in block-mode writing. */
	index_t sectorOffset;
#endif

	/* Status returned for the write action. */
	status = TRUE;

#if gNvByteAtATimeProgramming_d
	/* Using the Byte-at-the-time mode the pointer goes directly to the offset in
		the NVM page. */
	pDst = pPageAddress + dstPageOffset;

	/* Unable Interrumptions. */
	IrqControlLib_BackupIrqStatus( ccr );
	IrqControlLib_DisableAllIrqs();

	/* Copy the function into RAM, so it canbe executed form there. */
	NvHalCopyCode();

	while ( status && sourceLen-- )
	{
		/* Copy one byte at the time into NVM. */
		status = pfNvHalExecuteFlashCmd( *pSource++, ( void * ) pDst++, mNvFlashByteProgramCmd_c );
	}
	
  NvHalClearCode();
	/* Return the previously backup IRQ status for normal execution. */
	IrqControlLib_RestoreIrqStatus( ccr );

#else  /* Use the block mode instead. */

	pDst = pPageAddress + dstPageOffset;	
	sectorOffset = ( index_t ) ( sizeof( maNvSectorBuffer ));
	pSource += ((sourceLen > sizeof( maNvSectorBuffer )) ? sizeof( maNvSectorBuffer ):sourceLen )  - 1;

	/* While we haven't done the full set of bytes keep going. */
	while ( status && sourceLen-- ) 
	{
	  maNvSectorBuffer[--sectorOffset ] = *pSource--;
	  if((sourceLen == 0) || (sectorOffset == 0)) 
	   {
	   	IrqControlLib_BackupIrqStatus( ccr );
			IrqControlLib_DisableAllIrqs();

			/* Copy the routine into RAM to execute it form there instead that NVM. */
			NvHalCopyCode();

			/* Execute the NVM command and catch the status of it */
			status = pfNvHalExecuteFlashCmd( maNvSectorBuffer[ sectorOffset ], ( void * ) pDst, mNvFlashBurstProgramCmd_c, sectorOffset );
			
      NvHalClearCode();
			/* Restore previous IRQ information back to normal. */
			IrqControlLib_RestoreIrqStatus( ccr );
      if(sourceLen) 
        {
         pSource +=  sizeof( maNvSectorBuffer ) + 1;
         pSource += (sourceLen > sizeof( maNvSectorBuffer ) ? sizeof( maNvSectorBuffer ):sourceLen )  - 1;
         sectorOffset = ( index_t ) ( sizeof( maNvSectorBuffer ));
         pDst += sizeof( maNvSectorBuffer );
        }
	   }
	}
	
	
#endif

	return status;
}/* NvHalWrite() */
#endif /* (gNvHalIncluded_d && gNvMultiStorageIncluded_d ) */

/****************************************************************************/

/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 * There is no generally supported way to take the size of a function.
 * Some compilers support sizeof() a dereferenced function pointer, but
 * some don't. The CodeWarrior v5.1 HCS08 compiler does not.
 *
 * The NvHalExecuteFlashCmd() function is copied from code space to RAM.
 * At least enough RAM space must be reserved for it in the linker command
 * file (*.prm for HCS08 CodeWarrior v5.1). The #define below matches the
 * starting and ending RAM addresses in the prm file.
 *
 * If a different compiler is used, or the compiler version changes, or
 * the compiler options change, there may not be enough RAM space, which
 * may lead to some hard-to-find bugs.
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

#if gNvHalIncluded_d
static void NvHalCopyCode(void)
{
	index_t i;
	unsigned char *pCodeInROM;

#if gNvDebug_d
/*
	The flash HAL uses a data buffer and a code area mapped by the linker
	command file to overlay part of the stack. Check that the stack is
	short enough to leave space for these areas.
	Compilers are not constrained to allocate frame variables in declaration
	order, so it isn't knowable which local variable is further down the
	stack. The compiler may also push temporaries onto the stack. Make sure
	that there's some extra space to allow for this.
*/
	if ( NvRAMCodeSpace + sizeof( NvRAMCodeSpace) + 32 >= &i )
	{
		volatile bool_t waitForever = TRUE;
		while ( waitForever );
	}
#endif

/* Copy the function code into RAM, avoiding the default convetion. */
#pragma MESSAGE DISABLE C1805  /* Non standard conversion used */
	/* Set a function pointer into the RAM segment. */
	pfNvHalExecuteFlashCmd = ( pfNvHalExecuteFlashCmd_t ) NvRAMCodeSpace;

	/* Use byte at the time copy porcedure, so point into the place where the
		function will be copied. */
	pCodeInROM = ( uint8_t * ) NvHalExecuteFlashCmd;

	/* One byte at the time starting copying the function into RAM */
	for ( i = 0; i < mNvSizeof_NV_CODE_BUFFER_c; ++i )
	{
		NvRAMCodeSpace[ i ] = pCodeInROM[ i ];
	}
}
#endif /* gNvHalIncluded_d */

/****************************************************************************/


#if gNvHalIncluded_d
static void NvHalClearCode(void)
{
	index_t i;
	for ( i = 0; i < mNvSizeof_NV_CODE_BUFFER_c; ++i )
	{
		NvRAMCodeSpace[ i ] = mIllegalOpCode_c;
	}
}
#endif /* gNvHalIncluded_d */



/****************************************************************************
* Giving a command to the flash controller may disturb read access to flash
* addresses. This function is copied into RAM so it can run even if flash is
* not accessible.
*
* Retries here are not a good idea, both because it would add mode code that
* would reside in scarce RAM, and because some commands cannot be retried
* without doing something else first. In particular, no byte of flash should
* be written more than once without an erase between the write commands.
*
* This function should only be called while interrupts are disabled.
*
* Returns TRUE  if the command worked;
*         FALSE if the flash controller reported an error.
*****************************************************************************/

#if gNvHalIncluded_d

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

#if gNvByteAtATimeProgramming_d
/* This version uses byte-at-a-time programming. */
static bool_t NvHalExecuteFlashCmd
(
	uint8_t data,             /* IN: Data to be written */
	void *pDestination,       /* IN: Address where data has to be written */
	uint8_t flashCommand      /* IN: Flash command to be executed */
)
{
	/* Wait for flash to be ready for a command. */
	while ( !( FSTAT & FCCF ))
	{/* Do nothing for a while. */}

	/* Clear all errors. */
	FSTAT = FPVIOL | FACCERR | FSECERR;

	/* Write data to flash. This is required by the flash controller, even
		for commands that don't modify data. */
	*( unsigned char * ) pDestination = data;

	/* Load command register with flash command. */
	FCMD = flashCommand;

	/* Launch the command. */
	FSTAT = FCBEF;

	/* Wait 4 bus cycles. */
	__asm nop;
	__asm nop;
	__asm nop;
	__asm nop;

	/* Wait for the command to complete. */
	while ( !( FSTAT & FCCF ))
	{/* Do nothing for a while. */}

	if ( FSTAT & ( FPVIOL | FACCERR | FSECERR))
	{
		return FALSE;  /* An error bit is set. */
	}

	return TRUE;
}/* NvHalExecuteFlashCmd() */

#else  /* #if gNvByteAtATimeProgramming_d */

/* This version uses burst programming. */

/* Code space is vital in this routine. Speed is also important; the flash
 * controller won't wait forever for the next byte of a burst. Thus, burst
 * programming is handled with a loop here, but in an odd way. To save
 * space, the caller must provide the initial value of the loop index.
 * To use a flash command other than burst mode programming, call with the
 * loop index >= sizeof( maNvSectorBuffer ). The loop will end after the
 * first time through.
 */
static bool_t NvHalExecuteFlashCmd
(
	uint8_t data,          /* IN: Data to be written */
	void *pDestination,    /* IN: Address where data has to be written */
	uint8_t flashCommand,  /* IN: Flash command to be executed */
	uint8_t loopIndex      /* IN: Initial loop index. */
)
{
	/* Wait for flash to be ready for a command. */
	while ( !( FSTAT & FCCF ))
	{/* Do nothing for a while. */}

	/* Clear all errors. */
	FSTAT = FPVIOL | FACCERR | FSECERR;

	for (;/* Do it until a break point is reached. */;)
	{
		/* Write data to flash. This is required by the flash controller, even */
		/* for commands that don't modify data. */
		*( unsigned char * ) pDestination = data;

		/* Load command register with flash command. */
		FCMD = flashCommand;

		/* Launch the command. */
		FSTAT = FCBEF;

		/* Get the next byte form the logical sector buffer. */
		data = maNvSectorBuffer[ ++loopIndex ];
		if ( loopIndex >= sizeof( maNvSectorBuffer ))
		{
			/* If we are done get out */
			break;
		}

		/* Wait for the command buffer to clear. */
		while ( !( FSTAT & FCBEF ))
		{/* Do nothing for a while. */}

	}/* for (;;) */

	/* Wait for the command to complete. */
	while ( !( FSTAT & FCCF ))
	{/* Do nothing for a while. */}

	if ( FSTAT & ( FPVIOL | FACCERR | FSECERR))
	{
		/* If somethig got wrong, the set and return error. */
		return FALSE;  /* An error bit is set. */
	}

	/* We are totally done return success. */
	return TRUE;
}/* NvHalExecuteFlashCmd() */
#endif  /* #if !gNvByteAtATimeProgramming_d else */

#pragma CODE_SEG DEFAULT
#endif /* gNvHalIncluded_d */

/* EOF */
