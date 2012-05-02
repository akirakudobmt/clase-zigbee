/******************************************************************************
* This is the Source file for the AT25HP512C1 on MC1321x dev boards EEPROM driver
*														 
* (c) Copyright 2010, FreeScale Semiconductor inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from FreeScale.
******************************************************************************/

  /*      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  /* The write in this eeprom must be aligned to 128     */




#include "Embeddedtypes.h"
#include "IoConfig.h"
#include "Eeprom.h"

/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/

//#define EEPROM_3_PINS
#define EEPROM_4_PINS


#define EEPROM_PAGE_WRITE 128

#ifdef EEPROM_3_PINS
  #define EEPROM_SPI_PORT         PTCD            
  #define EEPROM_SPI_CS           (1<<2)/*(1<<7)*/
  #define EEPROM_SPI_MISO         (1<<6)
  #define EEPROM_SPI_MOSI         (1<<3)/*(1<<6)*/
  #define EEPROM_SPI_CLK          (1<<5)

  #define  EEPROM_SETUP_PORT_C() PTCPE |= EEPROM_SPI_CS;\
                                 PTCDD |= EEPROM_SPI_CLK | EEPROM_SPI_MOSI | EEPROM_SPI_CS;\
                                 PTCDD &= ~EEPROM_SPI_MISO

  #define EEPROM_SETUP_MISO()		 PTCDD &= ~EEPROM_SPI_MISO
  #define EEPROM_SETUP_MOSI()    PTCDD |=  EEPROM_SPI_MOSI

#else // EEPROM_4_PINS
  #define EEPROM_SPI_PORT         PTCD            
  #define EEPROM_SPI_CS           (1<<2)/*(1<<7)*/
  #define EEPROM_SPI_MISO         (1<<6)
  #define EEPROM_SPI_MOSI         (1<<3)  
  #define EEPROM_SPI_CLK          (1<<5)

  #define  EEPROM_SETUP_PORT_C() PTCPE |= EEPROM_SPI_CS;\
                                 PTCDD |= EEPROM_SPI_CLK | EEPROM_SPI_MOSI | EEPROM_SPI_CS;\
                                 PTCDD &= ~EEPROM_SPI_MISO

  #define EEPROM_SETUP_MISO()		 
  #define EEPROM_SETUP_MOSI()    

#endif


#define SET_EEPROM_SPI_CS() EEPROM_SPI_PORT |=  EEPROM_SPI_CS
#define CLR_EEPROM_SPI_CS() EEPROM_SPI_PORT &=  ~EEPROM_SPI_CS

#define SET_EEPROM_SPI_MOSI() EEPROM_SPI_PORT |=  (EEPROM_SPI_MOSI)
#define CLR_EEPROM_SPI_MOSI() EEPROM_SPI_PORT &=  ~(EEPROM_SPI_MOSI)


#define EEPROM_SPI_MISO_STATUS()       ((EEPROM_SPI_PORT & EEPROM_SPI_MISO) == EEPROM_SPI_MISO)

  
#define SET_EEPROM_SPI_CLK() EEPROM_SPI_PORT |=  EEPROM_SPI_CLK
#define CLR_EEPROM_SPI_CLK() EEPROM_SPI_PORT &=  ~EEPROM_SPI_CLK

/// EEPROM COMMANDS:
#define  EEPROM_WREN  0x06
#define  EEPROM_WRDI  0x04
#define  EEPROM_RDSR  0x05
#define  EEPROM_WRSR  0x01
#define  EEPROM_READ  0x03
#define  EEPROM_WRITE 0x02

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/

