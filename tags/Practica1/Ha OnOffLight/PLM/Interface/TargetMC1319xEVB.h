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

#ifndef _TARGET_MC1319x_EVB_
#define _TARGET_MC1319x_EVB_
#include "IoConfig.h"

////////////////////////////////////////
/////// Define LED port mapping ////////
////////////////////////////////////////
  #define LED_PORT  PTDD
  #define LED1_PIN  0x01
  #define LED2_PIN  0x02
  #define LED3_PIN  0x08
  #define LED4_PIN  0x10
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
  #define SWITCH_PORT   PTAD
  #define mSWITCH1_MASK 0x04
  #define mSWITCH2_MASK 0x08
  #define mSWITCH3_MASK 0x10
  #define mSWITCH4_MASK 0x20
  #define mSWITCH_MASK  (mSWITCH1_MASK | mSWITCH2_MASK |  mSWITCH3_MASK | mSWITCH4_MASK)

  #define mSWITCH_PORT_GET  ((SWITCH_PORT & mSWITCH_MASK) ^ mSWITCH_MASK)



////////////////////////////////////////
///// Define HW port/pin mapping ///////
////////////////////////////////////////

  // Define HW pin mapping
  #define gMC1319xAttnPort     PTCD
  #define gMC1319xRxTxPort     PTCD
  #define gMC1319xResetPort    PTCD

  #define gMC1319xAttnMask_c   0x04
  #define gMC1319xRxTxMask_c   0x08
  #define gMC1319xResetMask_c  0x10

  #define gMC1319xGPIO1Port    PTBD
  #define gMC1319xGPIO2Port    PTBD
  #define gMC1319xAntSwPort    PTBD

  #define gMC1319xGPIO1Mask_c  0x10
  #define gMC1319xGPIO2Mask_c  0x20
  #define gMC1319xAntSwMask_c  0x40

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
                            PTAPE = mSWITCH_MASK;\
                            PTADD = 0x42; // Set as input except 2 * CTS

  // Port B - Used by MC1319x driver. Add pins for application if needed.
  #define  mSETUP_PORT_B    PTBD  = 0x00;\
                            PTBPE = 0x00;\
                            PTBDD = (gMC1319xAntSwMask_c) & (~gMC1319xGPIO1Mask_c & ~gMC1319xGPIO2Mask_c); // Set AntSW as output and GPIO1+2 as input
                            
  // Port C - Used by MC1319x driver. Add pins for application if needed.
  #define  mSETUP_PORT_C    PTCD  = 0x1c;\
                            PTCPE = 0x00;\
                            PTCDD = 0xfd | (gMC1319xResetMask_c | gMC1319xAttnMask_c | gMC1319xRxTxMask_c ); // Set all allowed pins as output for power/EMC reasons (pin 1 is also output on UART)
                            
  // Port D - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_D    PTDD  = 0x1b;\
                            PTDPE = 0x00;\
                            PTDDD = 0xff; // Set all allowed pins as output for power/EMC reasons    

  // Port E - Used for MC1319x SPI access. Add pins for application if needed.
  #define  mSETUP_PORT_E    PTED  = gMC1319xSpiMosiMask_c | gMC1319xSpiSsMask_c;\
                            PTEPE = 0x00;\
                            PTEDD = (0xff | (gMC1319xSpiTxD1Mask_c  | gMC1319xSpiSsMask_c | gMC1319xSpiMosiMask_c  | gMC1319xSpiSpsckMask_c )) \
                                          & (~(gMC1319xSpiMisoMask_c | gMC1319xSpiRxD1Mask_c));

  // Port F - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_F    PTFD  = 0x00;\
                            PTFPE = 0x00;\
                            PTFDD = 0xff; // Set all allowed pins as output for power/EMC reasons

  // Port G - Not used by MC1319x driver. Available for application.
  #define  mSETUP_PORT_G    PTGD  = 0x01;\
                            PTGPE = 0x00;\
                            PTGDD = 0xfb; // Set all allowed pins as output for power/EMC reasons


#endif /* _TARGET_MC1319x_EVB_ */