/*****************************************************************************
* Declarations for the NV storage HAL for the HCS08.
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _NV_FLASHHAL_H_
#define _NV_FLASHHAL_H_

#include "icg.h"
#include "NVM_Interface.h"

/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* If a flash command fails, retry it a few times. If it still fails, an */
/* embedded device generally doesn't have any one to tell about the problem, */
/* so this number can be large. */
#define mNvFlashCmdRetries_c    32

/* Size of a flash memory page. */
#if defined PROCESSOR_MC1323X
#define gNvRawPageSize_c        1024
#else
#define gNvRawPageSize_c        512
#endif
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
 * The space for this buffer overlays the end of the stack. Its important
 * to only call flash HAL functions when other code is not using much of
 * the stack.
 *
 * If a different compiler is used, or the compiler version changes, or
 * the compiler options change, there may not be enough RAM space, which
 * may lead to some hard-to-find bugs.
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

/* End address - start address + 1 == 90 bytes. */
/* See the linker command file for the address definitions. */
/* This buffer could be somewhat smaller if byte-at-a-time programming */
/* is being used. */
#define mNvSizeof_NV_CODE_BUFFER_c  (( uint16_t ) 0x01A4 - 0x0140 )

/* Number of elements in an array. */
#ifndef NumberOfElements
#define NumberOfElements( array )   (( sizeof( array ) / ( sizeof( array[ 0 ]))))
#endif
#define gNvm_32B_default_init_c 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
#if defined PROCESSOR_MC1323X 
#define gNvmPageDefaultInit_c  {gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c \
                               } 

#else                              
#define gNvmPageDefaultInit_c  {gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c,\
                                gNvm_32B_default_init_c \
                               } 
#endif
/*****************************************************************************
******************************************************************************
* Public type declarations
******************************************************************************
*****************************************************************************/

typedef unsigned char NvRawPage_t[ gNvRawPageSize_c ];

/* An integer type large enough to contain the size of a flash memory page. */
typedef uint16_t NvSize_t;

#if gNvByteAtATimeProgramming_d

#ifdef MEMORY_MODEL_BANKED
typedef bool_t __near( * __near pfNvHalExecuteFlashCmd_t) \
                ( uint8_t data, void *pDestination, uint8_t flashCommand );
#else
typedef bool_t( * __near pfNvHalExecuteFlashCmd_t) \
                ( uint8_t data, void *pDestination, uint8_t flashCommand );
#endif /*MEMORY_MODEL_BANKED*/ 
               
#else
#ifdef MEMORY_MODEL_BANKED
typedef bool_t __near( * __near pfNvHalExecuteFlashCmd_t) \
                ( uint8_t data, void *pDestination, uint8_t flashCommand, uint8_t loopIndex );
#else
typedef bool_t( * __near pfNvHalExecuteFlashCmd_t) \
                ( uint8_t data, void *pDestination, uint8_t flashCommand, uint8_t loopIndex );
#endif /*MEMORY_MODEL_BANKED*/

#endif /*gNvByteAtATimeProgramming_d*/

/*****************************************************************************
******************************************************************************
* Public memory declarations
*****************************************************************************
*****************************************************************************/

/* Make the number of pages available to other code.
 * This must be coordinated with the linker command file and the number-
 * of-datasets definition in NV_Interface.h.
 * To change the number of data sets and/or NV storage pages:
 *      Set gNvNumberOfDataSets_c in NVM_Interface.h, and
 *      Make sure the number of entries in the NvDataSetTable[] array in
 *          NV_Data.c matches the value of gNvNumberOfDataSets_c. and
 *      Set NvNumberOfRawPages_c in NV_FlashHAL.h to >= gNvNumberOfDataSets_c + 1, and
 *      Change "NV_STORAGE_PAGE_*_SECTION" and "NV_STORAGE_PAGE_*" in Beestack.prm
 */
#if gStandardSecurity_d || gHighSecurity_d
#define gNvNumberOfRawPages_c   5
#else
#define gNvNumberOfRawPages_c   4
#endif

/* Make the addresses of the pages available to other code. */
#if gNvHalIncluded_d
extern unsigned char *const maNvRawPageAddressTable[ gNvNumberOfRawPages_c ];
#endif
#define gNvInvalidPageIndex_c   (( index_t ) -1 )

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

/* Erase one of the NV storage flash pages. */
#if gNvHalIncluded_d
void NvHalErasePage
  (
  //index_t pageIndex
  unsigned char const *pPageAddress,
  uint8_t eraseCommand
  );

/* Init the flash controller hardware. */
void NvHalInit( void );

