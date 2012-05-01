/************************************************************************************
* This module contains some basic init code.
* Note! Basic code is adapted (start08.h and hidef.h) from some code developed by Metrowerks.
* 
*
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#ifndef _CRT0_H
#define _CRT0_H

/***********************************************************************************/

#include "EmbeddedTypes.h"
#include "RST_Interface.h"  /*for gBeeStackProject_c definition to include*/

/***********************************************************************************/

#define STACK_FILL 1            /* init stack */
#define STACK_INIT_VALUE  0x55  /* stack init value */


#if defined (PROCESSOR_QE128)
 #define SYSTEMS_OPTION_REG_VALUE	0x32 // Cop disable, short timeout, STOP mode enabled, background debug enabled, PTA5 as IRQ
#elif defined (PROCESSOR_MC1323X)
 #define SYSTEMS_OPTION_REG_VALUE	0x23 // Cop disable, short timeout, STOP mode enabled, background debug enabled
 #define SYSTEMS_OPTION2_REG_VALUE 0x00 // Cop watchdog clock select
#else
 #define SYSTEMS_OPTION_REG_VALUE	0x33 // Cop disable, short timeout, STOP mode enabled, background debug enabled
#endif
// Table with some filter values and corresponding bus frequency (measure with scope).
// MSB  LSB  => Bus frequency in MHz:

// 0x00 0x00 => ~1,9 MHz minimum SCM frequency

// 0x00 0xC0 => ~4,2 MHz default out of reset
// 0x01 0x20 => ~5,2 MHz
// 0x01 0x80 => ~6,2 MHz
// 0x01 0xE0 => ~7,1 MHz
// 0x02 0x40 => ~7,9 MHz
// 0x02 0xA0 => ~8,7 MHz
// 0x03 0x00 => ~9,5 MHz

// 0x0F 0xFF => ~30  MHz maximum SCM frequency

// Note! Do not use minimum and maximum values.

// Default filter values => ~8 MHz bus frequency
#define ICG_FILTER_MSB 0x02;
#define ICG_FILTER_LSB 0x40; // LSB value must be written first

// Definitions for none-volatile registers.
#define NV_BACKKEY_LENGTH   8
#define NV_RESERVED_LENGTH  5

// ONLY CHANGE THIS SECTION
#ifdef PROCESSOR_QE128
#define NV_FTRIM_VALUE       0x00
#define NV_TRIM_VALUE        0xa9  
#define NV_RESERVED1_VALUE   0xFF
#endif

#ifdef PROCESSOR_MC1323X
#define NV_RESERVED1_VALUE   0xFF
#endif
#define NV_BACKKEY_VALUE    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
#define NV_RESERVED_VALUE   0xFF,0xFF,0xFF,0xFF,0xFF


// Use some default values - flash programming functions not available
#ifdef gUseBootloader_d

#if defined PROCESSOR_QE128
#define NV_PROT_VALUE       (0x7e<<1)|0x01 // Any flash location (not protected) can be erased or programmed, flash is protected as
							                	 // specified in FPS[6..0] (0x0_FC00-0x0_FFFF)(=1KB).
#elif defined PROCESSOR_MC1323X
#define NV_PROT_VALUE       (0x7e<<1)|0x01// Any flash location (not protected) can be erased or programmed, flash is protected as
							                	 // specified in FPS[6..0] (0x0_FC00-0x0_FFFF)(=1KB).							                	 
#else
#define NV_PROT_VALUE       0x88 // Any flash location (not protected) can be erased or programmed, flash is protected as
							                	 // specified in FPS2:FPS1:FPS0 (0xFC00-0xFFFF)(=1KB).
#endif	

#define NV_OPT_VALUE	     	0x02 // No backdoor key allowed, vectors redirect enabled, unsecure.

#else /* Bootloader not defined */

#if defined PROCESSOR_QE128
#define NV_PROT_VALUE       0xef // Any flash location (not protected) can be erased or programmed, flash is protected as
							                	 // specified in FPS[6..0] (0x0_E000-0x0_FFFF)(=8KB).
#elif defined PROCESSOR_MC1323X
#define NV_PROT_VALUE       0xef // Any flash location (not protected) can be erased or programmed, flash is protected as
							                	 // specified in FPS[6..0] (0x0_E000-0x0_FFFF)(=8KB).							                	 
#else
#define NV_PROT_VALUE       0x98 // Any flash location (not protected) can be erased or programmed, flash is protected as
							                	 // specified in FPS2:FPS1:FPS0 (0xF000-0xFFFF)(=4KB).
#endif							                	 

#define NV_OPT_VALUE	     	0x42 // No backdoor key allowed, vectors redirect enabled, unsecure.
#endif


// Default value
#ifdef PROCESSOR_HCS08
#define NV_ICG_TRIM_VALUE   0x80 // This value must/should be calibrated by production test system.
#endif
#define FAR  // Empty - for tiny FAR is used as segment modifier
#define _FAR __far

#define toCopyDownBegOffs          0

// Only 2 flags are define
#define STARTUP_FLAGS_NONE         0
#define STARTUP_FLAGS_ROM_LIB      (1<<0)  // If module is a ROM library
#define STARTUP_FLAGS_NOT_INIT_SP  (1<<1)  // If stack pointer has not to be initialized

// LibInits is only used for ROM libraries
#define INCLUDE_ROM_LIBRARIES      0 // Switch on to implement ROM libraries

