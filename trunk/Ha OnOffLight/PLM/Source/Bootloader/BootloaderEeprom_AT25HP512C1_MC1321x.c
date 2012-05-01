/******************************************************************************
* This is the Source file for the MC1321x dev boards EEPROM driver
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
                Check list file or map file 
*/

#include "Embeddedtypes.h"
#include "IoConfig.h"
#include "BootloaderEeprom.h"

/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/

#define EEPROM_BOOT_SPI_PORT         PTCD            
#define EEPROM_BOOT_SPI_CS           (1<<2)/*(1<<7)*/
#define EEPROM_BOOT_SPI_MISO         (1<<6)
#define EEPROM_BOOT_SPI_MOSI         (1<<3)  
#define EEPROM_BOOT_SPI_CLK          (1<<5)

#define  EEPROM_BOOT_SETUP_PORT_C() PTCPE |= EEPROM_BOOT_SPI_CS;\
                                 PTCDD |= EEPROM_BOOT_SPI_CLK | EEPROM_BOOT_SPI_MOSI | EEPROM_BOOT_SPI_CS;\
                                 PTCDD &= ~EEPROM_BOOT_SPI_MISO

#define EEPROM_BOOT_SETUP_MISO()		 
#define EEPROM_BOOT_SETUP_MOSI()    

#define SET_EEPROM_BOOT_SPI_CS() EEPROM_BOOT_SPI_PORT |=  EEPROM_BOOT_SPI_CS
#define CLR_EEPROM_BOOT_SPI_CS() EEPROM_BOOT_SPI_PORT &=  ~EEPROM_BOOT_SPI_CS

#define SET_EEPROM_BOOT_SPI_MOSI() EEPROM_BOOT_SPI_PORT |=  (EEPROM_BOOT_SPI_MOSI)
#define CLR_EEPROM_BOOT_SPI_MOSI() EEPROM_BOOT_SPI_PORT &=  ~(EEPROM_BOOT_SPI_MOSI)


#define EEPROM_BOOT_SPI_MISO_STATUS()       ((EEPROM_BOOT_SPI_PORT & EEPROM_BOOT_SPI_MISO) == EEPROM_BOOT_SPI_MISO)

  
#define SET_EEPROM_BOOT_SPI_CLK() EEPROM_BOOT_SPI_PORT |=  EEPROM_BOOT_SPI_CLK
#define CLR_EEPROM_BOOT_SPI_CLK() EEPROM_BOOT_SPI_PORT &=  ~EEPROM_BOOT_SPI_CLK

/// EEPROM COMMANDS:
#define  EEPROM_BOOT_WREN  0x06
#define  EEPROM_BOOT_WRDI  0x04
#define  EEPROM_BOOT_RDSR  0x05
#define  EEPROM_BOOT_WRSR  0x01
#define  EEPROM_BOOT_READ  0x03
#define  EEPROM_BOOT_WRITE 0x02

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
#ifdef gUseBootloader_d
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
#endif

static void     EEPROM_BOOT_WriteByteOverSPI(uint8_t out);
static uint8_t  EEPROM_BOOT_ReadByteOverSPI(void);
static uint8_t  EEPROM_BOOT_ReadStatusReq(void);
static void     EEPROM_BOOT_SetWREN(void);
static void     EEPROM_BOOT_WriteStatusReq(uint8_t Status); 
static boot_ee_err_t EEPROM_BOOT_WaitForReady(void);
static void     EEPROM_BOOT_Delay(void);

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

/* If the bootloader is enabled, the EEPROM_BOOT_Init and EEPROM_BOOT_ReadData functions
should be placed in the Bootloader's segment as they will be used by the 
bootloader */
#ifdef gUseBootloader_d
  #pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
#endif

/*****************************************************************************
*  EEPROM_BOOT_Init
*
*  Initializes the EEPROM_BOOT_ peripheral
*
*****************************************************************************/
boot_ee_err_t EEPROM_BOOT_Init(void) 
{
  boot_ee_err_t retval;
  EEPROM_BOOT_SETUP_PORT_C();
  CLR_EEPROM_BOOT_SPI_CLK();
  
  retval = EEPROM_BOOT_WaitForReady();
  if(boot_ee_ok == retval)
  {
    EEPROM_BOOT_SetWREN();
    EEPROM_BOOT_WriteStatusReq(0x00); // Set no write protection!.
  }
  return retval;
}
/*****************************************************************************
*  EEPROM_BOOT_ReadData
*
*  Reads a data buffer from EEPROM_BOOT_, from a given address
*
*****************************************************************************/
boot_ee_err_t EEPROM_BOOT_ReadData(uint16_t NoOfBytes, uint32_t Addr, uint8_t *inbuf)
{
  uint16_t i;
  boot_ee_err_t retval;
 
  retval = EEPROM_BOOT_WaitForReady(); 

  if(boot_ee_ok == retval)
  {
    CLR_EEPROM_BOOT_SPI_CLK();
    CLR_EEPROM_BOOT_SPI_CS();
    EEPROM_BOOT_Delay();
    EEPROM_BOOT_WriteByteOverSPI(EEPROM_BOOT_READ);
    EEPROM_BOOT_Delay();
    EEPROM_BOOT_WriteByteOverSPI((uint8_t) ((Addr >> 8) & 0x00FF));
    EEPROM_BOOT_Delay();
    EEPROM_BOOT_WriteByteOverSPI((uint8_t) (Addr & 0x00FF));


    for (i = 0; i < NoOfBytes; i++) {
      EEPROM_BOOT_Delay();  
      inbuf[i] = EEPROM_BOOT_ReadByteOverSPI();
    }
    SET_EEPROM_BOOT_SPI_CS();
  }
    
  return retval;
}
/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************
*  EEPROM_BOOT_Delay
*
*  Initializes the EEPROM_BOOT_ peripheral
*
*****************************************************************************/
static void EEPROM_BOOT_Delay(void)
{ 
  volatile uint8_t i=8;
  while(--i);
}

