/******************************************************************************
* This is the Source file for the AT24C1024BW dev boards EEPROM IIC driver
*
*														 
* (c) Copyright 2010, FreeScale Semiconductor inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from FreeScale.
*
******************************************************************************/
/*      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
      Check compiler in order not to use function from ANSI*.LIB  
                Check disasamble file or map file 
*/
#include "Embeddedtypes.h"
#include "IoConfigS08QE128.h"
#include "BootloaderEeprom.h"
#include "Bootloader.h"


/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/

#define     gEepHighAddress              0xFFFF

#define     gEEPROM_BOOT_AddressWriteLow_c     0xA0
#define     gEEPROM_BOOT_AddressReadLow_c      0xA1
#define     gEEPROM_BOOT_AddressWriteHigh_c    0xA2
#define     gEEPROM_BOOT_AddressReadHigh_c     0xA3

#define     gShortDelay_15us_c           0xFF /*1/16Mhz * 255*/
#define     gLongDelay_260us_c           0xFFF/*1/16Mhz * 1023*/

#define     gSectorMask_c                0x000000FF

/* Write protect is done with through WP pin of EEPROM controled by PTC5 */
#define EEPROM_BOOT_WriteProtectEnable()  PTCD  |=(uint8_t)(1<<5);  PTCDD |=(uint8_t)(1<<5);
#define EEPROM_BOOT_WriteProtectDisable() PTCD  &=~(uint8_t)(1<<5); PTCDD |=(uint8_t)(1<<5);

/* System clock gating control 1 register */
#define     eppSCGC1   SCGC1


/* IIC registers used for eeprom */
#define     eppIICA   IIC1A       
#define     eppIICF   IIC1F      
#define     eppIICC1  IIC1C1    
#define     eppIICS   IIC1S      
#define     eppIICD   IIC1D    
#define     eppIICC2  IIC1C2

/* Bus_speed 16Mhz */
#define eepIICClockSpeed100kHz              (uint8_t)((0<<6)| 0x1D);

#define eepIrqEnableIIC                     (uint8_t)(1<<6)
#define eepMasterModeIIC                    (uint8_t)(1<<5)


/* START/STOP signal */
#define EEPROM_BOOT_StartSignal()           eppIICC1 |= eepMasterModeIIC;
#define EEPROM_BOOT_StopSignal()            eppIICC1 &= ~eepMasterModeIIC;


/* Enable - disable IIC */
#define eepEnableFlagIIC                    (uint8_t)(1<<7)

#define EEPROM_BOOT_EnableIIC()             eppIICC1 |=  eepEnableFlagIIC
#define EEPROM_BOOT_DisableIIC()            eppIICC1 &= ~eepEnableFlagIIC


/* Set TX RX mode */
#define eepTxRxSelect                       (uint8_t)(1<<4)

#define EEPROM_BOOT_TXsetIIC()              eppIICC1 |=  eepTxRxSelect
#define EEPROM_BOOT_RXsetIIC()              eppIICC1 &= ~eepTxRxSelect


/* Set transmit ACK */
#define eepAckSelect                        (uint8_t)(1<<3)
                                   
#define EEPROM_BOOT_AckDisableIIC()         eppIICC1 |=  eepAckSelect
#define EEPROM_BOOT_AckEnableIIC()          eppIICC1 &= ~eepAckSelect


/* Repeat Start */                             
#define eepRepStart                         (uint8_t)(1<<2) 

#define EEPROM_BOOT_RepeatStartIIC()        eppIICC1 |=  eepRepStart


/* Status Flags */
#define eepTransferCompleteIIC              (uint8_t)(1<<7)
#define eepBusBusyIIC                       (uint8_t)(1<<5)
#define eepIICIF                            (uint8_t)(1<<1)
#define eepArbLost                          (uint8_t)(1<<4)
#define eepAckReceiveIIC                    (uint8_t)(1<<0)

/* Status checks */
#define EEPROM_BOOT_TransfCompleteCheck()   (eppIICS & eepTransferCompleteIIC)
#define EEPROM_BOOT_BusyCheck()             (eppIICS & eepBusBusyIIC)
#define EEPROM_BOOT_AckCheck()              (eppIICS & eepAckReceiveIIC)
#define EEPROM_BOOT_ClearIICIF()            eppIICS |= eepIICIF
#define EEPROM_BOOT_CheckIICIF()            (eppIICS & eepIICIF)
#define EEPROM_BOOT_ClearArbLost()          eppIICS |= eepArbLost
#define EEPROM_BOOT_ClearTCF()              eppIICS |= eepTransferCompleteIIC  


