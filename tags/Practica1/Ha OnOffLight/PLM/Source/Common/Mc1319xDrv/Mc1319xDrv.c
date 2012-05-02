/************************************************************************************
* This file contains the driver primitives for SPI and GPIO interfacing from the
* MAC/PHY to the MC1319x. The primitives may be implemented as macros or functions
* depending on the speed of the MCU and effectiveness of the compiler. There is a
* potential trade-off between code space and execution speed through this choice.
*
*
* (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#include "EmbeddedTypes.h"
#include "IoConfig.h"
#include "Mc1319xReg.h"
#include "Mc1319xDrv.h"
#include "PortConfig.h"
#include "IrqControlLib.h"
#include "ICG.h"
#include "AppToPlatformConfig.h" // For TARGET_TOROWEAP

/* Place it in NON_BANKED memory */
#ifdef MEMORY_MODEL_BANKED
#pragma CODE_SEG __NEAR_SEG NON_BANKED
#else
#pragma CODE_SEG DEFAULT
#endif /* MEMORY_MODEL_BANKED */
/************************************************************************************
* Global Driver variables for getting MC1319x GPIO pin values 
************************************************************************************/
uint8_t* pMC1319xDrvPortGpio1  = (uint8_t*) &gMC1319xGPIO1Port; 
uint8_t* pMC1319xDrvPortGpio2  = (uint8_t*) &gMC1319xGPIO2Port;  
uint8_t  mMC1319xDrvMaskGpio1  = gMC1319xGPIO1Mask_c;
uint8_t  mMC1319xDrvMaskGpio2  = gMC1319xGPIO2Mask_c;


/************************************************************************************
* Driver functions for controlling MC1319x GPIO pins
************************************************************************************/
void MC1319xDrv_RxTxEnable(void)            { gMC1319xRxTxPort  |=  gMC1319xRxTxMask_c;}
void MC1319xDrv_RxTxDisable(void)           { gMC1319xRxTxPort  &= ~gMC1319xRxTxMask_c;}


#if (defined(TARGET_TOROWEAP)) 
void MC1319xDrv_RxAntennaSwitchEnable(void) { }
void MC1319xDrv_TxAntennaSwitchEnable(void) { }
#elif defined(TARGET_USER_DEFINED) 
 #if(gUserTransceiverType_d == MC1321x)
 void MC1319xDrv_RxAntennaSwitchEnable(void) {EnableLNA }
 void MC1319xDrv_TxAntennaSwitchEnable(void) {EnablePA }
 #else
 void MC1319xDrv_RxAntennaSwitchEnable(void) 
 {
    gMC1319xAntSwPort &= ~gMC1319xAntSwMask_c; 
	EnableLNA
}
 void MC1319xDrv_TxAntennaSwitchEnable(void) 
 { 
    gMC1319xAntSwPort |=  gMC1319xAntSwMask_c; 
	EnablePA
 }
 #endif
#else
void MC1319xDrv_RxAntennaSwitchEnable(void) { gMC1319xAntSwPort &= ~gMC1319xAntSwMask_c; }
void MC1319xDrv_TxAntennaSwitchEnable(void) { gMC1319xAntSwPort |=  gMC1319xAntSwMask_c; }
#endif //TARGET_TOROWEAP

void MC1319xDrv_RxLnaDisable(void)          
{
    /* Customer specific code. Very fast execution required */
	#if gTarget_UserDefined_d == 1
	    DisableLNA
	#endif /* gTarget_UserDefined_c */	
}
void MC1319xDrv_TxPaDisable(void)           
{
    /* Customer specific code. Very fast execution required */
	#if gTarget_UserDefined_d == 1
	    DisablePA
	#endif /* gTarget_UserDefined_c */	
}