/*****************************************************************************
*  EEPROM_BOOT_WriteByteOverSPI
*
*
*****************************************************************************/

static void EEPROM_BOOT_WriteByteOverSPI(uint8_t out)
{
  unsigned char i;           // Bit counter 
  unsigned char in = 0;
 
  EEPROM_BOOT_SETUP_MOSI(); // setup for write.
  for (i = 0; i < 8; i++) 
  {
    EEPROM_BOOT_Delay();
    if ((out & 0x80) == 0x80)
      SET_EEPROM_BOOT_SPI_MOSI();
    else
      CLR_EEPROM_BOOT_SPI_MOSI();
   
    SET_EEPROM_BOOT_SPI_CLK();
    EEPROM_BOOT_Delay();
    CLR_EEPROM_BOOT_SPI_CLK();
    out = out << 1; 
   }
   CLR_EEPROM_BOOT_SPI_CLK();  
}

/*****************************************************************************
*  EEPROM_BOOT_ReadByteOverSPI
*
*
*****************************************************************************/
static uint8_t EEPROM_BOOT_ReadByteOverSPI(void)
{
  unsigned char i;           // Bit counter 
  unsigned char in = 0;
  EEPROM_BOOT_SETUP_MISO();

  for (i = 0; i < 8; i++) 
  {
    EEPROM_BOOT_Delay(); 
    SET_EEPROM_BOOT_SPI_CLK();
    EEPROM_BOOT_Delay();
    in = in << 1;
    in |= EEPROM_BOOT_SPI_MISO_STATUS();
    CLR_EEPROM_BOOT_SPI_CLK(); 
  }
  return in; 
}

/*****************************************************************************
*  EEPROM_BOOT_ReadStatusReq
*
*
*****************************************************************************/

static uint8_t EEPROM_BOOT_ReadStatusReq(void)
{
  uint8_t Status;

  CLR_EEPROM_BOOT_SPI_CLK();
  CLR_EEPROM_BOOT_SPI_CS();
  EEPROM_BOOT_Delay();
  EEPROM_BOOT_WriteByteOverSPI(EEPROM_BOOT_RDSR);
  EEPROM_BOOT_Delay();  
  Status = EEPROM_BOOT_ReadByteOverSPI();
  SET_EEPROM_BOOT_SPI_CS();
  return Status;  
}

/*****************************************************************************
*  EEPROM_BOOT_SetWREN
*
*
*****************************************************************************/

static void EEPROM_BOOT_SetWREN(void)
{
  CLR_EEPROM_BOOT_SPI_CS();
  EEPROM_BOOT_Delay();
  EEPROM_BOOT_WriteByteOverSPI(EEPROM_BOOT_WREN);
  SET_EEPROM_BOOT_SPI_CS();
}

/*****************************************************************************
*  EEPROM_BOOT_SetWREN
*
*
*****************************************************************************/

static void EEPROM_BOOT_WriteStatusReq(uint8_t Status)
{
  CLR_EEPROM_BOOT_SPI_CS();
  EEPROM_BOOT_Delay();
  EEPROM_BOOT_WriteByteOverSPI(EEPROM_BOOT_WRSR);
  EEPROM_BOOT_Delay();
  EEPROM_BOOT_WriteByteOverSPI(Status);
  SET_EEPROM_BOOT_SPI_CS();
  CLR_EEPROM_BOOT_SPI_CLK();
}

/*****************************************************************************
*  EEPROM_BOOT_WaitForReady
*
*
*****************************************************************************/

static boot_ee_err_t EEPROM_BOOT_WaitForReady(void)
{
  /*
       50 ms @ EEPROM_BOOT_Delay_STEP = 0,1,2
      300 ms @ EEPROM_BOOT_Delay_STEP = 3
      550 ms @ EEPROM_BOOT_Delay_STEP = 4
  */
  volatile uint16_t wait = 0x400; // near 50 ms @ 50 us/cycle

  while ( ((EEPROM_BOOT_ReadStatusReq() & 0x01) == 0x01) && (wait !=0) )
  {
    wait--;
  }

  if(wait != 0)
  {
    return boot_ee_ok;
  }
  return boot_ee_error;
}

/*****************************************************************************/


#pragma CODE_SEG DEFAULT