/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
#ifdef gUseBootloader_d
  #pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
#endif
static void     EEPROM_BOOT_AddressWrite(uint16_t eepAddress);
static void     EEPROM_BOOT_ByteWrite(uint8_t byteWrite);
static void     EEPROM_BOOT_StartSequence(uint32_t Addr);
static uint8_t  EEPROM_BOOT_ByteRead(void);
static void     EEPROM_BOOT_ReadStartSequence(uint32_t Addr);
static void     EEPROM_BOOT_Delay(volatile uint16_t delay);
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
/* Code segment used in case the component is used for bootloader */ 
#ifdef gUseBootloader_d
  #pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
#endif

/*****************************************************************************
*  EEPROM_Init
*
*  Initializes the EEPROM peripheral
*
*****************************************************************************/

boot_ee_err_t EEPROM_BOOT_Init(void) 
{ 
  
  /* Enable cloack gating */
  eppSCGC1|=(uint8_t)(1<<2);

  /* Set baud rate */
  eppIICF = eepIICClockSpeed100kHz;
  
  /* Enable IIC */
  eppIICC1 |= eepEnableFlagIIC;
  
  /* Bus not busy check */
  while(EEPROM_BOOT_BusyCheck());
  
  return boot_ee_ok;
}


/*****************************************************************************
*  EEPROM_ReadData
*
*  Reads a data buffer from EEPROM, from a given address
*
*****************************************************************************/

boot_ee_err_t EEPROM_BOOT_ReadData(uint16_t NoOfBytes, uint32_t Addr, uint8_t *inbuf)
{

  /* Start read sequence */
  EEPROM_BOOT_ReadStartSequence(Addr);
  
  /* Enable ACK for received bytes in case that is more than one to read */  

  if(NoOfBytes > 1)
  {
    EEPROM_BOOT_AckEnableIIC();
  }
  /* Dummy byte read to start transfer */ 
  (void)EEPROM_BOOT_ByteRead();
  
  while(NoOfBytes)
  {
    *inbuf++=EEPROM_BOOT_ByteRead();
    
    /* Eeprom memory adress increase in order to very the sector end */
    /* Addr++; */
    Boot_Add8BitValTo32BitVal(1,(uint16_t *)&Addr);
    
    
    /* If it is the last byte disable Ack */
    /* Protection for pass another sector */
    if((gSectorMask_c == (Addr & gSectorMask_c))||(2 == NoOfBytes))
    {
      EEPROM_BOOT_AckDisableIIC();
    }  
    
    /* Protection for pass to the next eeprm memory sector */
    if((0 == (Addr & gSectorMask_c)) && (NoOfBytes>2))
    {
      
      /* Wait for interrupt flag */
      while(!EEPROM_BOOT_CheckIICIF());
      
      /* Clear Interrupt flag */
      EEPROM_BOOT_ClearIICIF();
      
      /* Send Stop signal */
      EEPROM_BOOT_StopSignal();
      
      /* Check if bus is Busy */
      while(EEPROM_BOOT_BusyCheck());
      
      /* Set in EEPROM adress of the new sector*/
      EEPROM_BOOT_ReadStartSequence(Addr);
      
      /* If it is the last byte - disable Ack */
      if(NoOfBytes > 1)
      {
        EEPROM_BOOT_AckEnableIIC();
      }
      /* Dummy byte read to start transfer */ 
      (void)EEPROM_BOOT_ByteRead();
    }
     
    NoOfBytes--;  
  }
  
  /* Wait for the interrupt flag */
  while(!EEPROM_BOOT_CheckIICIF());
  
  /* Clear Interrupt flag */
  EEPROM_BOOT_ClearIICIF();
  
  /* Send stop signal*/
  EEPROM_BOOT_StopSignal();
  
  /* Verify that the bus is not busy */
  while(EEPROM_BOOT_BusyCheck());
  
  return boot_ee_ok;
}

#ifdef gUseBootloader_d
  #pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
#endif

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************
*  EepAddressWrite
*
*  Send on  IIC Eeprom adress
*
*****************************************************************************/

static void EEPROM_BOOT_AddressWrite(uint16_t eepAddress)
{
  /* Send high address where to writte */
  EEPROM_BOOT_ByteWrite((uint8_t)(eepAddress >> 8));
  /* Send low address where to writte */
  EEPROM_BOOT_ByteWrite((uint8_t)(eepAddress >> 0));
}

