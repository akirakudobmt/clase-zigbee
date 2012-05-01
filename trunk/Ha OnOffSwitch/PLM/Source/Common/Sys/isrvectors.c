/************************************************************************************
* This module contains the IRQ vector table
*
*
* (c) Copyright 2010, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#include "EmbeddedTypes.h"
#if defined(PROCESSOR_MC1323X)
#include "Mc1323xHandler.h"
#else
#include "Mc1319xHandler.h"
#endif
#include "TMR_Interface.h"
#include "Uart_Interface.h"
#include "IIC_Interface.h"
#include "Cmt_Interface.h"
#if defined(PROCESSOR_MC1323X)
	#include "SPI_Interface.h"
	#include "ACC_Interface.h"
	#include "Touchpad_Interface.h"
#endif
#include "ApplicationConf.h"
#include "keyboard.h"
#include "IrqControlLib.h"
#include "PwrLib.h"
#include "crt0.h"
#include "IIC_Interface.h"

#if gBeeStackIncluded_d
#include "BeeStackInit.h"
#endif

/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
#ifdef MEMORY_MODEL_BANKED
typedef void __near(* __near ISR_func_t)(void);
#else
typedef void(* __near ISR_func_t)(void);
#endif


/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/

#ifdef gUseBootloader_d
  /* When bootloader is used, on QE128 and MC1323x, the interrupt vectors must be redirected 
  from software */  
  #if defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X)
    #pragma CODE_SEG IRQ_VECTOR_SECTION_DEFAULT
    
    void __interrupt near JumpFunctionISR31(void);
    void __interrupt near JumpFunctionISR30(void);
    void __interrupt near JumpFunctionISR29(void);
    void __interrupt near JumpFunctionISR28(void);
    void __interrupt near JumpFunctionISR27(void);
    void __interrupt near JumpFunctionISR26(void);
    void __interrupt near JumpFunctionISR25(void);
    void __interrupt near JumpFunctionISR24(void);
    void __interrupt near JumpFunctionISR23(void);
    void __interrupt near JumpFunctionISR22(void);
    void __interrupt near JumpFunctionISR21(void);
    void __interrupt near JumpFunctionISR20(void);
    void __interrupt near JumpFunctionISR19(void);
    void __interrupt near JumpFunctionISR18(void);
    void __interrupt near JumpFunctionISR17(void);
    void __interrupt near JumpFunctionISR16(void);
    void __interrupt near JumpFunctionISR15(void);
    void __interrupt near JumpFunctionISR14(void);
    void __interrupt near JumpFunctionISR13(void);
    void __interrupt near JumpFunctionISR12(void);
    void __interrupt near JumpFunctionISR11(void);
    void __interrupt near JumpFunctionISR10(void);
    void __interrupt near JumpFunctionISR09(void);
    void __interrupt near JumpFunctionISR08(void);
    void __interrupt near JumpFunctionISR07(void);
    void __interrupt near JumpFunctionISR06(void);
    void __interrupt near JumpFunctionISR05(void);
    void __interrupt near JumpFunctionISR04(void);
    void __interrupt near JumpFunctionISR03(void);
    void __interrupt near JumpFunctionISR02(void);
    void __interrupt near JumpFunctionISR01(void);
    
    #pragma CODE_SEG DEFAULT
  #endif /* defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X) */
#endif /* defined(gUseBootloader_d) */

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/

/* Reset vector  */
#pragma CONST_SEG RESET_VECTOR_SECTION

const ISR_func_t Reset_vector[] = 

#ifdef gUseBootloader_d
  {_BootEntryPoint};    // Vector 0    Reset vector
#else
  {_Startup};           // Vector 0    Reset vector
#endif

#pragma CONST_SEG DEFAULT


/* Interrupt vector table */
#pragma CONST_SEG IRQ_VECTOR_SECTION

#if defined(gUseBootloader_d)&&( defined(PROCESSOR_QE128)|| defined(PROCESSOR_MC1323X))

