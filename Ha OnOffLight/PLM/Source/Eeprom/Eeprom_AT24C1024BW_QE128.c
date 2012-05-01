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

#include "Embeddedtypes.h"
#include "IoConfigS08QE128.h"
#include "eeprom.h"


/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/

#define     gEepHighAddress              0xFFFF

#define     gEepIICAddressWriteLow_c     0xA0
#define     gEepIICAddressReadLow_c      0xA1
#define     gEepIICAddressWriteHigh_c    0xA2
#define     gEepIICAddressReadHigh_c     0xA3

#define     gShortDelay_15us_c           0xFF /*1/16Mhz * 255*/
#define     gLongDelay_260us_c           0xFFF/*1/16Mhz * 1023*/

#define     gSectorMask_c                0x000000FF

/* Write protect is done with through WP pin of EEPROM controled by PTC5 */
#define eepWriteProtectEnable()  PTCD  |=(uint8_t)(1<<5);  PTCDD |=(uint8_t)(1<<5);
#define eepWriteProtectDisable() PTCD  &=~(uint8_t)(1<<5); PTCDD |=(uint8_t)(1<<5);

/* System clock gating control 1 register */
#define     eppSCGC1   SCGC1


/* IIC registers used for eeprom */
#define     eppIICA   IIC1A       
#define     eppIICF   IIC1F      
#define     eppIICC1  IIC1C1    
#define     eppIICS   IIC1S      
#define     eppIICD   IIC1D    
#define     eppIICC2  IIC1C2

/* Bus_speed 16Mhz -- check 400 */
#define eepIICClockSpeed100kHz     (uint8_t)((0<<6)| 0x1D);

#define eepIrqEnableIIC            (uint8_t)(1<<6)
#define eepMasterModeIIC           (uint8_t)(1<<5)


/* START/STOP signal */
#define eepStartSignal()           eppIICC1 |= eepMasterModeIIC;
#define eepStopSignal()            eppIICC1 &= ~eepMasterModeIIC;


/* Enable - disable IIC */
#define eepEnableFlagIIC           (uint8_t)(1<<7)

#define eepEnableIIC()             eppIICC1 |=  eepEnableFlagIIC
#define eepDisableIIC()            eppIICC1 &= ~eepEnableFlagIIC


/* Set TX RX mode */
#define eepTxRxSelect              (uint8_t)(1<<4)

#define eepTXsetIIC()              eppIICC1 |=  eepTxRxSelect
#define eepRXsetIIC()              eppIICC1 &= ~eepTxRxSelect


/* Set transmit ACK */
#define eepAckSelect               (uint8_t)(1<<3)
                                   
#define eepAckDisableIIC()         eppIICC1 |=  eepAckSelect
#define eepAckEnableIIC()          eppIICC1 &= ~eepAckSelect


/* Repeat Start */                             
#define eepRepStart                (uint8_t)(1<<2) 

#define eepRepeatStartIIC()        eppIICC1 |=  eepRepStart


/* Status Flags */
#define eepTransferCompleteIIC     (uint8_t)(1<<7)
#define eepBusBusyIIC              (uint8_t)(1<<5)
#define eepIICIF                   (uint8_t)(1<<1)
#define eepArbLost                 (uint8_t)(1<<4)
#define eepAckReceiveIIC           (uint8_t)(1<<0)

/* Status checks */
#define eepTransfCompleteCheck()   (eppIICS & eepTransferCompleteIIC)
#define eepBusyCheck()             (eppIICS & eepBusBusyIIC)
#define eepAckCheck()              (eppIICS & eepAckReceiveIIC)
#define eepClearIICIF()            eppIICS |= eepIICIF
#define eepCheckIICIF()            (eppIICS & eepIICIF)
#define eepClearArbLost()          eppIICS |= eepArbLost
#define eepClearTCF()              eppIICS |= eepTransferCompleteIIC  


/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
static void     IIC_Init(void);
static void     EepAddressWrite(uint16_t eepAddress);
static void     EepIICByteWrite(uint8_t byteWrite);
static void     EepIICStartSequence(uint32_t Addr);
static uint8_t  EepIICByteRead(void);
static void     EepIICReadStartSequence(uint32_t Addr);
static void     EepDelay(volatile uint16_t delay);
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

