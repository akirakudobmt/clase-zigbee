/************************************************************************************
* Crt0.c
*
* This module contains the cold and warm-boot startup code, called by the MCU before 
* calling main(). Sets up the stack, copies initialized data to RAM, etc...
*
* Note! Basic code is adapted (start08.c) from some code developed by Metrowerks.
* Not used code has been deleted. Code has been cleaned up and rearranged.
*
*
* Copyright (c) 2010, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/


/**********************************************************************/
// Includes

#include "EmbeddedTypes.h"
#include "Crt0.h"
#include "IoConfig.h"
#include "NV_Data.h"
#include "NV_FlashHAL.h"
#include "NV_Flash.h"
#include "IrqControlLib.h"
#include "FunctionLib.h"

#ifdef gUseBootloader_d
  #include "Bootloader.h"
#endif

/**********************************************************************/

#pragma MESSAGE DISABLE C20001 // Warning C20001: Different value of stackpointer depending on control-flow
#pragma MESSAGE DISABLE C4200  // Warning C4200: Other segment than in previous declaration

/**********************************************************************/

// Define the STARTUP data segement. Start code must be allocated to this segment.
#pragma DATA_SEG FAR _STARTUP

// Define a _startup_Data structure. The linker searches for the name "_startup_Data". The linker
// initialize the structure and the structure is allocated in ROM

// Store for init code stored in flash
_startupData_t _startupData;


unsigned char *pStack;

// **************************************************************************

#ifdef gUseBootloader_d
  
  #pragma CONST_SEG BOOT_FLAG
    const volatile uint8_t  bNewBootImageAvailable   = gBootValueForFALSE_c;
    const volatile uint8_t  bBootProcessCompleted    = gBootValueForTRUE_c;
  #pragma CONST_SEG DEFAULT
  
  #pragma CONST_SEG BOOTLOADER_DATA_ROM
    const volatile uint8_t bootValueForTRUE = gBootValueForTRUE_c;
    const volatile uint8_t boot_Version[2] 	= {0x01,0x00};
  #pragma CONST_SEG DEFAULT 
#endif


#pragma CONST_SEG NV_REGISTERS_SECTION // Section to store


#if defined PROCESSOR_QE128
const volatile NV_REG_t none_volatile_struct =
{   // DO NOT CHANGE
    NV_FTRIM_VALUE,
    NV_TRIM_VALUE,
    NV_BACKKEY_VALUE,
    NV_RESERVED_VALUE,
    NV_PROT_VALUE,
    NV_RESERVED1_VALUE,
    NV_OPT_VALUE
};
#elif defined PROCESSOR_MC1323X
  const volatile NV_REG_t none_volatile_struct =
{   // DO NOT CHANGE

    NV_BACKKEY_VALUE,
    NV_RESERVED_VALUE,
    NV_PROT_VALUE,
    NV_RESERVED1_VALUE,
    NV_OPT_VALUE
};
#else
// HCS08 NV register struct with values.
const volatile NV_REG_t none_volatile_struct =
{   // DO NOT CHANGE
    NV_BACKKEY_VALUE,
    NV_RESERVED_VALUE,
    NV_PROT_VALUE,
    NV_ICG_TRIM_VALUE,
    NV_OPT_VALUE
};
#endif /* PROCESSOR_QE128 */

#pragma CONST_SEG DEFAULT

/**********************************************************************/

// Local prototypes
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
static void Copy_Down(void);
static void Finish(void);

/**********************************************************************/
// External prototypes

extern void _COPY_L(void); // Function from RTSHC08.C.o (ansiis.lib)
#pragma CODE_SEG DEFAULT

extern char __SEG_START_SSTACK[];
extern char __SEG_END_SSTACK[];

extern void main_interface(void); // function in main.c

/****************************************************************************************************/
/* Function name:       loadByte()                                                                  */
/* Description:                                                                                     */
/* Input parameter:                                                                                 */
/* Output parameter:                                                                                */
/****************************************************************************************************/
/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
    #pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
    #pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

static void near loadByte(void)
{
asm
{
             PSHH
             PSHX

             LDHX    5,SP
             LDA     0,X
             AIX     #1
             STHX    5,SP
             PULX
             PULH
             RTS
} // ASM end
}