const ISR_func_t ISR_vectors[] =
/* When bootloader is used, on QE128 and MC1323x, the interrupt vectors must be redirected 
    from software */  
    
{
      JumpFunctionISR31,
      JumpFunctionISR30,
      JumpFunctionISR29,
      JumpFunctionISR28,
      JumpFunctionISR27,
      JumpFunctionISR26,
      JumpFunctionISR25,
      JumpFunctionISR24,
      JumpFunctionISR23,
      JumpFunctionISR22,
      JumpFunctionISR21,
      JumpFunctionISR20,
      JumpFunctionISR19,
      JumpFunctionISR18,
      JumpFunctionISR17,
      JumpFunctionISR16,
      JumpFunctionISR15,
      JumpFunctionISR14,
      JumpFunctionISR13,
      JumpFunctionISR12,
      JumpFunctionISR11,
      JumpFunctionISR10,
      JumpFunctionISR09,
      JumpFunctionISR08,
      JumpFunctionISR07,
      JumpFunctionISR06,
      JumpFunctionISR05,
      JumpFunctionISR04,
      JumpFunctionISR03,
      JumpFunctionISR02,
      JumpFunctionISR01,
};

#elif defined(PROCESSOR_QE128)

/* ISR Vectors definition for HCS08QE128 */
const ISR_func_t ISR_vectors[] =
{
        // Interrupt table
        Default_Dummy_ISR,       // vector 31   - TPM3 overflow
        Default_Dummy_ISR,       // vector 30   - TPM3 channel 5
        Default_Dummy_ISR,       // vector 29   - TPM3 channel 4
        Default_Dummy_ISR,       // vector 28   - TPM3 channel 3
        Default_Dummy_ISR,       // vector 27   - TPM3 channel 2
        Default_Dummy_ISR,       // vector 26   - TPM3 channel 1
        Default_Dummy_ISR,       // vector 25   - TPM3 channel 0
#if (gLpmIncluded_d == 1)
        PWRLib_RTIClock_ISR,     // vector 24   - Real time interrupt
#else
        Default_Dummy_ISR,       // vector 24   - Real time interrupt
#endif
        gUart2_TxIsr_c   ,       // vector 23   - SCI2 transmit
        gUart2_RxIsr_c   ,       // vector 22   - SCI2 receive
        gUart2_ErrorIsr_c,       // vector 21   - SCI2 error
        Default_Dummy_ISR,       // vector 20   - Analog comparator x
        Default_Dummy_ISR,       // vector 19   - ADC
        
        Switch_Press_ISR,        // vector 18   - Keyboard x pins
#if (gIIC_Enabled_d == 1)
        IIC_Isr,                 // vector 17   - IICx control        
#else
        Default_Dummy_ISR,       // vector 17   - IICx control
#endif

        gUart1_TxIsr_c,          // vector 16   - SCI1 transmit
        gUart1_RxIsr_c,          // vector 15   - SCI1 receive
        gUart1_ErrorIsr_c,       // vector 14   - SCI1 error

        Default_Dummy_ISR,       // vector 13   - SPI1
        Default_Dummy_ISR,       // vector 12   - SPI2

        Default_Dummy_ISR,       // vector 11   - TPM2 overflow
        Default_Dummy_ISR,       // vector 10   - TPM2 channel 2
        Default_Dummy_ISR,       // vector 09   - TPM2 channel 1
        Default_Dummy_ISR,       // vector 08   - TPM2 channel 0        

        Default_Dummy_ISR,       // vector 07   - TPM1 overflow
        Default_Dummy_ISR,       // vector 06   - TPM1 channel 2
        Default_Dummy_ISR,       // vector 05   - TPM1 channel 1
        TMR_InterruptHandler,    // vector 04   - TPM1 channel 0

        Default_Dummy_ISR,       // vector 03   - Low voltage detect, Low voltage warning
        MC1319xInterrupt,        // vector 02   - External IRQ (MC1319x interrupt)
        Default_Dummy_ISR,       // vector 01   - Software interrupt (SWI)
                                 // vector 00   - Reset (Watchdog timer, Low voltage detect, external pin, illegal opcode)
};

