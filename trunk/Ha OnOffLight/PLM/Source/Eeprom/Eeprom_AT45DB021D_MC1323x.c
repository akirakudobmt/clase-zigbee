/******************************************************************************
* This is the Source file for the AT45DB021D EEPROM driver
*														 
* (c) Copyright 2010, FreeScale Semiconductor inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from FreeScale.
*
******************************************************************************/
/*
  
  The devices are initially shipped with the page size set to 264-bytes.
  
  
                    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  Power of 2 binary page size Configuration Register is a user-programmable
  nonvolatile register that allows the page size of the main memory to be
  configured for binary page size (256-bytes) or the Atmel® DataFlash® standard
  page size (264-bytes). The power of 2 page size is a One-time Programmable (OTP)
  register and once the device is configured for power of 2 page size,
  it cannot be reconfigured again. 
  

          Bit     7     6    5  4  3  2    1         0
              RDY/BUSY COMP  0  1  0  1  PROTECT PAGE_SIZE
              
              
      Power of Two Page Size 0x3D 0x2A 0x80 0xA6
 
  static ee_err_t EEPROM_WriteFusePage256(void)
 {
   volatile uint16_t wait = 0x400; // near 50 ms @ 50 us/cycle

    Bit     7     6    5  4  3  2    1         0
        RDY/BUSY COMP  0  1  0  1  PROTECT PAGE_SIZE

   while (!(EEPROM_ReadStatusReq() & 0x80) && (wait !=0) )
   {
     wait--;
   }

   CLR_EEPROM_SPI_CLK();
   CLR_EEPROM_SPI_CS();
   EEPROM_Delay();
   EEPROM_WriteByteOverSPI(0x3D);
   EEPROM_Delay();
   EEPROM_WriteByteOverSPI(0x2A);
   EEPROM_Delay();
   EEPROM_WriteByteOverSPI(0x80);
   EEPROM_Delay();
   EEPROM_WriteByteOverSPI(0xA6);
   SET_EEPROM_SPI_CS();
  
   return 0;
 }
            
*/



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

#ifdef EEPROM_3_PINS
  #define EEPROM_SPI_PORT         PTCD            
  #define EEPROM_SPI_CS           (1<<5)/*(1<<7)*/
  #define EEPROM_SPI_MISO         (1<<6)
  #define EEPROM_SPI_MOSI         (1<<7)/*(1<<6)*/
  #define EEPROM_SPI_CLK          (1<<4)

  #define  EEPROM_SETUP_PORT_C() PTCPE |= EEPROM_SPI_CS;\
                                 PTCDD |= EEPROM_SPI_CLK | EEPROM_SPI_MOSI | EEPROM_SPI_CS;\
                                 PTCDD &= ~EEPROM_SPI_MISO

  #define EEPROM_SETUP_MISO()		 PTCDD &= ~EEPROM_SPI_MISO
  #define EEPROM_SETUP_MOSI()    PTCDD |=  EEPROM_SPI_MOSI

#else // EEPROM_4_PINS
  #define EEPROM_SPI_PORT         PTCD            
  #define EEPROM_SPI_CS           (1<<5)/*(1<<7)*/
  #define EEPROM_SPI_MISO         (1<<6)
  #define EEPROM_SPI_MOSI         (1<<7)  
  #define EEPROM_SPI_CLK          (1<<4)

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


#define  EEPROM_RDSR            0xD7
#define  EEPROM_READ            0x03
#define  EEPROM_WRITE           0x82
#define  EEPROM_PAGE_TO_BUFFER  0x53
#define  EEPROM_BUFFER_TO_PAGE  0x83
#define  EEPROM_BUFFER_WRITE    0x84

/* adress mask */
#define ADDRESS_MASK 0x000000FF

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
static uint8_t  EEPROM_ReadByteOverSPI(void);       
static uint8_t  EEPROM_ReadStatusReq(void);
static ee_err_t EEPROM_WaitForReady(void);
static ee_err_t EEPROM_Command(uint8_t opCode, uint32_t Addr);
static void     EEPROM_WriteByteOverSPI(uint8_t out);
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
  //uint16_t i;
  ee_err_t retval;
 
  retval = EEPROM_Command(EEPROM_READ,Addr); 

  if(ee_ok == retval)
  {
    while(NoOfBytes--)
    {
        
        EEPROM_Delay();  
        *inbuf++ = EEPROM_ReadByteOverSPI();
    }
    SET_EEPROM_SPI_CS();
  }
    
  return retval;
}

#ifdef gUseBootloader_d
  #pragma CODE_SEG DEFAULT
#endif