/*****************************************************************************
*  EEPROM_Init
*
*  Initializes the EEPROM peripheral
*
*****************************************************************************/

ee_err_t EEPROM_Init(void) 
{
  /* Eeprom Init */ 
  IIC_Init();
  
  return ee_ok;
}


/*****************************************************************************
*  EEPROM_ReadData
*
*  Reads a data buffer from EEPROM, from a given address
*
*****************************************************************************/

ee_err_t EEPROM_ReadData(uint16_t NoOfBytes, uint32_t Addr, uint8_t *inbuf)
{

  /* Start read sequence */
  EepIICReadStartSequence(Addr);
  
  /* Enable ACK for received bytes in case that is more than one to read */  

  if(NoOfBytes > 1)
  {
    eepAckEnableIIC();
  }
  /* Dummy byte read to start transfer */ 
  (void)EepIICByteRead();
  
  while(NoOfBytes)
  {
    *inbuf++=EepIICByteRead();
    
    /* Eeprom memory adress increase in order to very the sector end */
    Addr++;

    /* If it is the last byte disable Ack */
    /* Protection for pass another sector */
    if((gSectorMask_c == (Addr & gSectorMask_c))||(2 == NoOfBytes))
    {
      eepAckDisableIIC();
    }  
    
    /* Protection for pass to the next eeprm memory sector */
    if((0 == (Addr & gSectorMask_c)) && (NoOfBytes>2))
    {
      
      /* Wait for interrupt flag */
      while(!eepCheckIICIF());
      
      /* Clear Interrupt flag */
      eepClearIICIF();
      
      /* Send Stop signal */
      eepStopSignal();
      
      /* Check if bus is Busy */
      while(eepBusyCheck());
      
      /* Set in EEPROM adress of the new sector*/
      EepIICReadStartSequence(Addr);
      
      /* If it is the last byte - disable Ack */
      if(NoOfBytes > 1)
      {
        eepAckEnableIIC();
      }
      /* Dummy byte read to start transfer */ 
      (void)EepIICByteRead();
    }
     
    NoOfBytes--;  
  }
  
  /* Wait for the interrupt flag */
  while(!eepCheckIICIF());
  
  /* Clear Interrupt flag */
  eepClearIICIF();
  
  /* Send stop signal*/
  eepStopSignal();
  
  /* Verify that the bus is not busy */
  while(eepBusyCheck());
  
  return ee_ok;
}

/*****************************************************************************
*  EEPROM_WriteData
*
*  Writes a data buffer into EEPROM, at a given address
*
*****************************************************************************/

ee_err_t EEPROM_WriteData(uint8_t NoOfBytes, uint32_t Addr, uint8_t *Outbuf)
{
   /* Disable eeprom write protection -> put PTC5 in 0 */
  eepWriteProtectDisable();
  
  /* Start write sequence */
  EepIICStartSequence(Addr);
  
  while(NoOfBytes)
  {
    /* Write byte from the buffer*/
    EepIICByteWrite(*Outbuf++);
    
    /* Increase eeprom memory address for sector limit protection */
    Addr++;
    
    /* Protection for pass another sector */
    if((0 == (Addr & gSectorMask_c))&&(1<NoOfBytes))
    {
      /* Clear interrupt flag */
      eepClearIICIF();
      
      /* Send stop signal*/
      eepStopSignal();

      /* Check if bus is Busy */
      while(eepBusyCheck());
      
      /* Reinit writte process */ 
      EepIICStartSequence(Addr);
    }
    
    NoOfBytes--;
  }
  
  /* Clear interrupt flag */
  eepClearIICIF();
  
  /* Send stop signal*/ 
  eepStopSignal();
  
  /* Bus busy check */
  while(eepBusyCheck());
  
  /* Enable eeprom write protection -> put PTC5 in 1 */ 
  eepWriteProtectEnable();
  
  return ee_ok;
}

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************
*  IIC_Init
*
*  Init IIC 
*
*****************************************************************************/
static void IIC_Init(void)
{
  
  /* Enable write protection to prevent wrong write to eeprom*/ 
  eepWriteProtectEnable();
  
  /* Enable cloack gating */
  eppSCGC1|=(uint8_t)(1<<2);

  /* Set baud rate */
  eppIICF = eepIICClockSpeed100kHz;
  
  /* Enable IIC */
  eppIICC1 |= eepEnableFlagIIC;
  
  /* Bus not busy check */
  while(eepBusyCheck());
      
}