#elif defined(PROCESSOR_MC1323X)
/* ISR Vectors definition for MC1323X */
const ISR_func_t ISR_vectors[] =
{
        // Interrupt table
        Default_Dummy_ISR,       // vector 31   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 30   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 29   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 28   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 27   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 26   - NOT A REAL VECTOR
        
#if (gLpmIncluded_d == 1)
        PWRLib_RTIClock_ISR,     // vector 25   - Real time interrupt
#else
        Default_Dummy_ISR,       // vector 25   - Real time interrupt
#endif
        Switch_Press_ISR,        // vector 24   - KBI1

#if ((gTouchpadIncluded_d == 1) && (gTP_Mode_c == gTP_KbiIsrMode_c))
        TP_InterruptHandler,     // vector 23   - KBI2
#else       
        Default_Dummy_ISR,       // vector 23   - KBI2
#endif

#if (gIIC_Enabled_d == 1)        
        IIC_Isr,                 // vector 22   - IIC 
#else
        Default_Dummy_ISR,       // vector 22   - IIC
#endif
        
#if ( gCmtIncluded_d == 1)       
        CMT_InterruptHandler,    // vector 21   - CMT
#else
        Default_Dummy_ISR,       // vector 21   - CMT
#endif        
        gUart1_TxIsr_c,          // vector 20   - SCI1 transmit
        gUart1_RxIsr_c,          // vector 19   - SCI1 receive        
        gUart1_ErrorIsr_c,       // vector 18   - SCI1 error
#if(gSPI_Enabled_d == 1) 
        SPI_Isr,                 // vector 17   - SPI      
#else       
        Default_Dummy_ISR,       // vector 17   - SPI        
#endif 

        Default_Dummy_ISR,       // vector 16   - TPM4 overflow
        Default_Dummy_ISR,       // vector 15   - TPM4 channel 0
        
        Default_Dummy_ISR,       // vector 14   - TPM3 overflow
        Default_Dummy_ISR,       // vector 13   - TPM3 channel 0
        
        Default_Dummy_ISR,       // vector 12   - TPM2 overflow
        Default_Dummy_ISR,       // vector 11   - TPM2 channel 0
        
        Default_Dummy_ISR,       // vector 10   - TPM1 overflow
        TMR_InterruptHandler,    // vector 09   - TPM1 channel 0
        
        Default_Dummy_ISR,       // vector 08   - AES 128 Cipher      

        PhyHandlerRxWtrmrk_ISR,  // vector 07   - 802.15.4  Watermark
        PhyHandlerTmr_ISR,       // vector 06   - 802.15.4  Timers
        PhyHandlerTx_ISR,        // vector 05   - 802.15.4  Transmit
        PhyHandlerRx_ISR,        // vector 04   - 802.15.4  Receive

        Default_Dummy_ISR,       // vector 03   - Low voltage detect, Low voltage warning
#if gAccelerometerSupported_d
        ACC_Isr,                 // vector 02   - External IRQ 
#else
        Default_Dummy_ISR,       // vector 02   - External IRQ 
 #endif
        Default_Dummy_ISR,       // vector 01   - Software interrupt (SWI)
                                 // vector 00   - Reset (Watchdog timer, Low voltage detect, external pin, illegal opcode)
};