/* Copy data from an NV storage page. */
void NvHalRead
  (
  unsigned char const *pPageAddress,//index_t srcPageIndex,
  NvSize_t srcPageOffset,
  unsigned char *pDst,
  NvSize_t dstLen
  );

/* Write data to an NV storage page. Return TRUE if successful. */
/* To write any data into flash, this function must be called to write */
/* enough bytes to fill an entire flash page, at successive page offsets. */
/* The result of doing anything else is undefined. */
bool_t NvHalWrite
  (
  unsigned char const *pPageAddress,
  NvSize_t dstPageOffset,
  unsigned char *pSource,
  NvSize_t sourceLen
  );
#if gNvMultiStorageIncluded_d  
bool_t NvHalUnbufferedWrite
 (
 	uint8_t const *pPageAddress,
	NvSize_t dstPageOffset,
	uint8_t *pSource,
	NvSize_t sourceLen
) ;
#endif

#else
#define NvHalErasePage( pPageAddress, eraseCommand)

#define NvHalInit()

#define NvHalRead(pPageAddress, srcPageOffset, pDst, dstLen)

#define NvHalWrite(pPageAddress, dstPageOffset, pSource, sourceLen)
#endif gNvHalIncluded_d

/*****************************************************************************
******************************************************************************
* Private macros
******************************************************************************
*****************************************************************************/

/* On the HCS08, the lowest numbered page in memory is reserved by the CPU, */
/* and can never be used for NV storage. */
#define gNvInvalidRawPageNumber_c       0

/* All defined HCS08 flash controller commands. */

/* Check whatever all bytes in the flash is erased. */
#define mNvFlashBlankCheckCmd_c         (( uint8_t ) 0x05 )

/* Program a single byte to a location. */
#define mNvFlashByteProgramCmd_c        (( uint8_t ) 0x20 )

/* Program multiple bytes to flash. */
#define mNvFlashBurstProgramCmd_c       (( uint8_t ) 0x25 )

/* Erase a single flash page. */
#define mNvFlashPageEraseCmd_c          (( uint8_t ) 0x40 )

/* Erase all of flash memory. */
#define mNvFlashMassEraseCmd_c          (( uint8_t ) 0x41 )

/* Bits in the HCS08 flash status register (FSTAT). */
#define FCBEF   (( uint8_t ) 0x80 )     /* Flash command buffer is empty. */
#define FCCF    (( uint8_t ) 0x40 )     /* Flash command is complete. */
#define FPVIOL  (( uint8_t ) 0x20 )     /* Flash protection violation. */
#define FACCERR (( uint8_t ) 0x10 )     /* Flash access error. */
#define FBLANK  (( uint8_t ) 0x04 )     /* Flash is entirely blank. */
#ifdef  PROCESSOR_MC1323X
#define FSECERR (( uint8_t ) 0x08 )     /* Flash security error. */
#else
#define FSECERR (( uint8_t ) 0x00 )
#endif
/* Writes to 0xFFB0-0xFFB7 are interpreted as the start of a Flash */
/* programming or erase command. */
#define FCNFGValue_c            0

/*  Define a value at selected bus clock  */
#ifdef USE_62_5KHZ_CLKO                     /* 8 MHz bus clock. */
  #define FCDIVValue_c  (0x40 + 0x05)       /* Set Flash clock to 166.7KHz */
#endif USE_62_5KHZ_CLKO

#ifdef USE_32_78KHZ_CLKO_AT_16_78HMZ        /* 16.78 MHz bus clock. */
  #define FCDIVValue_c  (0x40 + 0x0A)       /* Set Flash clock to 190.6KHz. */
#endif USE_32_78KHZ_CLKO_AT_16_78HMZ

#ifdef EXT_CRYSTAL_4MHZ_AT_8MHZ             /* 8 MHz bus clock. */
  #define FCDIVValue_c  (0x40 + 0x05)       /* Set Flash clock to 166.7KHz. */
#endif EXT_CRYSTAL_4MHZ_AT_8MHZ

#ifdef EXT_CRYSTAL_9_216MHZ_AT_9_216MHZ     /* 9.216 MHz bus clock. */
  #define FCDIVValue_c  (0x40 + 0x05)       /* Set Flash clock to 192KHz. */
#endif EXT_CRYSTAL_9_216MHZ_AT_9_216MHZ

#ifdef USE_62_5KHZ_CLKO_AT_16MHZ            /* 16 MHz bus clock. */
  #define FCDIVValue_c      (0x40 + 0x0A)   /* Set Flash clock to 181KHz */
#endif USE_62_5KHZ_CLKO_AT_16MHZ

#endif _NV_FLASHHAL_H_