/****************************************************************************************************/
/* Function name:   Init()                                                                          */
/* Description:         1) zero out RAM-areas where data is allocated                               */
/*                      2) init run-time data                                                       */
/*                      3) copy initialization data from ROM to RAM                                 */
/* Input parameter:                                                                                 */
/* Output parameter:                                                                                */
/****************************************************************************************************/
void Init(void)
{
int i;
int *far p;

asm
{
ZeroOut:     ;
             LDA    STARTUP_DATA.nofZeroOuts:1 ; nofZeroOuts
             INCA
             STA    i:1                        ; i is counter for number of zero outs
             LDA    STARTUP_DATA.nofZeroOuts:0 ; nofZeroOuts
             INCA
             STA    i:0
             LDHX   STARTUP_DATA.pZeroOut      ; *pZeroOut
             BRA    Zero_5
Zero_3:    ;
            ; CLR    i:1 is already 0
Zero_4:    ;
             ; { HX == _pZeroOut }
             PSHX
             PSHH
             ; { nof bytes in (int)2,X }
             ; { address in (int)0,X   }
             LDA    0,X
             PSHA
             LDA    2,X
             INCA
             STA    p                  ; p:0 is used for high byte of byte counter
             LDA    3,X
             LDX    1,X
             PULH
             INCA
             BRA    Zero_0
Zero_1:    ;
           ;  CLRA   A is already 0, so we do not have to clear it
Zero_2:    ;
             CLR    0,X
             AIX    #1
Zero_0:    ;
             DBNZA  Zero_2
Zero_6:
             DBNZ   p, Zero_1
             PULH
             PULX                           ; restore *pZeroOut
             AIX    #4                      ; advance *pZeroOut
Zero_5:    ;
             DBNZ   i:1, Zero_4
             DBNZ   i:0, Zero_3
             ;
CopyDown:    JSR    Copy_Down;

} // ASM end
}

/****************************************************************************************************/
/* Function name:   Copy_Down()                                                                     */
/* Description:     _startupData.toCopyDownBeg  --->                                                */
/*                  {nof(16) dstAddr(16) {bytes(8)}^nof} Zero(16)                                   */
/* Input parameter: None                                                                            */
/* Output parameter:    None                                                                        */
/* Error handling:      None                                                                        */
/****************************************************************************************************/

static void Copy_Down(void)
{
int i;

asm
{
             LDHX   STARTUP_DATA.toCopyDownBeg:toCopyDownBegOffs
             PSHX
             PSHH
Loop0:
             JSR    loadByte  ; load high byte counter
             TAX              ; save for compare
             INCA
             STA    i
             JSR    loadByte  ; load low byte counter
             INCA
             STA    i:1
             DECA
             BNE    notfinished
             CBEQX  #0, finished
notfinished:

             JSR    loadByte  ; load high byte ptr
             PSHA
             PULH
             JSR    loadByte  ; load low byte ptr
             TAX              ; HX is now destination pointer
             BRA    Loop1
Loop3:
Loop2:
             JSR    loadByte  ; load data byte
             STA    0,X
             AIX    #1
Loop1:
             DBNZ   i:1, Loop2
             DBNZ   i:0, Loop3
             BRA    Loop0

finished:
             AIS #2

             JSR    Finish;

} // ASM end
}

/****************************************************************************************************/
/* Function name:   Finish()                                                                        */
/* Description:     optimized asm version. Some bytes (ca 3) larger than C version                  */
/*                  (when considering the runtime routine too), but about 4 times                   */
/*                  faster.                                                                         */
/* Input parameter: None                                                                            */
/* Output parameter:    None                                                                        */
/* Error handling:      None                                                                        */
/****************************************************************************************************/

static void Finish(void)
{
asm
{
             LDHX   STARTUP_DATA.toCopyDownBeg:toCopyDownBegOffs
next:
             LDA   0,X    ; list is terminated by 2 zero bytes
             ORA   1,X
             BEQ copydone
             PSHX         ; store current position
             PSHH
             LDA   3,X    ; psh dest low
             PSHA
             LDA   2,X    ; psh dest high
             PSHA
             LDA   1,X    ; psh cnt low
             PSHA
             LDA   0,X    ; psh cnt high
             PSHA
             AIX   #4
             JSR  _COPY_L ; copy one block
             PULH
             PULX
             TXA
             ADD   1,X    ; add low
             PSHA
             PSHH
             PULA
             ADC   0,X    ; add high
             PSHA
             PULH
             PULX
             AIX   #4
             BRA next
copydone:

} // ASM end
}