/*****************************************************************************
*  EEPROM_WriteData
*
*  Writes a data buffer into EEPROM, at a given address
*
*****************************************************************************/
ee_err_t EEPROM_WriteData(uint8_t NoOfBytes, uint32_t Addr, uint8_t *Outbuf)
{
  ee_err_t retval; 
  uint32_t AddrOld = Addr;
  uint16_t *pIncrAddr=(uint16_t *)&Addr; 
  
  /* Done to not use ansi lib*/
  /* Point to LSB*/
  pIncrAddr++;
  
  while(NoOfBytes)
  {
    retval = EEPROM_Command(EEPROM_PAGE_TO_BUFFER, Addr);
    if(ee_ok != retval)
    {
      break; 
    }
    SET_EEPROM_SPI_CS();
    EEPROM_Delay();
    
    retval = EEPROM_Command(EEPROM_BUFFER_WRITE, Addr);
    if(ee_ok != retval)
    {
      break; 
    }
    
    do
    {
      EEPROM_Delay();  
      EEPROM_WriteByteOverSPI(*Outbuf++);
      
      /* Optimization -> not use ansi lib*/
      /* Addr++ */
      (*pIncrAddr)++;
      if(!(*pIncrAddr))
      {
        /* Point to MSB */
        pIncrAddr--;
        (*pIncrAddr)++;
        /* Point to LSB */
        pIncrAddr++;
      }
      
      NoOfBytes--;
    }while((Addr & ADDRESS_MASK) && (NoOfBytes));
    
    SET_EEPROM_SPI_CS();
    EEPROM_Delay();
    retval = EEPROM_Command(EEPROM_BUFFER_TO_PAGE, AddrOld);
    if(ee_ok != retval)
    {
      break; 
    }
    AddrOld = Addr;
    SET_EEPROM_SPI_CS();
    EEPROM_Delay();
  } //end while
  
  SET_EEPROM_SPI_CS();
  return retval;
}


/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
******************************************************************************/

/*****************************************************************************
*  EEPROM_Delay
*
*  Initializes the EEPROM peripheral
*
*****************************************************************************/
static void EEPROM_Delay(void)
{ 
  volatile uint8_t i=8;
  while(--i);
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
  uint8_t status;

  CLR_EEPROM_SPI_CLK();
  CLR_EEPROM_SPI_CS();
  EEPROM_Delay();
  EEPROM_WriteByteOverSPI(EEPROM_RDSR);
  EEPROM_Delay();  
  status = EEPROM_ReadByteOverSPI();
  SET_EEPROM_SPI_CS();
  return status;  
}


/*****************************************************************************
*  EEPROM_WaitForReady
*
*
*****************************************************************************/
static ee_err_t EEPROM_WaitForReady(void)
{
  /*
       50 ms @ EEPROM_DELAY_STEP = 0,1,2
      300 ms @ EEPROM_DELAY_STEP = 3
      550 ms @ EEPROM_DELAY_STEP = 4
  */
  volatile uint16_t wait = 0x400; // near 50 ms @ 50 us/cycle
  volatile status = 0;
  

  /*Bit     7     6    5  4  3  2    1         0
        RDY/BUSY COMP  0  1  0  1  PROTECT PAGE_SIZE  */

  while (!(status & 0x80) && (wait !=0) )
  {
    
    status = EEPROM_ReadStatusReq();
    wait--;
  }
  /* Check if the page size is 256 bytes - if not make it */
  if(!(status & 0x01))
  {
     CLR_EEPROM_SPI_CLK();
     CLR_EEPROM_SPI_CS();
     EEPROM_Delay();
     EEPROM_WriteByteOverSPI(0x3D);
     EEPROM_Delay();
     EEPROM_WriteByteOverSPI(0x2A);
     EEPROM_Delay();
     EEPROM_WriteByteOverSPI(0x80);
     EEPROM_Delay();
     EEPROM_WriteByteOverSPI(0xA6);
     SET_EEPROM_SPI_CS();
  }

  wait = 0x400;
  status = 0;

  while (!(status & 0x80) && (wait !=0) )
  {
    
    status = EEPROM_ReadStatusReq();
    wait--;
  }
  
  if(wait != 0)
  {
    return ee_ok;
  }
  return ee_error;
}


/*****************************************************************************
*  EEPROM_Command
*
*
*****************************************************************************/
static ee_err_t EEPROM_Command(uint8_t opCode, uint32_t Addr)
{
  ee_err_t retval;
   
  retval = EEPROM_WaitForReady();
   
  if(ee_ok == retval)
  {
    CLR_EEPROM_SPI_CLK();
    CLR_EEPROM_SPI_CS();
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI(opCode);
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI((uint8_t) ((Addr >> 16) & ADDRESS_MASK));
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI((uint8_t) ((Addr >> 8)  & ADDRESS_MASK));
    EEPROM_Delay();
    EEPROM_WriteByteOverSPI((uint8_t) ((Addr >> 0)  & ADDRESS_MASK));
    EEPROM_Delay();
  }
  return retval;
}

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