static void     EEPROM_WriteByteOverSPI(uint8_t out);
static uint8_t  EEPROM_ReadByteOverSPI(void);
static uint8_t  EEPROM_ReadStatusReq(void);
static void     EEPROM_SetWREN(void);
static void     EEPROM_WriteStatusReq(uint8_t Status); 
static ee_err_t EEPROM_WaitForReady(void);
static ee_err_t EEPROM_Write128Data_Aligned(uint16_t Addr);
static void     EEPROM_Delay(void);

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
static uint8_t eepromWritebuffer[EEPROM_PAGE_WRITE];

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
  ee_err_t retval;
  EEPROM_SETUP_PORT_C();
  CLR_EEPROM_SPI_CLK();
  
  retval = EEPROM_WaitForReady();
  if(ee_ok == retval)
  {
    EEPROM_SetWREN();
    EEPROM_WriteStatusReq(0x00); // Set no write protection!.
  }
  return retval;
}

/*****************************************************************************
*  EEPROM_ReadData
*
*  Reads a data buffer from EEPROM, from a given address
*
*****************************************************************************/

ee_err_t EEPROM_ReadData(uint16_t NoOfBytes, uint32_t Addr, uint8_t *inbuf)
{
  ee_err_t retval;
 
  retval = EEPROM_WaitForReady(); 

  if(ee_ok == retval)
  {
    CLR_EEPROM_SPI_CLK();
    CLR_EEPROM_SPI_CS();
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI(EEPROM_READ);
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI((uint8_t) ((Addr >> 8) & 0x00FF));
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI((uint8_t) (Addr & 0x00FF));

    while(NoOfBytes--)
    {
      EEPROM_Delay();  
      *inbuf++ = EEPROM_ReadByteOverSPI();
    }

    SET_EEPROM_SPI_CS();
  }
    
  return retval;
}

/*****************************************************************************
*  EEPROM_WriteData
*
*  Writes a data buffer into EEPROM, at a given address
*
*  This function use an intermediary buffer - 
*  in order to write in more than one page
*****************************************************************************/

ee_err_t EEPROM_WriteData(uint8_t NoOfBytes, uint32_t Addr, uint8_t *Outbuf)
{
  ee_err_t retval=ee_ok;
  uint8_t  index=0;
  uint8_t  indBuffer=0;
  uint16_t eepPageAddress = (uint16_t)(Addr&0xFF80);
  
  /* Read the buffer to be modified */
  if(ee_ok != EEPROM_ReadData(EEPROM_PAGE_WRITE, eepPageAddress ,eepromWritebuffer))
  {
    return ee_error;
  }
  
  indBuffer = (uint8_t)(Addr&0x00FF);
  
  if(indBuffer >= EEPROM_PAGE_WRITE)
  {
    indBuffer-=EEPROM_PAGE_WRITE;
  }
  
  while(NoOfBytes)
  {
    eepromWritebuffer[indBuffer++]=*Outbuf++;
    if((indBuffer == 0)||(indBuffer==EEPROM_PAGE_WRITE))
    {
      break;
    }
    NoOfBytes--;
  }
  
  /* Write the page */
  if(ee_ok != EEPROM_Write128Data_Aligned(eepPageAddress))
  {
    return ee_error;
  }
  
  /* If we didn't finish the buffer put in next page */
  if(0 != NoOfBytes)
  { 
    
    indBuffer =0;
    
    eepPageAddress += EEPROM_PAGE_WRITE;
    
    if(ee_ok != EEPROM_ReadData(EEPROM_PAGE_WRITE,eepPageAddress,eepromWritebuffer))
    {
      return ee_error;
    }
    
    while(NoOfBytes--)
    {
      eepromWritebuffer[indBuffer++]=*Outbuf++;
    }
    
    if(ee_ok != EEPROM_Write128Data_Aligned(eepPageAddress))
    {
      return ee_error;
    }
  }
  return retval;
}

/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************
*  EEPROM_WriteByteOverSPI
*
*
*****************************************************************************/

static void EEPROM_WriteByteOverSPI(uint8_t out)
{
  unsigned char i;           // Bit counter 
  unsigned char in = 0;
 
  EEPROM_SETUP_MOSI(); // setup for write.
  for (i = 0; i < 8; i++) 
  {
    EEPROM_Delay();
    if ((out & 0x80) == 0x80)
      SET_EEPROM_SPI_MOSI();
    else
      CLR_EEPROM_SPI_MOSI();
   
    SET_EEPROM_SPI_CLK();
    EEPROM_Delay();
    CLR_EEPROM_SPI_CLK();
    out = out << 1; 
   }
   CLR_EEPROM_SPI_CLK();  
}

