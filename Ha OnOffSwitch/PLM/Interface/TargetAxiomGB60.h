/************************************************************************************
* This header file is for the Axiom GB 60 port mappings.
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _TARGET_AXIOM_GB_60_
#define _TARGET_AXIOM_GB_60_

#include "IoConfig.h"
////////////////////////////////////////
/////// Define LED port mapping ////////
////////////////////////////////////////
  #define LED_PORT  PTFD
  #define LED1_PIN  0x01
  #define LED2_PIN  0x02
  #define LED3_PIN  0x04
  #define LED4_PIN  0x08
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
  #define TURN_OFF_LEDS     LED_PORT |= LED_MASK;
  
    // Turn on LEDs
  #define TURN_ON_LEDS      LED_PORT &= ~LED_MASK;

//////// Sniffer LED Defines ///////////
  #define GREEN_LED_OFF     LED1OFF
  #define GREEN_LED_ON      LED1ON
  #define GREEN_LED_TOGGLE  LED1TOGGLE

  #define RED_LED_OFF       LED2OFF
  #define RED_LED_ON        LED2ON
  #define RED_LED_TOGGLE    LED2TOGGLE
////////////////////////////////////////


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
  #define SWITCH_PORT   PTAD
  #define mSWITCH1_MASK 0x10
  #define mSWITCH2_MASK 0x20
  #define mSWITCH3_MASK 0x40
  #define mSWITCH4_MASK 0x80
  #define mSWITCH_MASK  (mSWITCH1_MASK | mSWITCH2_MASK |  mSWITCH3_MASK | mSWITCH4_MASK)

  #define mSWITCH_PORT_GET  ((SWITCH_PORT & mSWITCH_MASK) ^ mSWITCH_MASK)


////////////////////////////////////////
///// Define HW port/pin mapping ///////
////////////////////////////////////////

  // Define HW pin mapping
  #define gMC1319xAttnPort     PTDD
  #define gMC1319xRxTxPort     PTDD
  #define gMC1319xResetPort    PTDD

  #define gMC1319xAttnMask_c   0x20
  #define gMC1319xRxTxMask_c   0x40
  #define gMC1319xResetMask_c  0x80

  #define gMC1319xGPIO1Port    PTBD
  #define gMC1319xGPIO2Port    PTBD
  #define gMC1319xAntSwPort    PTBD

  #define gMC1319xGPIO1Mask_c  0x10
  #define gMC1319xGPIO2Mask_c  0x20
  #define gMC1319xAntSwMask_c  0x00

  #define gMC1319xSPIPort        PTED

  #define gMC1319xSpiTxD1Mask_c  0x01
  #define gMC1319xSpiRxD1Mask_c  0x02
  #define gMC1319xSpiSsMask_c    0x04
  #define gMC1319xSpiMisoMask_c  0x08
  #define gMC1319xSpiMosiMask_c  0x10
  #define gMC1319xSpiSpsckMask_c 0x20

  // Define HW port mapping 

  // Port A - Not used by MC1319x driver. Available for application. 
  #define  mSETUP_PORT_A    PTAD  = 0x00;\
                            PTAPE = mSWITCH_MASK;\
                            PTADD = 0x00; // Set as input

  // Port B - Used by MC1319x driver. Add pins for application if needed.
  #define  mSETUP_PORT_B    PTBD  = 0x00;\
                            PTBPE = 0x00;\
                            PTBDD = (gMC1319xAntSwMask_c) & (~gMC1319xGPIO1Mask_c & ~gMC1319xGPIO2Mask_c); // Set as input

                            
  // Port C - Used by MC1319x driver. Add pins for application if needed.
  #define  mSETUP_PORT_C    PTCD  = 0x00;\
                            PTCPE = 0x00;\
                            PTCDD = 0x63;// Verify use!!!!!!!
                            
  // Port D - Used by MC1319x driver. Add pins for application if needed.
  #define  mSETUP_PORT_D    PTDD  = 0x00;\
                            PTDPE = 0x00;\
   						              PTDDD = (0x01 | gMC1319xResetMask_c | gMC1319xAttnMask_c | gMC1319xRxTxMask_c);

  // Port E - Used for MC1319x SPI access. Add pins for application if needed.
  #define  mSETUP_PORT_E    PTED  = gMC1319xSpiMosiMask_c | gMC1319xSpiSsMask_c;\
                            PTEPE = 0x00;\
                            PTEDD = (0xff | (gMC1319xSpiTxD1Mask_c  | gMC1319xSpiSsMask_c | gMC1319xSpiMosiMask_c  | gMC1319xSpiSpsckMask_c )) \
                                          & (~(gMC1319xSpiMisoMask_c | gMC1319xSpiRxD1Mask_c));

  // Port F - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_F    PTFD  = 0x00;\
                            PTFPE = 0x00;\
   						              PTFDD = LED_MASK; // Set LEDS as outputs

  // Port G - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_G    
#endif /* _TARGET_AXIOM_GB_60_ */