/************************************************************************************
*  SPI initialization for MC1319x driver interface                                  *
************************************************************************************/
void MC1319xDrv_Init(void)
{
  // Init Driver Pins
  MC1319xDrv_AssertReset();    // Keep MC1319x in reset for now
  
  // SPI Bit Clock Rate = BUSCLK / 2
  // SPI port (Port E on GB60) already configured in PortConfig 
  // PTED = 0x04;	//PTED.2=SS = LOW  : 0x00 for autoselect. Deasserts CE
  // PTEPE = 0x00;
  // PTEDD |= 0x34;

  SPIC1 = 0x50; //SPIC1.1 = SSOE   : 0x52 for autoselect
  SPIC2 = 0x00; //SPIC2.4 = MODEEN : 0x10 for autoselect
  SPIBR = 0x00;
  {char dummy=SPIS;}
}

/*******************************************************************
* Make a pulse on hardware reset PIN at ABEL                       *
*******************************************************************/
void MC1319xDrv_Reset(void)
{
  MC1319xDrv_AssertReset();     //To ensure time goes....
  MC1319xDrv_AssertReset();
  MC1319xDrv_AssertReset();
  MC1319xDrv_AssertReset();



  

  MC1319xDrv_DeassertReset();

  
  
}


/*******************************************************************************
* Pulse ATTN pin of the MC1319x in order to bring it out of power save mode    *
*******************************************************************************/
void MC1319xDrv_WakeUp(void)
{
  uint8_t i=10; // ~10us ATTN pulse @ 62.5KHz

  MC1319xDrv_AttEnable();

  while(i) { i--; }

  MC1319xDrv_AttDisable();

    // Wait for ICG to lock onto operational clock frequency.
  while(!ICG_IsFllLocked());
}



/************************************************************************************
* Local macros for SPI functions
************************************************************************************/
#define AssertCE                {gMC1319xSPIPort &= ~gMC1319xSpiSsMask_c;}
#define DeAssertCE              {gMC1319xSPIPort |= gMC1319xSpiSsMask_c;}

#define SPIS_DummyRead          {char dummy=SPIS;}
#define SPID_DummyRead          {char dummy=SPID;}


#define WaitSPITxHoldingEmpty   {while (!(SPIS & 0x20));}  //Check if it is legal to write into TX Double buffer register
#define WaitSPITransferDone     {Wait_8_BitClks(); SPIS_DummyRead;} //__asm nop; while (!(SPIS & 0x80));  //Using RXDone flag to check all transfer done (tx followed by rx)
#define WaitSPITransmitterEmpty {WaitSPITxHoldingEmpty; Wait_6_BitClks(); SPIS_DummyRead; SPID_DummyRead;} 
#define WaitSPIRxFull           {while (!(SPIS & 0x80));}

void Wait_8_BitClks(void);
void Wait_6_BitClks(void);

/************************************************************************************
* Old local SPI macros. Easy to read versions (slow and safe) for read core macros
************************************************************************************/
/*
*** Easy to read versions (slow and safe) for read core macros  ***
#define SPIDummyWrite           __asm STA 0x2D ! {}, {};

#define MC1319xDrv_ReadSpiIntCoreMacro(addr, pb) \
  AssertCE;                             \
                                        \
  SPID = addr;                          \
  WaitSPITransferDone;                  \
  SPID;                                 \
                                        \
  SPIDummyWrite                         \
  WaitSPITransferDone;                  \
  pb[0] = SPID;                         \
                                        \
  SPIDummyWrite                         \
  WaitSPITransferDone;                  \
  pb[1] = SPID;                         \
                                        \
  DeAssertCE;
*/


/*
#define MC1319xDrv_ReadSpiIntCoreMacro(addr, pb) \
                                              \
  AssertCE;                                   \
                                              \
  SPID = addr;                                \
  WaitSPITransferDone;                        \
  SPID=SPID;                                  \
                                              \
  WaitSPITransferDone;                        \
  tmpAccum = SPID;                            \
  SPID = tmpAccum;                            \
  pb[0] = tmpAccum;                           \
                                              \
  WaitSPITransferDone;                        \
  pb[1] = SPID;                               \
  DeAssertCE; 
*/

