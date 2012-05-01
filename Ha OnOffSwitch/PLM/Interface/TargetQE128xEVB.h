/******************************************************************************
* This header file is for the MC1319x EVB port mappings.
*
* (c) Copyright 2006, Freescale, Inc. All rights reserved.
*
* 
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
******************************************************************************/

#ifndef _TARGET_QE128_EVB_
#define _TARGET_QE128_EVB_
#include "IoConfig.h"

////////////////////////////////////////
/////// Define LED port mapping ////////
////////////////////////////////////////
  #define LED_PORT  PTED
  #define LED1_PIN  0x08
  #define LED2_PIN  0x10
  #define LED3_PIN  0x20
  #define LED4_PIN  0x40
  #define LED_MASK  (LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN)

  #define LED1ON            LED_PORT &= ~LED1_PIN;
  #define LED1OFF           LED_PORT |= LED1_PIN;
  #define LED1TOGGLE        LED_PORT ^= LED1_PIN;

  #define LED2ON            LED_PORT &= ~LED2_PIN;
  #define LED2OFF           LED_PORT |= LED2_PIN;
  #define LED2TOGGLE        LED_PORT ^= LED2_PIN;

  #define LED3ON            LED_PORT &= ~LED3_PIN;
  #define LED3OFF           LED_PORT |= LED3_PIN;
  #define LED3TOGGLE        LED_PORT ^= LED3_PIN;

  #define LED4ON            LED_PORT &= ~LED4_PIN;
  #define LED4OFF           LED_PORT |= LED4_PIN;
  #define LED4TOGGLE        LED_PORT ^= LED4_PIN;

  // Turn off LEDs
  #define TURN_OFF_LEDS     LED_PORT  |= LED_MASK;
                            // Note! LED5 is used as power LED.
  // Turn on LEDs
  #define TURN_ON_LEDS      LED_PORT  &= ~LED_MASK;
                            // Note! LED5 is used as power LED.

////////////////////////////////////////
/////// Define PORT PINS mapping ///////
////////////////////////////////////////
  #define PORT_PINS         PTAD
  #define PORT1_PIN         0x01
  #define PORT2_PIN         0x02
  #define PORT3_PIN         0x04
  #define PORT4_PIN         0x08
  #define PORT_PINS_MASK    (PORT1_PIN | PORT2_PIN | PORT3_PIN | PORT4_PIN)


////////////////////////////////////////
////// Define PCB switch mapping ///////
////////////////////////////////////////
  #define SWITCH_PORT   PTDD
  
#ifndef mSWITCH1_MASK  
  #define mSWITCH1_MASK 0x01
#endif  

#ifndef mSWITCH2_MASK  
  #define mSWITCH2_MASK 0x02
#endif  

#ifndef mSWITCH3_MASK  
  #define mSWITCH3_MASK 0x04
#endif  

#ifndef mSWITCH4_MASK  
  #define mSWITCH4_MASK 0x08
#endif  

  #define mSWITCH_MASK  (mSWITCH1_MASK | mSWITCH2_MASK |  mSWITCH3_MASK | mSWITCH4_MASK)

  #define mSWITCH_PORT_GET  ((SWITCH_PORT & mSWITCH_MASK) ^ mSWITCH_MASK)