/**********************************************************************/
// Types

// HCS08 NV register type
#if defined  PROCESSOR_QE128
typedef struct NV_REG
{
  uint8_t nv_ftrim;
  uint8_t nv_trim;
	uint8_t	nv_backkey[NV_BACKKEY_LENGTH];
	uint8_t	nv_reserved[NV_RESERVED_LENGTH];
	uint8_t	nv_prot;
	uint8_t	nv_reserved1; // This value must/should be calibrated by production test system.
	uint8_t	nv_opt;
} NV_REG_t;

#elif  defined PROCESSOR_MC1323X
typedef struct NV_REG
{
	uint8_t	nv_backkey[NV_BACKKEY_LENGTH];
	uint8_t	nv_reserved[NV_RESERVED_LENGTH];
	uint8_t	nv_prot;
	uint8_t	nv_reserved1; // This value must/should be calibrated by production test system.
	uint8_t	nv_opt;
} NV_REG_t;

#else
typedef struct NV_REG
{
	uint8_t	nv_backkey[NV_BACKKEY_LENGTH];
	uint8_t	nv_reserved[NV_RESERVED_LENGTH];
	uint8_t	nv_prot;
	uint8_t	nv_icg_trim; // This value must/should be calibrated by production test system.
	uint8_t	nv_opt;
} NV_REG_t;
#endif

//  the following datastructures contain the data needed to
//  initialize the processor and memory 

typedef struct
{
	unsigned char *_FAR beg;
	int size; // [beg..beg+size]
} _Range;

typedef struct _Copy
{
	int size;
	unsigned char *_FAR dest;
} _Copy;

typedef void (*_PFunc)(void);

#ifdef INCLUDE_ROM_LIBRARIES
	typedef struct _LibInit
	{
		_PFunc  *startup; // Address of startup desc
	} _LibInit;
#endif INCLUDE_ROM_LIBRARIES

// attention: the linker scans the debug information for this structures
// to obtain the available fields and their sizes.
// So dont change the names in this file.

typedef struct _tagStartup
{
	unsigned char   flags; 					// STARTUP_FLAGS_xxx
	_PFunc          main;  					// Top level procedure of user program
	unsigned short  stackOffset; 			// Initial value of the stack pointer
	unsigned short  nofZeroOuts;			// Number of zero out ranges
	_Range          *_FAR pZeroOut;			// Vector of ranges with nofZeroOuts elements
	_Copy           *_FAR toCopyDownBeg;	// ROM-address where copydown-data begins

#if INCLUDE_ROM_LIBRARIES
	unsigned short  nofLibInits; 	        // number of library startup descriptors
	_LibInit        *_FAR libInits;         // Vector of pointers to library startup descriptors
#endif INCLUDE_ROM_LIBRARIES

} _startupData_t;    


/**********************************************************************/
extern void main(void);

#ifdef gUseBootloader_d

#pragma CONST_SEG BOOT_FLAG
  extern const volatile uint8_t  bNewBootImageAvailable;
  extern const volatile uint8_t  bBootProcessCompleted;
#pragma CONST_SEG DEFAULT


  #pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
    extern void _BootEntryPoint(void);
  #pragma CODE_SEG DEFAULT
  
  #pragma CODE_SEG __NEAR_SEG APP_STARTUP_FUNCTION
#else 
  /* Place it in NON_BANKED memory */
  #ifdef MEMORY_MODEL_BANKED
    #pragma CODE_SEG __NEAR_SEG NON_BANKED
  #else
    #pragma CODE_SEG DEFAULT
  #endif /* MEMORY_MODEL_BANKED */
#endif /* gUseBootloader_d */  
extern void _Startup (void);

#ifdef gUseBootloader_d 
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
  #pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
  #pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
#endif
  extern void Init(void);
#pragma CODE_SEG DEFAULT

#if (gBeeStackProject_c == 1)
  extern void RST_GetResetStatus(void);
#endif /*gBeeStackProject_c*/

#define STARTUP_DATA		_startupData

#if (gBeeStackProject_c == 1)
	#define CALL_MAIN_INTERFACE		Init();\
	                              RST_GetResetStatus();  \
									              main();	// Call user "main" function
#else
	#define CALL_MAIN_INTERFACE		Init();\
	                              main();	// Call user "main" function
#endif /*gBeeStackProject_c*/

/**********************************************************************/
// Prototypes data

extern const volatile NV_REG_t none_volatile_struct;

/**********************************************************************/
// Prototypes
#ifdef gUseBootloader_d

  #pragma CODE_SEG __NEAR_SEG APP_STARTUP_FUNCTION  
  
#else 
/* Place it in NON_BANKED memory */
  #ifdef MEMORY_MODEL_BANKED
    #pragma CODE_SEG __NEAR_SEG NON_BANKED
  #else
    #pragma CODE_SEG DEFAULT
  #endif /* MEMORY_MODEL_BANKED */
	  extern void _Startup (void); // DO NOT CHANGE FUNCTION NAME
#endif
#pragma CODE_SEG DEFAULT	
	extern void Copy_startup_structure(_startupData_t * copy_structure);
	extern _startupData_t _startupData; // Stored in flash
	extern _startupData_t _startupData_var;

extern void NVM_FindNVRAM(void);

/**********************************************************************/

#endif _CRT0_H