/*****************************************************************************
*  EEPROM_ReadByteOverSPI
*
*
*****************************************************************************/

static uint8_t EEPROM_ReadByteOverSPI(void)
{
  unsigned char i;           // Bit counter 
  unsigned char in = 0;
  EEPROM_SETUP_MISO();

  for (i = 0; i < 8; i++) 
  {
    EEPROM_Delay(); 
    SET_EEPROM_SPI_CLK();
    EEPROM_Delay();
    in = in << 1;
    in |= EEPROM_SPI_MISO_STATUS();
    CLR_EEPROM_SPI_CLK(); 
  }
  return in; 
}

/*****************************************************************************
*  EEPROM_ReadStatusReq
*
*
*****************************************************************************/

static uint8_t EEPROM_ReadStatusReq(void)
{
  uint8_t Status;

  CLR_EEPROM_SPI_CLK();
  CLR_EEPROM_SPI_CS();
  EEPROM_Delay();
  EEPROM_WriteByteOverSPI(EEPROM_RDSR);
  EEPROM_Delay();  
  Status = EEPROM_ReadByteOverSPI();
  SET_EEPROM_SPI_CS();
  return Status;  
}

/*****************************************************************************
*  EEPROM_SetWREN
*
*
*****************************************************************************/

static void EEPROM_SetWREN(void)
{
  CLR_EEPROM_SPI_CS();
  EEPROM_Delay();
  EEPROM_WriteByteOverSPI(EEPROM_WREN);
  SET_EEPROM_SPI_CS();
}

/*****************************************************************************
*  EEPROM_WriteStatusReq
*
*
*****************************************************************************/

static void EEPROM_WriteStatusReq(uint8_t Status)
{
  CLR_EEPROM_SPI_CS();
  EEPROM_Delay();
  EEPROM_WriteByteOverSPI(EEPROM_WRSR);
  EEPROM_Delay();
  EEPROM_WriteByteOverSPI(Status);

  SET_EEPROM_SPI_CS();
  CLR_EEPROM_SPI_CLK();
}

/*****************************************************************************
*  EEPROM_WaitForReady
*
*
*****************************************************************************/

static ee_err_t EEPROM_WaitForReady(void)
{
  /*
       50 ms @ EEPROM_Delay_STEP = 0,1,2
      300 ms @ EEPROM_Delay_STEP = 3
      550 ms @ EEPROM_Delay_STEP = 4
  */
  volatile uint16_t wait = 0x400; // near 50 ms @ 50 us/cycle

  while ( ((EEPROM_ReadStatusReq() & 0x01) == 0x01) && (wait !=0) )
  {
    wait--;
  }

  if(wait != 0)
  {
    return ee_ok;
  }
  return ee_error;
}

/*****************************************************************************
*  EEPROM_Write128Data_Aligned
*
*
*****************************************************************************/
static ee_err_t EEPROM_Write128Data_Aligned(uint16_t Addr)
{
  ee_err_t retval; 
  uint8_t i;
 
  retval = EEPROM_WaitForReady(); 

  if(ee_ok == retval)  
  {
    EEPROM_SetWREN();  

    CLR_EEPROM_SPI_CLK();
    CLR_EEPROM_SPI_CS();
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI(EEPROM_WRITE);
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI((uint8_t) ((Addr >> 8) & 0x00FF));
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI((uint8_t) (Addr & 0x00FF));

    for (i = 0; i < EEPROM_PAGE_WRITE; i++) 
    {
      EEPROM_Delay();  
      EEPROM_WriteByteOverSPI(eepromWritebuffer[i]);
    }

    SET_EEPROM_SPI_CS();
  }
  return retval;
}
/*****************************************************************************
*  EEPROM_Delay
*
*
*****************************************************************************/
static void EEPROM_Delay(void)
{
  volatile uint8_t i=8; 
  do {} while(--i);
}




