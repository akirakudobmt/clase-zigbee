/************************************************************************************
* This header file is for the MC1321x SRB port mappings.
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _TARGET_MC1321x_SRB_
#define _TARGET_MC1321x_SRB_
#include "IoConfig.h"

////////////////////////////////////////
/////// Define LED port mapping ////////
////////////////////////////////////////
  #define LED_PORT  PTDD
  #define LED1_PIN  0x10
  #define LED2_PIN  0x20
  #define LED3_PIN  0x40
  #define LED4_PIN  0x80
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

  #define LED5_PORT  PTCD
  #define LED5_PIN  0x10
  #define LED5_MASK  (LED5_PIN)
  
  #define LED5ON            LED5_PORT &= ~LED5_PIN;
  #define LED5OFF           LED5_PORT |= LED5_PIN;
  #define LED5TOGGLE        LED5_PORT ^= LED5_PIN;

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
  // PCB switches defines
  #define SWITCH_PORT   PTAD

#ifndef mSWITCH1_MASK  
  #define mSWITCH1_MASK 0x04
#endif  

#ifndef mSWITCH2_MASK  
  #define mSWITCH2_MASK 0x08
#endif  

#ifndef mSWITCH3_MASK  
  #define mSWITCH3_MASK 0x10
#endif  

#ifndef mSWITCH4_MASK  
  #define mSWITCH4_MASK 0x20
#endif  

  #define mSWITCH_MASK  (mSWITCH1_MASK | mSWITCH2_MASK |  mSWITCH3_MASK | mSWITCH4_MASK)
  
  #define mSWITCH_PORT_GET  ((SWITCH_PORT & mSWITCH_MASK) ^ mSWITCH_MASK)


////////////////////////////////////////
///// Define HW port/pin mapping ///////
////////////////////////////////////////
  // Define HW pin mapping
  #define gMC1319xAttnPort     PTDD
  #define gMC1319xRxTxPort     PTDD
  #define gMC1319xResetPort    PTDD
  #define gMC1319xSpiPort      PTED

  #define gMC1319xAttnMask_c    0x01
  #define gMC1319xRxTxMask_c    0x02
  #define gMC1319xResetMask_c   0x08

  #define gMC1319xGPIO1Port    PTED
  #define gMC1319xGPIO2Port    PTED
  //#define gMC1319xAntSwPort // No antenna switch port defined for Toroweap

  #define gMC1319xGPIO1Mask_c  0x80
  #define gMC1319xGPIO2Mask_c  0x40
  //#define gMC1319xAntSwMask_c // No antenna switch port defined for Toroweap

  #define gMC1319xSPIPort        PTED
  
  #define gMC1319xSpiTxD1Mask_c  0x01
  #define gMC1319xSpiRxD1Mask_c  0x02
  #define gMC1319xSpiSsMask_c    0x04
  #define gMC1319xSpiMisoMask_c  0x08
  #define gMC1319xSpiMosiMask_c  0x10
  #define gMC1319xSpiSpsckMask_c 0x20

  // Define HW port mapping (Refer to MCU IO spreadsheet or schematic for details)

  // Port A - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_A    PTAD  = 0x00;\
                            PTAPE |= mSWITCH_MASK;\
                            PTADD = 0x42; // Set as input except 2 * CTS

  // Port B - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_B    //PTBD  = 0x00;\
                            //PTBPE = 0x00;\
                            //PTBDD = 0x00; // Set as input

  // Port C - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_C    PTCD  = 0x00;\
                            PTCPE = 0x00;\
                            PTCDD = (LED5_MASK); // Set as input

  // Port D - Used by MC1319x driver. Add pins for application if needed.
  #define  mSETUP_PORT_D    PTDD  = gMC1319xRxTxMask_c;\
                            PTDPE = 0x00;\
                            PTDDD = (gMC1319xResetMask_c | gMC1319xAttnMask_c | gMC1319xRxTxMask_c |LED_MASK);
  
  // Port E - Used by MC1319x driver. Add pins for application if needed.
  #define  mSETUP_PORT_E    PTED  = gMC1319xSpiMosiMask_c | gMC1319xSpiSsMask_c;\
                            PTEPE = 0x00;\
                            PTEDD = (0xff | (gMC1319xSpiTxD1Mask_c  | gMC1319xSpiSsMask_c | gMC1319xSpiMosiMask_c  | gMC1319xSpiSpsckMask_c )) \
                                         & (~(gMC1319xSpiMisoMask_c | gMC1319xGPIO1Mask_c | gMC1319xGPIO2Mask_c | gMC1319xSpiRxD1Mask_c));

  // Port F - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_F    //PTFD  = 0x00;\
                            //PTFPE = 0x00;\
                            //PTFDD = 0x00; // Set as input

  // Port G - Not used by MC1319x driver. Available for application.
  #define mSETUP_PORT_G     //PTGD  = 0x00;\
                            //PTGPE = 0x00;\
   						              //PTGDD = 0x00;


#endif /* _TARGET_MC1321x_SRB_ */