#else
/* ISR Vectors definition for HCS08 */
const ISR_func_t ISR_vectors[] =
{
        // Interrupt table
        Default_Dummy_ISR,       // vector 31   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 30   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 29   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 28   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 27   - NOT A REAL VECTOR
        Default_Dummy_ISR,       // vector 26   - NOT A REAL VECTOR
#if (gLpmIncluded_d == 1)
        PWRLib_RTIClock_ISR,     // vector 25   Real time interrupt
#else
        Default_Dummy_ISR,       // vector 25   Real time interrupt
#endif
#if (gIIC_Enabled_d == 1)
        IIC_Isr,                 // vector 24   IIC control
#else
        Default_Dummy_ISR,       // vector 24   IIC control
#endif
        
        Default_Dummy_ISR,       // vector 23   AD conversion complete

        Switch_Press_ISR,        // vector 22    Keyboard pins

        gUart2_TxIsr_c,          // vector 21  Uart2 transmit
        gUart2_RxIsr_c,          // vector 20  Uart2 receive
        gUart2_ErrorIsr_c,       // vector 19  Uart2 error

        gUart1_TxIsr_c,          // vector 18  Uart1 transmit
        gUart1_RxIsr_c,          // vector 17  Uart1 receive
        gUart1_ErrorIsr_c,       // vector 16  Uart1 error

        Default_Dummy_ISR,       // vector 15   SPI

        Default_Dummy_ISR,       // vector 14   TPM2 overflow
        Default_Dummy_ISR,       // vector 13   TPM2 channel 4
        Default_Dummy_ISR,       // vector 12   TPM2 channel 3
        Default_Dummy_ISR,       // vector 11   TPM2 channel 2
        Default_Dummy_ISR,       // vector 10   TPM2 channel 1
        Default_Dummy_ISR,       // vector 9    TPM2 channel 0

        Default_Dummy_ISR,       // vector 8    TPM1 overflow
        Default_Dummy_ISR,       // vector 7    TPM1 channel 2
        Default_Dummy_ISR,       // vector 6    TPM1 channel 1
        TMR_InterruptHandler,    // vector 5    TPM1 channel 0

        FLL_Lost_Lock_ISR,       // vector 4    ICG (FLL lock of clock)
        Default_Dummy_ISR,       // vector 3    Low voltage detect
        MC1319xInterrupt,        // vector 2    External IRQ (MC1319x interrupt)
        Default_Dummy_ISR,       // vector 1    Software interrupt (SWI)
                                 // vector 0    Reset (Watchdog timer, Low voltage detect, external pin, illegal opcode)
};



#endif

