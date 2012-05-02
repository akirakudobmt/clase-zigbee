/************************************************************************************
* This library provides the MAC/PHY with primitives for interrupt protection,
* configuration, checking and acknowledgement. Protection primitives are provided
* both for the general MCU interrupts as well as for the specific MC1319x interrupts,
* while configuration, checking and acknowledgement primitives exist only for the
* MC1319x interrupts. 
*
* Copyright (c) 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#ifndef _IRQCONTROLLIB_H_
#define _IRQCONTROLLIB_H_

#include "IoConfig.h"
#include "EmbeddedTypes.h"

#ifdef PROCESSOR_MC1323X
#include "PhyMC1323X.h"
#endif /* PROCESSOR_MC1323X */


/************************************************************************************
* Keyword to signify that a function must be treated as an interrupt
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#if defined(gUseBootloader_d)&&( defined(PROCESSOR_QE128)|| defined(PROCESSOR_MC1323X))
  #define INTERRUPT_KEYWORD 
#else
  #define INTERRUPT_KEYWORD __interrupt 
#endif

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */

/************************************************************************************
* Dummy interrupt vector
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
extern INTERRUPT_KEYWORD void Default_Dummy_ISR(void);
#pragma CODE_SEG DEFAULT

/************************************************************************************
* Dummy interrupt vector
*   
* Interface assumptions:
*   None
*   
************************************************************************************/
extern INTERRUPT_KEYWORD void FLL_Lost_Lock_ISR(void);

/************************************************************************************/
/************************************************************************************/
// Micro Processor Irq Control
/************************************************************************************/
/************************************************************************************/

/************************************************************************************
* Primitives to enable/disable all processor interrupts.
* Through immediate access and no backup handles the state of the interrupt mask bit 
* in the status register.
* 
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#define IrqControlLib_EnableAllIrqs()       {__asm CLI;}    
#define IrqControlLib_DisableAllIrqs()      {__asm SEI;} 

/************************************************************************************
* Primitives to protect/unprotect from processor interrupts.
*   - a safer/slower way of enabling/disabling processor interrupts. 
* Using backup backup of irq status register handles the state
* of the interrupt mask bit in the status register.
* 
* The backup/restore primitives use a local variable for backup.
* The push/pull primitives use the stack. 
*
* NOTE: 
*   While push/pull is slightly faster than backup/restore, it is potentially more risky. 
*   Care must be taken to not corrupt the stack inside the push/pull protection! 
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#define IrqControlLib_BackupIrqStatus(var)  {__asm TPA; __asm STA var;}
#define IrqControlLib_RestoreIrqStatus(var) {__asm LDA var; __asm TAP;}
// Alternate way of storing/restoring status register on stack.
// Push/Pull is used, and care must be taken not to mess up the
// stack when using these macros. Compared to IrqControlLib_BackupIrqStatus/
// IrqControlLib_RestoreIrqStatus the push/pull method results in a small
// decrease in code size, and a small increase in execution
// speed (9 bytes/11 cycles compared to 5 bytes/8 cycles) on HCS08.
#define IrqControlLib_PushIrqStatus()       {__asm TPA;  __asm PSHA;}
#define IrqControlLib_PullIrqStatus()       {__asm PULA; __asm TAP;}



/************************************************************************************/
/************************************************************************************/
// MC1319x Irq Control 
// Handled through associated processor control register (IRQSC on HCS08)
/************************************************************************************/
/************************************************************************************/


/************************************************************************************
* Primitives to enable/disable MC1319x interrupts.
* Performed locally on the processors external irq registers
* 
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#ifndef PROCESSOR_MC1323X

#define IrqControlLib_EnableMC1319xIrq()              {IRQSC |=  0x02;}      // Set IRQPE in HCS08 => IRQ pin from MC1319x enabled
#define IrqControlLib_DisableMC1319xIrq()             {IRQSC &= ~0x02;}      // Clr IRQPE in HCS08 => IRQ pin from MC1319x disabled

#else  // ifdef PROCESSOR_MC1323X

#define IrqControlLib_EnableMC1323xIrq()                {PP_PHY_CTL2 &= ~PHY_IRQ_LIB_MASK;} // Clear the TRCV_MSK bit to enable interrupts
#define IrqControlLib_DisableMC1323xIrq()               {PP_PHY_CTL2 |=  PHY_IRQ_LIB_MASK;} // Set the   TRCV_MSK bit to disable interrupts

#endif // PROCESSOR_MC1323X


/************************************************************************************
* Primitives to protect/unprotect from MC1319x interrupts.
* Performed locally on the processors external irq registers.
* Utilizes a backup/restore approach
* 
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#ifndef PROCESSOR_MC1323X

#define IrqControlLib_ProtectFromMC1319xIrq(var)      {var = IRQSC; IrqControlLib_DisableMC1319xIrq(); }
#define IrqControlLib_UnprotectFromMC1319xIrq(var)    {IRQSC = var;}

#else  // ifdef PROCESSOR_MC1323X

#define IrqControlLib_ProtectFromMC1323xIrq(var)      {var =   PP_PHY_CTL2 &  PHY_IRQ_LIB_MASK;  IrqControlLib_DisableMC1323xIrq(); }
#define IrqControlLib_UnprotectFromMC1323xIrq(var)    {if(var){PP_PHY_CTL2 |= PHY_IRQ_LIB_MASK;} else {PP_PHY_CTL2 &= ~PHY_IRQ_LIB_MASK;}}

#endif // PROCESSOR_MC1323X

/************************************************************************************
* Primitive to check if irq is (still) asserted by MC1319x.
* Checks irq pin state through processors external irq registers.
* Used to ensure that predicted ISRs like Tx/RxStream, which clear the MC1319x irq source 
* does not require redundant read of the MC1319x interrupt status register (0x24).
* 
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#ifndef PROCESSOR_MC1323X
#define IrqControlLib_IsMC1319xIrqAsserted()          (IRQSC & 0x08)
#endif // PROCESSOR_MC1323X

/************************************************************************************
* Primitive to init/setup MC1319x interrupts in the processor.
* Enables external interrupt from MC1319x and configures the processor external irq
* to react to active low and edge-triggered irq.
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#ifndef PROCESSOR_MC1323X
#define IrqControlLib_InitMC1319xIrq()                {IRQSC  = 0x17;} // Enable external interrupt from MC1319x, active low and edge-triggered
#endif // PROCESSOR_MC1323X

/************************************************************************************
* Primitive to acknowledge MC1319x interrupts in the processors external irq register.
* Clears latched irqs in the processor (if such mechanism on the processor).
* On HCS08 clears IRQF bit in IRQSC register
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
#ifndef PROCESSOR_MC1323X
#define IrqControlLib_AckMC1319xIrq()                 {IRQSC |= 0x04;}
#endif // PROCESSOR_MC1323X


/***********************************************************************************/

//-----------------------------------------------------------------------------------

#endif //_IRQCONTROLLIB_H_