/*****************************************************************************
*  EepAddressWrite
*
*  Send on  IIC Eeprom adress
*
*****************************************************************************/

static void EepAddressWrite(uint16_t eepAddress)
{
  /* Send high address where to writte */
  EepIICByteWrite((uint8_t)(eepAddress >> 8));
  /* Send low address where to writte */
  EepIICByteWrite((uint8_t)(eepAddress >> 0));
}

/*****************************************************************************
*  EepIICByteWrite
*
*  Write a byte  on  IIC 
*
*****************************************************************************/
static void EepIICByteWrite(uint8_t byteWrite)
{
  /* Clear Interrupt flag */
  eepClearIICIF();
  
  /* Write byte - this generate start transfer */
  eppIICD = byteWrite;

  /* Pool transfer complete and ack received*/
  while  (!eepCheckIICIF()); 

}

/*****************************************************************************
*  EepIICByteRead
*
*  Read a byte  on  IIC 
*
*****************************************************************************/

static uint8_t EepIICByteRead(void)
{
  
  /* Pool transfer complete and ack received*/
  while(!eepCheckIICIF());
  
  /* Clear Interrupt flag */
  eepClearIICIF();
 
  /* Read value of IIC data register - will generate a new read */
  return eppIICD; 
}

/*****************************************************************************
*  EepIICStartSequence
*
*  Send start sequence on IIC
*
*****************************************************************************/
static void EepIICStartSequence(uint32_t Addr)
{
  uint8_t  eppAdress;
  
  /* Compute eeprom adress */
  /* Eeprom IIC adress contain MSB 17 bit of eeprom memory address */
  //if(Addr > gEepHighAddress) 
  if((uint16_t)(Addr>>16)>(uint16_t)0)
  {
    eppAdress = gEepIICAddressWriteHigh_c;
  }
  else
  {
    eppAdress = gEepIICAddressWriteLow_c;
  }
  
  /* Put IIC in Tx mode */
  eepTXsetIIC();
    
  /* Implement also aknowledge pooling for 2 consective eeprom acces in short time*/
  for(;;)
  {
        
    /* Start signal*/  
    eepStartSignal();
    
    /* Bus busy check */ 
    while(!eepBusyCheck())
  
    
    /* Writte byte and start transfer */
    eppIICD = eppAdress;

    /* Verify complete transfer flag */
    while  (!eepTransfCompleteCheck());
    
    /* Wait around 15 us for ack signal to came */ 
    EepDelay(gShortDelay_15us_c);
    
    /* In case the Ack was received - EEPROM has finish last process - maybe a write*/
    if(!eepAckCheck()) break;
    
    /* send stop signal on the bus */
    eepStopSignal();
    
    /* Check if bus is Busy */
    while(eepBusyCheck());
    
    /* Between two ack pooling test wait around 260us */
    EepDelay(gLongDelay_260us_c);
    
  }
  
  /* Write Eprom start address*/
  EepAddressWrite((uint16_t)Addr);

}

/*****************************************************************************
*  EepIICReadStartSequence
*
*  Send start sequence for read on IIC
*
*****************************************************************************/

static void EepIICReadStartSequence(uint32_t Addr)
{
  
  uint8_t  eppAdress;
    
  /* Start sequence in order to put in EEPROM memory address to be read */ 
  EepIICStartSequence(Addr);

  /* For read send a repeat start signal  */
  eepRepeatStartIIC();

  /* Short delay - maybe can be eliminated*/
  EepDelay(gShortDelay_15us_c);
  
  /* Eeprom IIC adress contain MSB 17 bit of eeprom memory address */ 
  //if(Addr > gEepHighAddress)
  if((uint16_t)(Addr>>16)>(uint16_t)0)
  {
    eppAdress = gEepIICAddressReadHigh_c;
  }
  else
  {
    eppAdress = gEepIICAddressReadLow_c;  
  }
  
  /* Write IIC eeprom adress */
  EepIICByteWrite(eppAdress);
  
  /* Put IIC in RX mode */
  eepRXsetIIC();
  
}

/*****************************************************************************
*  EepDelay
*
*  Creat a delay 
*  used for example to permit register update
*
*****************************************************************************/
static void EepDelay(volatile uint16_t delay)
{
  while(delay--); 
}