////////////////////////////////////////
///// Define HW port/pin mapping ///////
////////////////////////////////////////

  // Define HW pin mapping
  #define gMC1319xAttnPort       PTCD
  #define gMC1319xRxTxPort       PTCD
  #define gMC1319xResetPort      PTCD
  #define gMC1319xIrqPort        PTAD


  #define gMC1319xAttnMask_c     0x02
  #define gMC1319xRxTxMask_c     0x04
  #define gMC1319xResetMask_c    0x01
  #define gMC1319xIrqMask        0x20

  #define gMC1319xGPIO1Port      PTCD
  #define gMC1319xGPIO2Port      PTCD
  #define gMC1319xAntSwPort      PTCD

  #define gMC1319xGPIO1Mask_c    0x08
  #define gMC1319xGPIO2Mask_c    0x10
  #define gMC1319xAntSwMask_c    0x40

  #define gMC1319xSPIPort        PTBD
  
  #define gMC1319xSpiSsMask_c    0x20
  #define gMC1319xSpiMisoMask_c  0x08
  #define gMC1319xSpiMosiMask_c  0x04
  #define gMC1319xSpiSpsckMask_c 0x20
  

  // Define HW port mapping (Refer to MCU IO spreadsheet or schematic for details)
  #define gPinPullUpEnable_c     0x01
  #define gPinPullUpDisable_c    0x00
  
  #define gPinDirectionOutput_c  0x01
  #define gPinDirectionInput_c   0x00
  
  #define gPinLevelHigh_c        0x01
  #define gPinLevelLow_c         0x00
  
  #define gPinDriveStrengthHigh  0x01
  #define gPinDriveStrengthLow   0x00
  
  #define gPinSlewRateEnable     0x01
  #define gPinSlewRateDisable    0x00
  
  #define PORT_SETUP(b7, b6, b5, b4, b3, b2, b1, b0)    (uint8_t)( ((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | (b0))
  
  
  
  /* Port A. Add pins for application if needed */  
  #define  mSETUP_PORT_A    PTAD  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port A, Pin 7 */ \
                                                 gPinLevelLow_c,           /* Port A, Pin 6 */ \
                                                 gPinLevelLow_c,           /* Port A, Pin 5 -> IRQ */ \
                                                 gPinLevelLow_c,           /* Port A, Pin 4 */ \
                                                 gPinLevelLow_c,           /* Port A, Pin 3 */ \
                                                 gPinLevelLow_c,           /* Port A, Pin 2 -> SCL */ \
                                                 gPinLevelLow_c,           /* Port A, Pin 1 -> SDA */ \
                                                 gPinLevelLow_c            /* Port A, Pin 0 */ \
                                               );\
                            PTAPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port A, Pin 7 */ \
                                                 gPinPullUpDisable_c,      /* Port A, Pin 6 */ \
                                                 gPinPullUpEnable_c,       /* Port A, Pin 5 -> IRQ */ \
                                                 gPinPullUpDisable_c,      /* Port A, Pin 4 */ \
                                                 gPinPullUpDisable_c,      /* Port A, Pin 3 */ \
                                                 gPinPullUpDisable_c,      /* Port A, Pin 2 -> SCL */ \
                                                 gPinPullUpDisable_c,      /* Port A, Pin 1 -> SDA */ \
                                                 gPinPullUpDisable_c       /* Port A, Pin 0 */ \
                                              );\
                            PTADD = PORT_SETUP(\
                                                 gPinDirectionOutput_c,    /* Port A, Pin 7 */ \
                                                 gPinDirectionOutput_c,    /* Port A, Pin 6 */ \
                                                 gPinDirectionInput_c,     /* Port A, Pin 5 -> IRQ */ \
                                                 gPinDirectionOutput_c,    /* Port A, Pin 4 */ \
                                                 gPinDirectionOutput_c,    /* Port A, Pin 3 */ \
                                                 gPinDirectionInput_c,     /* Port A, Pin 2 -> SCL */ \
                                                 gPinDirectionInput_c,     /* Port A, Pin 1 -> SDA */ \
                                                 gPinDirectionOutput_c     /* Port A, Pin 0 */ \
                                              );\
                            PTADS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port A, Pin 7 */ \
                                                 gPinDriveStrengthLow,     /* Port A, Pin 6 */ \
                                                 gPinDriveStrengthLow,     /* Port A, Pin 5 -> IRQ */ \
                                                 gPinDriveStrengthLow,     /* Port A, Pin 4 */ \
                                                 gPinDriveStrengthLow,     /* Port A, Pin 3 */ \
                                                 gPinDriveStrengthLow,     /* Port A, Pin 2 -> SCL */ \
                                                 gPinDriveStrengthLow,     /* Port A, Pin 1 -> SDA */ \
                                                 gPinDriveStrengthLow      /* Port A, Pin 0 */ \
                                              );                                              

  /* Port B. Add pins for application if needed */  
  #define  mSETUP_PORT_B    PTBD  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port B, Pin 7 -> EXTAL */ \
                                                 gPinLevelLow_c,           /* Port B, Pin 6 */ \
                                                 gPinLevelHigh_c,          /* Port B, Pin 5 -> SPI1 SS */ \
                                                 gPinLevelLow_c,           /* Port B, Pin 4 -> SPI1 MISO */ \
                                                 gPinLevelHigh_c,          /* Port B, Pin 3 -> SPI1 MOSI */ \
                                                 gPinLevelHigh_c,          /* Port B, Pin 2 -> SPI1 CLK */ \
                                                 gPinLevelLow_c,           /* Port B, Pin 1 -> SCI1 TX */ \
                                                 gPinLevelLow_c            /* Port B, Pin 0 -> SCI1 RX */ \
                                              );\
                            PTBPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port B, Pin 7 -> EXTAL */ \
                                                 gPinPullUpDisable_c,      /* Port B, Pin 6 */ \
                                                 gPinPullUpDisable_c,      /* Port B, Pin 5 -> SPI1 SS */ \
                                                 gPinPullUpDisable_c,      /* Port B, Pin 4 -> SPI1 MISO */ \
                                                 gPinPullUpDisable_c,      /* Port B, Pin 3 -> SPI1 MOSI */ \
                                                 gPinPullUpDisable_c,      /* Port B, Pin 2 -> SPI1 CLK */ \
                                                 gPinPullUpDisable_c,      /* Port B, Pin 1 -> SCI1 TX */ \
                                                 gPinPullUpDisable_c       /* Port B, Pin 0 -> SCI1 RX */ \
                                              );\
                            PTBDD = PORT_SETUP(\
                                                 gPinDirectionInput_c,     /* Port B, Pin 7 -> EXTAL */ \
                                                 gPinDirectionOutput_c,    /* Port B, Pin 6 */ \
                                                 gPinDirectionOutput_c,    /* Port B, Pin 5 -> SPI1 SS */ \
                                                 gPinDirectionInput_c,     /* Port B, Pin 4 -> SPI1 MISO */ \
                                                 gPinDirectionOutput_c,    /* Port B, Pin 3 -> SPI1 MOSI */ \
                                                 gPinDirectionOutput_c,    /* Port B, Pin 2 -> SPI1 CLK */ \
                                                 gPinDirectionOutput_c,    /* Port B, Pin 1 -> SCI1 TX */ \
                                                 gPinDirectionInput_c      /* Port B, Pin 0 -> SCI1 RX */ \
                                              );\
                            PTBDS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port B, Pin 7 -> EXTAL */ \
                                                 gPinDriveStrengthLow,     /* Port B, Pin 6 */ \
                                                 gPinDriveStrengthHigh,    /* Port B, Pin 5 -> SPI1 SS (High drive strength required for fast switching at high speed SPI) */ \
                                                 gPinDriveStrengthLow,     /* Port B, Pin 4 -> SPI1 MISO */ \
                                                 gPinDriveStrengthLow,     /* Port B, Pin 3 -> SPI1 MOSI */ \
                                                 gPinDriveStrengthLow,     /* Port B, Pin 2 -> SPI1 CLK */ \
                                                 gPinDriveStrengthLow,     /* Port B, Pin 1 -> SCI1 TX */ \
                                                 gPinDriveStrengthLow      /* Port B, Pin 0 -> SCI1 RX */ \
                                              );                                             
                                                                                                                               
  /* Port C. Add pins for application if needed */                                                                                         
  #define  mSETUP_PORT_C    PTCD  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port C, Pin 7 */ \
                                                 gPinLevelLow_c,           /* Port C, Pin 6 -> TRX AntSw */ \
                                                 gPinLevelHigh_c,           /* Port C, Pin 5 */ \
                                                 gPinLevelLow_c,           /* Port C, Pin 4 -> GPIO2 */ \
                                                 gPinLevelLow_c,           /* Port C, Pin 3 -> GPIO1 */ \
                                                 gPinLevelHigh_c,          /* Port C, Pin 2 -> RTXEN */ \
                                                 gPinLevelHigh_c,          /* Port C, Pin 1 -> ATTN */ \
                                                 gPinLevelHigh_c           /* Port C, Pin 0 -> RSTO */ \
                                              );\
                            PTCPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port C, Pin 7 */ \
                                                 gPinPullUpDisable_c,      /* Port C, Pin 6 -> TRX AntSw */ \
                                                 gPinPullUpDisable_c,      /* Port C, Pin 5 */ \
                                                 gPinPullUpDisable_c,      /* Port C, Pin 4 -> GPIO2 */ \
                                                 gPinPullUpDisable_c,      /* Port C, Pin 3 -> GPIO1 */ \
                                                 gPinPullUpDisable_c,      /* Port C, Pin 2 -> RTXEN */ \
                                                 gPinPullUpDisable_c,      /* Port C, Pin 1 -> ATTN */ \
                                                 gPinPullUpDisable_c       /* Port C, Pin 0 -> RSTO */ \
                                              );\
                            PTCDD = PORT_SETUP(\
                                                 gPinDirectionOutput_c,    /* Port C, Pin 7 */ \
                                                 gPinDirectionOutput_c,    /* Port C, Pin 6 -> TRX AntSw */ \
                                                 gPinDirectionOutput_c,    /* Port C, Pin 5 */ \
                                                 gPinDirectionInput_c,     /* Port C, Pin 4 -> GPIO2 */ \
                                                 gPinDirectionInput_c,     /* Port C, Pin 3 -> GPIO1 */ \
                                                 gPinDirectionOutput_c,    /* Port C, Pin 2 -> RTXEN */ \
                                                 gPinDirectionOutput_c,    /* Port C, Pin 1 -> ATTN */ \
                                                 gPinDirectionOutput_c     /* Port C, Pin 0 -> RSTO */ \
                                               );\
                            PTCDS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port C, Pin 7 */ \
                                                 gPinDriveStrengthLow,     /* Port C, Pin 6 -> TRX AntSw */ \
                                                 gPinDriveStrengthLow,     /* Port C, Pin 5 */ \
                                                 gPinDriveStrengthLow,     /* Port C, Pin 4 -> GPIO1 */ \
                                                 gPinDriveStrengthLow,     /* Port C, Pin 3 -> GPIO1 */ \
                                                 gPinDriveStrengthHigh,    /* Port C, Pin 2 -> RTXEN (High drive strength required for fast switching at high speed SPI) */ \
                                                 gPinDriveStrengthHigh,    /* Port C, Pin 1 -> ATTN */ \
                                                 gPinDriveStrengthHigh     /* Port C, Pin 0 -> RSTO */ \
                                              );                                               
                            
  /* Port D. Add pins for application if needed */  
  #define  mSETUP_PORT_D    PTDD  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port D, Pin 7 */ \
                                                 gPinLevelLow_c,           /* Port D, Pin 6 */ \
                                                 gPinLevelLow_c,           /* Port D, Pin 5 */ \
                                                 gPinLevelLow_c,           /* Port D, Pin 4 */ \
                                                 gPinLevelLow_c,           /* Port D, Pin 3 */ \
                                                 gPinLevelLow_c,           /* Port D, Pin 2 */ \
                                                 gPinLevelLow_c,           /* Port D, Pin 1 */ \
                                                 gPinLevelLow_c            /* Port D, Pin 0 */ \
                                              );\
                            PTDPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port D, Pin 7 */ \
                                                 gPinPullUpDisable_c,      /* Port D, Pin 6 */ \
                                                 gPinPullUpDisable_c,      /* Port D, Pin 5 */ \
                                                 gPinPullUpDisable_c,      /* Port D, Pin 4 */ \
                                                 gPinPullUpEnable_c,       /* Port D, Pin 3 */ \
                                                 gPinPullUpEnable_c,       /* Port D, Pin 2 */ \
                                                 gPinPullUpEnable_c,       /* Port D, Pin 1 */ \
                                                 gPinPullUpEnable_c        /* Port D, Pin 0 */ \
                                              );\
                            PTDDD = PORT_SETUP(\
                                                 gPinDirectionOutput_c,    /* Port D, Pin 7 */ \
                                                 gPinDirectionOutput_c,    /* Port D, Pin 6 */ \
                                                 gPinDirectionOutput_c,    /* Port D, Pin 5 */ \
                                                 gPinDirectionOutput_c,    /* Port D, Pin 4 */ \
                                                 gPinDirectionInput_c,     /* Port D, Pin 3 -> KBI4 */ \
                                                 gPinDirectionInput_c,     /* Port D, Pin 2 -> KBI3 */ \
                                                 gPinDirectionInput_c,     /* Port D, Pin 1 -> KBI2 */ \
                                                 gPinDirectionInput_c      /* Port D, Pin 0 -> KBI1 */ \
                                               );\
                            PTDDS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port D, Pin 7 */ \
                                                 gPinDriveStrengthLow,     /* Port D, Pin 6 */ \
                                                 gPinDriveStrengthLow,     /* Port D, Pin 5 */ \
                                                 gPinDriveStrengthLow,     /* Port D, Pin 4 */ \
                                                 gPinDriveStrengthLow,     /* Port D, Pin 3 */ \
                                                 gPinDriveStrengthLow,     /* Port D, Pin 2 */ \
                                                 gPinDriveStrengthLow,     /* Port D, Pin 1 */ \
                                                 gPinDriveStrengthLow      /* Port D, Pin 0 */ \
                                              );                                                

  /* Port E. Add pins for application if needed */  
  #define  mSETUP_PORT_E    PTED  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port E, Pin 7 */ \
                                                 gPinLevelLow_c,           /* Port E, Pin 6 */ \
                                                 gPinLevelLow_c,           /* Port E, Pin 5 */ \
                                                 gPinLevelLow_c,           /* Port E, Pin 4 */ \
                                                 gPinLevelLow_c,           /* Port E, Pin 3 */ \
                                                 gPinLevelLow_c,           /* Port E, Pin 2 */ \
                                                 gPinLevelLow_c,           /* Port E, Pin 1 */ \
                                                 gPinLevelLow_c            /* Port E, Pin 0 */ \
                                              );\
                            PTEPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port E, Pin 7 */ \
                                                 gPinPullUpDisable_c,      /* Port E, Pin 6 */ \
                                                 gPinPullUpDisable_c,      /* Port E, Pin 5 */ \
                                                 gPinPullUpDisable_c,      /* Port E, Pin 4 */ \
                                                 gPinPullUpDisable_c,      /* Port E, Pin 3 */ \
                                                 gPinPullUpDisable_c,      /* Port E, Pin 2 */ \
                                                 gPinPullUpDisable_c,      /* Port E, Pin 1 */ \
                                                 gPinPullUpDisable_c       /* Port E, Pin 0 */ \
                                              );\
                            PTEDD = PORT_SETUP(\
                                                 gPinDirectionOutput_c,    /* Port E, Pin 7 */ \
                                                 gPinDirectionOutput_c,    /* Port E, Pin 6 */ \
                                                 gPinDirectionOutput_c,    /* Port E, Pin 5 */ \
                                                 gPinDirectionOutput_c,    /* Port E, Pin 4 */ \
                                                 gPinDirectionOutput_c,    /* Port E, Pin 3 */ \
                                                 gPinDirectionOutput_c,    /* Port E, Pin 2 */ \
                                                 gPinDirectionOutput_c,    /* Port E, Pin 1 */ \
                                                 gPinDirectionOutput_c     /* Port E, Pin 0 */ \
                                               );\
                            PTEDS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port E, Pin 7 */ \
                                                 gPinDriveStrengthLow,     /* Port E, Pin 6 */ \
                                                 gPinDriveStrengthLow,     /* Port E, Pin 5 */ \
                                                 gPinDriveStrengthLow,     /* Port E, Pin 4 */ \
                                                 gPinDriveStrengthLow,     /* Port E, Pin 3 */ \
                                                 gPinDriveStrengthLow,     /* Port E, Pin 2 */ \
                                                 gPinDriveStrengthLow,     /* Port E, Pin 1 */ \
                                                 gPinDriveStrengthLow      /* Port E, Pin 0 */ \
                                               );                                               

  // Port F - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_F    PTFD  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port F, Pin 7 */ \
                                                 gPinLevelLow_c,           /* Port F, Pin 6 */ \
                                                 gPinLevelLow_c,           /* Port F, Pin 5 */ \
                                                 gPinLevelLow_c,           /* Port F, Pin 4 */ \
                                                 gPinLevelLow_c,           /* Port F, Pin 3 */ \
                                                 gPinLevelLow_c,           /* Port F, Pin 2 */ \
                                                 gPinLevelLow_c,           /* Port F, Pin 1 */ \
                                                 gPinLevelLow_c            /* Port F, Pin 0 */ \
                                              );\
                            PTFPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port F, Pin 7 */ \
                                                 gPinPullUpDisable_c,      /* Port F, Pin 6 */ \
                                                 gPinPullUpDisable_c,      /* Port F, Pin 5 */ \
                                                 gPinPullUpDisable_c,      /* Port F, Pin 4 */ \
                                                 gPinPullUpDisable_c,      /* Port F, Pin 3 */ \
                                                 gPinPullUpDisable_c,      /* Port F, Pin 2 */ \
                                                 gPinPullUpDisable_c,      /* Port F, Pin 1 */ \
                                                 gPinPullUpDisable_c       /* Port F, Pin 0 */ \
                                              );\
                            PTFDD = PORT_SETUP(\
                                                 gPinDirectionOutput_c,    /* Port F, Pin 7 */ \
                                                 gPinDirectionOutput_c,    /* Port F, Pin 6 */ \
                                                 gPinDirectionOutput_c,    /* Port F, Pin 5 */ \
                                                 gPinDirectionOutput_c,    /* Port F, Pin 4 */ \
                                                 gPinDirectionOutput_c,    /* Port F, Pin 3 */ \
                                                 gPinDirectionOutput_c,    /* Port F, Pin 2 */ \
                                                 gPinDirectionOutput_c,    /* Port F, Pin 1 */ \
                                                 gPinDirectionOutput_c     /* Port F, Pin 0 */ \
                                               );\
                            PTFDS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port F, Pin 7 */ \
                                                 gPinDriveStrengthLow,     /* Port F, Pin 6 */ \
                                                 gPinDriveStrengthLow,     /* Port F, Pin 5 */ \
                                                 gPinDriveStrengthLow,     /* Port F, Pin 4 */ \
                                                 gPinDriveStrengthLow,     /* Port F, Pin 3 */ \
                                                 gPinDriveStrengthLow,     /* Port F, Pin 2 */ \
                                                 gPinDriveStrengthLow,     /* Port F, Pin 1 */ \
                                                 gPinDriveStrengthLow      /* Port F, Pin 0 */ \
                                               );   

  // Port G - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_G    PTGD  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port G, Pin 7 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 6 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 5 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 4 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 3 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 2 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 1 */ \
                                                 gPinLevelLow_c            /* Port G, Pin 0 */ \
                                              );\
                            PTGPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port G, Pin 7 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 6 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 5 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 4 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 3 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 2 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 1 */ \
                                                 gPinPullUpDisable_c       /* Port G, Pin 0 */ \
                                              );\
                            PTGDD = PORT_SETUP(\
                                                 gPinDirectionOutput_c,    /* Port G, Pin 7 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 6 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 5 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 4 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 3 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 2 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 1 */ \
                                                 gPinDirectionOutput_c     /* Port G, Pin 0 */ \
                                               );\
                            PTGDS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port G, Pin 7 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 6 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 5 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 4 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 3 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 2 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 1 */ \
                                                 gPinDriveStrengthLow      /* Port G, Pin 0 */ \
                                               );  

  // Port h - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_H    PTHD  = PORT_SETUP(\
                                                 gPinLevelLow_c,           /* Port G, Pin 7 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 6 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 5 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 4 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 3 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 2 */ \
                                                 gPinLevelLow_c,           /* Port G, Pin 1 */ \
                                                 gPinLevelLow_c            /* Port G, Pin 0 */ \
                                              );\
                            PTHPE = PORT_SETUP(\
                                                 gPinPullUpDisable_c,      /* Port G, Pin 7 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 6 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 5 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 4 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 3 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 2 */ \
                                                 gPinPullUpDisable_c,      /* Port G, Pin 1 */ \
                                                 gPinPullUpDisable_c       /* Port G, Pin 0 */ \
                                              );\
                            PTHDD = PORT_SETUP(\
                                                 gPinDirectionOutput_c,    /* Port G, Pin 7 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 6 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 5 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 4 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 3 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 2 */ \
                                                 gPinDirectionOutput_c,    /* Port G, Pin 1 */ \
                                                 gPinDirectionOutput_c     /* Port G, Pin 0 */ \
                                               );\
                            PTHDS = PORT_SETUP(\
                                                 gPinDriveStrengthLow,     /* Port G, Pin 7 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 6 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 5 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 4 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 3 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 2 */ \
                                                 gPinDriveStrengthLow,     /* Port G, Pin 1 */ \
                                                 gPinDriveStrengthLow      /* Port G, Pin 0 */ \
                                               );  


#endif /* _TARGET_QE128_EVB_ */