/************************************************************************************
* Local macros
************************************************************************************/
/*
#define MC1319xDrv_ReadSpiIntCoreMacro()               \
{   asm BSR   Wait_8_BitClks;                       \
    asm LDA   43;                                   \
    asm MOV   45,45;                                \
    asm BSR   Wait_8_BitClks;                       \
    asm LDA   43;                                   \
    asm LDA   45;                                   \
    asm STA   45;                                   \
    asm STA   ,X;                                   \
    asm BSR   Wait_6_BitClks;                       \
    asm LDA   43;                                   \
    asm LDA   45;                                   \
    asm STA   1,X;                                  \
}
*/
/*
#define MC1319xDrv_ReadSpiIntCoreLEMacro()          \
{   asm BSR   Wait_8_BitClks;                       \
    asm LDA   43;                                   \
    asm MOV   45,45;                                \
    asm BSR   Wait_8_BitClks;                       \
    asm LDA   43;                                   \
    asm LDA   45;                                   \
    asm STA   45;                                   \
    asm STA   1,X;                                  \
    asm BSR   Wait_6_BitClks;                       \
    asm LDA   43;                                   \
    asm LDA   45;                                   \
    asm STA   ,X;                                   \
}
*/


/*************************************************************************
*           HardCore HCS08 Delay Helper Functions                        *
*************************************************************************/
void Wait_6_BitClks(void)       // Duration : BRA/BSR/JSR to here: 3/5/6 mcuCycles 
{                               //            2xNOP+RTS          :     8 mcuCycles
  __asm nop;
  __asm nop;
}

void Wait_8_BitClks(void) 
{                               // Duration : BSR/JSR to here: 5/6 mcuCycles
  Wait_6_BitClks();             // Uses BRA (3 mcuCyc) to above function (so it really only takes 5,5 bitclks)
}

/*************************************************************************
*                       SPI Read functions                               *
*************************************************************************/

/************************************************************************
* Read one 16bit, MSB alligned, data-location from Abel register "addr" *
* Unprotected - only to be called with disabled interrupts              *
* Macro used in below functions                                         *
*************************************************************************/
#define MC1319xDrv_ReadSpiIntCoreMacro(addr, pb)                                \
{                                                                               \
  AssertCE;                                                                     \
                                                                                \
  SPID = addr;             /* Write Addr byte */                                \
  WaitSPIRxFull;           /* Wait for Rx data to be sampled from MISO (also ensures TX transfer done) */  \
                                                                                \
  SPID = 0;                /* Dummy write. It will trigger the read transfer */ \
  WaitSPIRxFull;           /* Wait for Rx data to be sampled from MISO (also ensures TX transfer done) */  \
  *((uint8_t*)pb) = SPID;  /* Read the High byte */                             \
                                                                                \
  SPID = 0;                /* Dummy write. It will trigger the read transfer */ \
  WaitSPIRxFull;           /* Wait for Rx data to be sampled from MISO (also ensures TX transfer done) */  \
  *((uint8_t*)pb + 1) = SPID;  /* Read the Low byte */                          \
                                                                                \
  DeAssertCE;                                                                   \
}

/************************************************************************
* Read one 16bit, LSB alligned, data-location from Abel register "addr" *
* Unprotected - only to be called with disabled interrupts              *
* Macro used in below functions                                         *
*************************************************************************/
#define MC1319xDrv_ReadSpiIntCoreLEMacro(addr, pb)                              \
{                                                                               \
  AssertCE;                                                                     \
                                                                                \
  SPID = addr;             /* Write Addr byte */                                \
  WaitSPIRxFull;           /* Wait for Rx data to be sampled from MISO (also ensures TX transfer done) */  \
                                                                                \
  SPID = 0;                /* Dummy write. It will trigger the read transfer */ \
  WaitSPIRxFull;           /* Wait for Rx data to be sampled from MISO (also ensures TX transfer done) */  \
  *((uint8_t*)pb + 1) = SPID;  /* Read the Low byte */                          \
                                                                                \
  SPID = 0;                /* Dummy write. It will trigger the read transfer */ \
  WaitSPIRxFull;           /* Wait for Rx data to be sampled from MISO (also ensures TX transfer done) */  \
  *((uint8_t*)pb) = SPID;  /* Read the High byte */                             \
                                                                                \
  DeAssertCE;                                                                   \
}