/*****************************************************************************
*  EEPROM_BOOT_ByteWrite
*
*  Write a byte  on  IIC 
*
*****************************************************************************/
static void EEPROM_BOOT_ByteWrite(uint8_t byteWrite)
{
  /* Clear Interrupt flag */
  EEPROM_BOOT_ClearIICIF();
  
  /* Write byte - this generate start transfer */
  eppIICD = byteWrite;

  /* Pool transfer complete and ack received*/
  while  (!EEPROM_BOOT_CheckIICIF()); 

}

/*****************************************************************************
*  EEPROM_BOOT_ByteRead
*
*  Read a byte  on  IIC 
*
*****************************************************************************/

static uint8_t EEPROM_BOOT_ByteRead(void)
{
  
  /* Pool transfer complete and ack received*/
  while(!EEPROM_BOOT_CheckIICIF());
  
  /* Clear Interrupt flag */
  EEPROM_BOOT_ClearIICIF();
 
  /* Read value of IIC data register - will generate a new read */
  return eppIICD; 
}

/*****************************************************************************
*  EEPROM_BOOT_StartSequence
*
*  Send start sequence on IIC
*
*****************************************************************************/
static void EEPROM_BOOT_StartSequence(uint32_t Addr)
{
  uint8_t  eppAdress;
  
  /* Compute eeprom adress */
  /* Eeprom IIC adress contain MSB 17 bit of eeprom memory address */
  /* if(Addr > gEepHighAddress) */ 
  if((uint16_t)(Addr>>16)>(uint16_t)0)
  {
    eppAdress = gEEPROM_BOOT_AddressWriteHigh_c;
  }
  else
  {
    eppAdress = gEEPROM_BOOT_AddressWriteLow_c;
  }
  
  /* Put IIC in Tx mode */
  EEPROM_BOOT_TXsetIIC();
    
  /* Implement also aknowledge pooling for 2 consective eeprom acces in short time*/
  for(;;)
  {
        
    /* Start signal*/  
    EEPROM_BOOT_StartSignal();
    
    /* Bus busy check */ 
    while(!EEPROM_BOOT_BusyCheck())
  
    
    /* Writte byte and start transfer */
    eppIICD = eppAdress;

    /* Verify complete transfer flag */
    while  (!EEPROM_BOOT_TransfCompleteCheck());
    
    /* Wait around 15 us for ack signal to came */ 
    EEPROM_BOOT_Delay(gShortDelay_15us_c);
    
    /* In case the Ack was received - EEPROM has finish last process - maybe a write*/
    if(!EEPROM_BOOT_AckCheck()) break;
    
    /* send stop signal on the bus */
    EEPROM_BOOT_StopSignal();
    
    /* Check if bus is Busy */
    while(EEPROM_BOOT_BusyCheck());
    
    /* Between two ack pooling test wait around 260us */
    EEPROM_BOOT_Delay(gLongDelay_260us_c);
    
  }
  
  /* Write Eprom start address */
  EEPROM_BOOT_AddressWrite((uint16_t)Addr);

}

/*****************************************************************************
*  EEPROM_BOOT_ReadStartSequence
*
*  Send start sequence for read on IIC
*
*****************************************************************************/

static void EEPROM_BOOT_ReadStartSequence(uint32_t Addr)
{
  
  uint8_t  eppAdress;
    
  /* Start sequence in order to put in EEPROM memory address to be read */ 
  EEPROM_BOOT_StartSequence(Addr);

  /* For read send a repeat start signal  */
  EEPROM_BOOT_RepeatStartIIC();

  /* Short delay - maybe can be eliminated*/
  EEPROM_BOOT_Delay(gShortDelay_15us_c);
  
  /* Eeprom IIC adress contain MSB 17 bit of eeprom memory address */ 
  /* if(Addr > gEepHighAddress) */
  if((uint16_t)(Addr>>16)>(uint16_t)0)
  {
    eppAdress = gEEPROM_BOOT_AddressReadHigh_c;
  }
  
  else
  {
    eppAdress = gEEPROM_BOOT_AddressReadLow_c;  
  }
  
  /* Write IIC eeprom adress */
  EEPROM_BOOT_ByteWrite(eppAdress);
  
  /* Put IIC in RX mode */
  EEPROM_BOOT_RXsetIIC();
  
}

/*****************************************************************************
*  EepDelay
*
*  Creat a delay 
*  used for example to permit register update
*
*****************************************************************************/
static void EEPROM_BOOT_Delay(volatile uint16_t delay)
{
  while(delay--); 
}

/*****************************************************************************/
#pragma CODE_SEG DEFAULT