// **************************************************************************
/******************************************************************************
*******************************************************************************
* Public Functions
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

#ifdef gUseBootloader_d
  /* When bootloader is used, on QE128 and MC1323x, the interrupt vectors must be redirected 
  from software */  
  #if defined(PROCESSOR_QE128)
    #pragma CODE_SEG IRQ_VECTOR_SECTION_DEFAULT
      
        void __interrupt near JumpFunctionISR31(void){ Default_Dummy_ISR(); };
        void __interrupt near JumpFunctionISR30(void){ Default_Dummy_ISR(); };
        void __interrupt near JumpFunctionISR29(void){ Default_Dummy_ISR(); };
        void __interrupt near JumpFunctionISR28(void){ Default_Dummy_ISR(); };
        void __interrupt near JumpFunctionISR27(void){ Default_Dummy_ISR(); };
        void __interrupt near JumpFunctionISR26(void){ Default_Dummy_ISR(); };
        void __interrupt near JumpFunctionISR25(void){ Default_Dummy_ISR(); };
      #if (gLpmIncluded_d == 1)
        void __interrupt near JumpFunctionISR24(void){ PWRLib_RTIClock_ISR(); };
      #else
        void __interrupt near JumpFunctionISR24(void){ Default_Dummy_ISR(); };
      #endif
        void __interrupt near JumpFunctionISR23(void){gUart2_TxIsr_c();};
        void __interrupt near JumpFunctionISR22(void){gUart2_RxIsr_c();};
        void __interrupt near JumpFunctionISR21(void){gUart2_ErrorIsr_c();};
        void __interrupt near JumpFunctionISR20(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR19(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR18(void){Switch_Press_ISR();};
      #if (gIIC_Enabled_d == 1)
        void __interrupt near JumpFunctionISR17(void){IIC_Isr();};        
      #else
        void __interrupt near JumpFunctionISR17(void){Default_Dummy_ISR();};
      #endif
        void __interrupt near JumpFunctionISR16(void){gUart1_TxIsr_c();};
        void __interrupt near JumpFunctionISR15(void){gUart1_RxIsr_c();};
        void __interrupt near JumpFunctionISR14(void){gUart1_ErrorIsr_c();};
        void __interrupt near JumpFunctionISR13(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR12(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR11(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR10(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR09(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR08(void){Default_Dummy_ISR();};        
        void __interrupt near JumpFunctionISR07(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR06(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR05(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR04(void){TMR_InterruptHandler();};
        void __interrupt near JumpFunctionISR03(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR02(void){MC1319xInterrupt();};
        void __interrupt near JumpFunctionISR01(void){Default_Dummy_ISR();};    
    
    #pragma CODE_SEG DEFAULT 
  #elif defined(PROCESSOR_MC1323X)
    #pragma CODE_SEG IRQ_VECTOR_SECTION_DEFAULT
    
        void __interrupt near JumpFunctionISR31(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR30(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR29(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR28(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR27(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR26(void){Default_Dummy_ISR();};
      #if (gLpmIncluded_d == 1)
        void __interrupt near JumpFunctionISR25(void){PWRLib_RTIClock_ISR();};
      #else
        void __interrupt near JumpFunctionISR25(void){Default_Dummy_ISR();};
      #endif
        void __interrupt near JumpFunctionISR24(void){Switch_Press_ISR();};
        void __interrupt near JumpFunctionISR23(void){Default_Dummy_ISR();};
      #if (gIIC_Enabled_d == 1)        
        void __interrupt near JumpFunctionISR22(void){IIC_Isr();};
      #else
        void __interrupt near JumpFunctionISR22(void){Default_Dummy_ISR();};
      #endif
      #if ( gCmtIncluded_d == 1)       
        void __interrupt near JumpFunctionISR21(void){CMT_InterruptHandler();};
      #else
        void __interrupt near JumpFunctionISR21(void){Default_Dummy_ISR();};
      #endif        
        void __interrupt near JumpFunctionISR20(void){gUart1_TxIsr_c();};
        void __interrupt near JumpFunctionISR19(void){gUart1_RxIsr_c();};       
        void __interrupt near JumpFunctionISR18(void){gUart1_ErrorIsr_c();};
        void __interrupt near JumpFunctionISR17(void){Default_Dummy_ISR();};        
        void __interrupt near JumpFunctionISR16(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR15(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR14(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR13(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR12(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR11(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR10(void){Default_Dummy_ISR();};
        void __interrupt near JumpFunctionISR09(void){TMR_InterruptHandler();};
        void __interrupt near JumpFunctionISR08(void){Default_Dummy_ISR();};     
        void __interrupt near JumpFunctionISR07(void){PhyHandlerRxWtrmrk_ISR();};
        void __interrupt near JumpFunctionISR06(void){PhyHandlerTmr_ISR();};
        void __interrupt near JumpFunctionISR05(void){PhyHandlerTx_ISR();};
        void __interrupt near JumpFunctionISR04(void){PhyHandlerRx_ISR();};
        void __interrupt near JumpFunctionISR03(void){Default_Dummy_ISR();};
      #if (gTouchpadIncluded_d == 1)
        void __interrupt near JumpFunctionISR02(void){TP_InterruptHandler();};
      #else       
        void __interrupt near JumpFunctionISR02(void){Default_Dummy_ISR();};
      #endif
        void __interrupt near JumpFunctionISR01(void){Default_Dummy_ISR();};
        
    #pragma CODE_SEG DEFAULT 
  
  #endif /* defined(PROCESSOR_QE128) || defined(PROCESSOR_MC1323X) */
#endif /* defined(gUseBootloader_d) */
