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
/*      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
      Check compiler in order not to call functions from ANSI*.LIB  
                Check list file or map file 
*/

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
#include "BootloaderEeprom.h"

/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/
//#define EEPROM_BOOT_3_PINS
#define EEPROM_BOOT_4_PINS

#ifdef EEPROM_BOOT_3_PINS
  #define EEPROM_BOOT_SPI_PORT         PTCD            
  #define EEPROM_BOOT_SPI_CS           (1<<5)/*(1<<7)*/
  #define EEPROM_BOOT_SPI_MISO         (1<<6)
  #define EEPROM_BOOT_SPI_MOSI         (1<<7)/*(1<<6)*/
  #define EEPROM_BOOT_SPI_CLK          (1<<4)

  #define  EEPROM_BOOT_SETUP_PORT_C() PTCPE |= EEPROM_BOOT_SPI_CS;\
                                 PTCDD |= EEPROM_BOOT_SPI_CLK | EEPROM_BOOT_SPI_MOSI | EEPROM_BOOT_SPI_CS;\
                                 PTCDD &= ~EEPROM_BOOT_SPI_MISO

  #define EEPROM_BOOT_SETUP_MISO()		 PTCDD &= ~EEPROM_BOOT_SPI_MISO
  #define EEPROM_BOOT_SETUP_MOSI()    PTCDD |=  EEPROM_BOOT_SPI_MOSI

#else /* EEPROM_BOOT_4_PINS */
  #define EEPROM_BOOT_SPI_PORT         PTCD            
  #define EEPROM_BOOT_SPI_CS           (1<<5)/*(1<<7)*/
  #define EEPROM_BOOT_SPI_MISO         (1<<6)
  #define EEPROM_BOOT_SPI_MOSI         (1<<7)  
  #define EEPROM_BOOT_SPI_CLK          (1<<4)

  #define  EEPROM_BOOT_SETUP_PORT_C() PTCPE |= EEPROM_BOOT_SPI_CS;\
                                 PTCDD |= EEPROM_BOOT_SPI_CLK | EEPROM_BOOT_SPI_MOSI | EEPROM_BOOT_SPI_CS;\
                                 PTCDD &= ~EEPROM_BOOT_SPI_MISO

  #define EEPROM_BOOT_SETUP_MISO()		 
  #define EEPROM_BOOT_SETUP_MOSI()    

#endif


#define SET_EEPROM_BOOT_SPI_CS() EEPROM_BOOT_SPI_PORT |=  EEPROM_BOOT_SPI_CS
#define CLR_EEPROM_BOOT_SPI_CS() EEPROM_BOOT_SPI_PORT &=  ~EEPROM_BOOT_SPI_CS

#define SET_EEPROM_BOOT_SPI_MOSI() EEPROM_BOOT_SPI_PORT |=  (EEPROM_BOOT_SPI_MOSI)
#define CLR_EEPROM_BOOT_SPI_MOSI() EEPROM_BOOT_SPI_PORT &=  ~(EEPROM_BOOT_SPI_MOSI)


#define EEPROM_BOOT_SPI_MISO_STATUS()       ((EEPROM_BOOT_SPI_PORT & EEPROM_BOOT_SPI_MISO) == EEPROM_BOOT_SPI_MISO)

  
#define SET_EEPROM_BOOT_SPI_CLK() EEPROM_BOOT_SPI_PORT |=  EEPROM_BOOT_SPI_CLK
#define CLR_EEPROM_BOOT_SPI_CLK() EEPROM_BOOT_SPI_PORT &=  ~EEPROM_BOOT_SPI_CLK


#define  EEPROM_BOOT_RDSR            0xD7
#define  EEPROM_BOOT_READ            0x03
#define  EEPROM_BOOT_WRITE           0x82
#define  EEPROM_BOOT_PAGE_TO_BUFFER  0x53
#define  EEPROM_BOOT_BUFFER_TO_PAGE  0x83
#define  EEPROM_BOOT_BUFFER_WRITE    0x84

/* adress mask */
#define ADDRESS_MASK 0x000000FF

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
#ifdef gUseBootloader_d
#pragma CODE_SEG __NEAR_SEG BOOTLOADER_CODE_ROM
#endif
  static uint8_t  EEPROM_BOOT_ReadByteOverSPI(void);       
  static uint8_t  EEPROM_BOOT_ReadStatusReq(void);
  static boot_ee_err_t EEPROM_BOOT_WaitForReady(void);
  static boot_ee_err_t EEPROM_BOOT_Command(uint8_t opCode, uint32_t Addr);
  static void     EEPROM_BOOT_WriteByteOverSPI(uint8_t out);
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
  boot_ee_err_t retval;
 
  retval = EEPROM_BOOT_Command(EEPROM_BOOT_READ,Addr); 

  if(boot_ee_ok == retval)
  {
    while(NoOfBytes--)
    {
        
        EEPROM_BOOT_Delay();  
        *inbuf++ = EEPROM_BOOT_ReadByteOverSPI();
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
  uint8_t status;

  CLR_EEPROM_BOOT_SPI_CLK();
  CLR_EEPROM_BOOT_SPI_CS();
  EEPROM_BOOT_Delay();
  EEPROM_BOOT_WriteByteOverSPI(EEPROM_BOOT_RDSR);
  EEPROM_BOOT_Delay();  
  status = EEPROM_BOOT_ReadByteOverSPI();
  SET_EEPROM_BOOT_SPI_CS();
  return status;  
}


/*****************************************************************************
*  EEPROM_BOOT_WaitForReady
*
*
*****************************************************************************/
static boot_ee_err_t EEPROM_BOOT_WaitForReady(void)
{
  /*
       50 ms @ EEPROM_BOOT_DELAY_STEP = 0,1,2
      300 ms @ EEPROM_BOOT_DELAY_STEP = 3
      550 ms @ EEPROM_BOOT_DELAY_STEP = 4
  */
  volatile uint16_t wait = 0x400; // near 50 ms @ 50 us/cycle

  //Bit     7     6    5  4  3  2    1         0
  //    RDY/BUSY COMP  0  1  0  1  PROTECT PAGE_SIZE

  while (!(EEPROM_BOOT_ReadStatusReq() & 0x80) && (wait !=0) )
  {
    wait--;
  }

  if(wait != 0)
  {
    return boot_ee_ok;
  }
  return boot_ee_error;
}


/*****************************************************************************
*  EEPROM_BOOT_Command
*
*
*****************************************************************************/
static boot_ee_err_t EEPROM_BOOT_Command(uint8_t opCode, uint32_t Addr)
{
  boot_ee_err_t retval;
   
  retval = EEPROM_BOOT_WaitForReady();
   
  if(boot_ee_ok == retval)
  {
    CLR_EEPROM_BOOT_SPI_CLK();
    CLR_EEPROM_BOOT_SPI_CS();
    EEPROM_BOOT_Delay();
    EEPROM_BOOT_WriteByteOverSPI(opCode);
    EEPROM_BOOT_Delay();
    EEPROM_BOOT_WriteByteOverSPI((uint8_t) ((Addr >> 16) & ADDRESS_MASK));
    EEPROM_BOOT_Delay();
    EEPROM_BOOT_WriteByteOverSPI((uint8_t) ((Addr >> 8)  & ADDRESS_MASK));
    EEPROM_BOOT_Delay();
    EEPROM_BOOT_WriteByteOverSPI((uint8_t) ((Addr >> 0)  & ADDRESS_MASK));
    EEPROM_BOOT_Delay();
  }
  return retval;
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

#ifdef gUseBootloader_d
  #pragma CODE_SEG DEFAULT
#endif