#ifdef gUseBootloader_d
  #pragma CODE_SEG __NEAR_SEG APP_STARTUP_FUNCTION
#else  
  #pragma CODE_SEG DEFAULT
#endif /* gUseBootloader_d */
  /* Place it in NON_BANKED memory */
  #ifdef MEMORY_MODEL_BANKED
    #pragma CODE_SEG __NEAR_SEG NON_BANKED
  #else
    #pragma CODE_SEG DEFAULT
  #endif /* MEMORY_MODEL_BANKED */

  

/****************************************************************************************************/
/* Function name:   _Startup()                                                                      */
/* Description:     Entry point for the SW. DO NOT CHANGE FUNCTION NAME.                            */
/*                  It will initialize the stack and the ZI and RW memory.                          */
/* Input parameter: None                                                                            */
/* Output parameter:None                                                                            */
/* Error handling:  None                                                                            */
/****************************************************************************************************/
void _Startup(void)
{
  /* Note that the stack pointer may not be initialized yet, so this function */
  /* cannot have any local data. */

  // Disable interrupts. Only needed for warm restarts. The HCS08 comes out of
  // reset with interupts disabled.
  IrqControlLib_DisableAllIrqs();

  // Setup SIM options
  SIMOPT  = SYSTEMS_OPTION_REG_VALUE;
 #if defined (PROCESSOR_MC1323X)
  SIMOPT2 = SYSTEMS_OPTION2_REG_VALUE;
 #endif

  // Setup stack
  if ( !( _startupData.flags & STARTUP_FLAGS_NOT_INIT_SP )) 
    {
 #if (STACK_FILL)
	    // Initialize the stack
	    pStack = __SEG_START_SSTACK;
	    while ( pStack < __SEG_END_SSTACK )
	    {
	      *pStack++ = STACK_INIT_VALUE;
	    }
 #endif	    
    // Initialize the stack pointer */
    __asm LDHX @__SEG_END_SSTACK;
    __asm TXS;
      }    
  
  #ifdef PROCESSOR_HCS08
  // Change default SCM (mode 2) bus frequency from ~4 MHz to ~8 MHz
  // SCM is only temporary until Abel is ready. No precise frequency is required.
  // Note! FLL is bypassed in SCM mode.

  // Store ICG filter values to change bus frequency for self clock mode
  // Filter values are copied from NV RAM - Search for valid NV RAM

  // Check NV RAM section 0

   ICGFLTL = gHardwareParameters.NV_ICGFLTL;
   ICGFLTH = gHardwareParameters.NV_ICGFLTU;
   
  // A few nops for clock to settle.
   __asm nop
   __asm nop
   __asm nop
   __asm nop
  #endif   

  CALL_MAIN_INTERFACE
}
#pragma CODE_SEG DEFAULT

#ifdef gUseBootloader_d
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
void _BootEntryPoint(void)
{
   // Disable interrupts. Only needed for warm restarts. The HCS08 comes out of
  // reset with interupts disabled.
  IrqControlLib_DisableAllIrqs();
  
  /* Check if there is no boot image available in the external EEPROM and if eventually
  the booting of the previous one has been completed. If both conditions are met, start
  running the application in the internal Flash. Else, start the process of booting from
  external EEPROM */   
  if((bNewBootImageAvailable != bootValueForTRUE) && (bBootProcessCompleted == bootValueForTRUE))
  {
    __asm jmp _Startup;  
  }

  // Initialize the stack pointer */
  __asm LDHX @__SEG_END_SSTACK;
  __asm TXS;
  
  // Setup SIM options
  SIMOPT  = SYSTEMS_OPTION_REG_VALUE;
 
  #if defined (PROCESSOR_MC1323X)
    SIMOPT2 = SYSTEMS_OPTION2_REG_VALUE;
  #endif
  
  /* This function will never exit */
  Boot_LoadImage();
}
#endif


// **************************************************************************