/*****************************************************************
* This function reads 1 16bit Abel Register at address "addr"    *
* This read may be called from anywhere. (Async and sync context)*
*****************************************************************/
void MC1319xDrv_ReadSpi(uint8_t addr, uint16_t *pb)
{
  uint8_t   CCR;

  IrqControlLib_BackupIrqStatus(CCR)
  IrqControlLib_DisableAllIrqs();

  MC1319xDrv_ReadSpiIntCoreMacro(addr, pb); // Macro
                                                         
  IrqControlLib_RestoreIrqStatus(CCR)
}

/****************************************************************
* This function reads 1 16bit Abel Register at address "addr"   *
* This read is only to be used from Abel interrupt context      *
****************************************************************/
void MC1319xDrv_ReadSpiInt(uint8_t addr, uint16_t * pb)
{
  IrqControlLib_DisableAllIrqs();

  MC1319xDrv_ReadSpiIntCoreMacro(addr, pb); // Macro
  
  IrqControlLib_EnableAllIrqs();
}

/****************************************************************
* This function reads 1 16bit Abel Register at address "addr"   *
* This read is only to be used from Abel interrupt context      *
* Note: Forced Little Endianess!                                *
****************************************************************/
void MC1319xDrv_ReadSpiIntLE(uint8_t addr, uint16_t *pb)
{
  IrqControlLib_DisableAllIrqs();

  MC1319xDrv_ReadSpiIntCoreLEMacro(addr, pb); // Macro

  IrqControlLib_EnableAllIrqs();
}


/*************************************************************************
*                      SPI Write functions                               *
*************************************************************************/

/********************************************************************
* Write one 16bit data-location into Abel at address "addr"         *
* without monitoring SPSCR. Data is transferred to this routine     *
* over the stack.                                                   *
* Unprotected - only to be called with disabled interrupts          *
* Macro used in below functions                                     *
********************************************************************/
#define MC1319xDrv_WriteSpiIntCoreMacro(addr, pb) {                            \
  AssertCE;                                                           \
  SPID = addr;             /* Write Addr byte */                      \
                                                                      \
  WaitSPITxHoldingEmpty;   /* Wait for room in holding register */    \
  SPID =(uint8_t)(pb>>8);  /* Write MSB data byte */                  \
                                                                      \
  WaitSPITxHoldingEmpty;   /* Wait for room in holding register */    \
  SPID = (uint8_t) pb;     /* Write LSB data byte */                  \
                                                                      \
  WaitSPITransmitterEmpty; /* Wait for transmit queue empty */        \
  DeAssertCE;                                                         \
}

/********************************************************************
* Write one 16bit data-location into Abel at address "addr"         *
* without monitoring SPSCR.                                         *
* This write is only to be used from Abel interrupt context         *
* Protected!                                                        *
********************************************************************/
void MC1319xDrv_WriteSpiInt(uint8_t addr, uint16_t content)
{
  uint8_t CCR;
  IrqControlLib_BackupIrqStatus(CCR)
  IrqControlLib_DisableAllIrqs();
  MC1319xDrv_WriteSpiIntCoreMacro(addr, content); // Macro 
  IrqControlLib_RestoreIrqStatus(CCR)
}

/********************************************************************
* Write one 16bit data-location into Abel at address "addr"         *
* without monitoring SPSCR.                                         *
* This write is only to be used from Abel interrupt context         *
* Not Protected!                                                    *
********************************************************************/
void MC1319xDrv_WriteSpiIntFast(uint8_t addr, uint16_t content)
{
  MC1319xDrv_WriteSpiIntCoreMacro(addr, content); // Macro
}



/********************************************************************
* Write one 16bit data-location into Abel at address "addr"         *
* without monitoring SPSCR. Data is transferred to this routine     *
* over the stack.                                                   *
********************************************************************/
void MC1319xDrv_WriteSpi(uint8_t addr, uint16_t content){
  uint8_t CCR;
  IrqControlLib_BackupIrqStatus(CCR)
  IrqControlLib_DisableAllIrqs();
  MC1319xDrv_WriteSpiIntFast(addr, content); // Normal Function call
  IrqControlLib_RestoreIrqStatus(CCR)
}

#pragma CODE_SEG DEFAULT
