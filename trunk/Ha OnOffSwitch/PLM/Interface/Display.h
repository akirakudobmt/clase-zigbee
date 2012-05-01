/************************************************************************************
* This header file is for LCD Driver Interface.
*
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#ifndef _Display_H_
#define _Display_H_
#include "AppToPlatformConfig.h"

/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/

#define gMAX_LCD_CHARS_c 16

#if ((gTargetMC13213NCB_d == 1) || (gTargetAxiomGB60_d == 1) || (gTargetQE128EVB_d == 1) || (gTargetMC1323xRCM_d == 1))
#define gLCDSupported_d TRUE
#else
#define gLCDSupported_d TRUE
#endif

#if (gLCDSupported_d == 1)

#if (gTargetAxiomGB60_d == 1)
/* I/O port YY data direction register */
/* Setup Bits 6-7 as outputs (EN & RS) (PTEDD) */
#define gLCD_SETUP_EN_c   6,0x13
#define gLCD_SETUP_RS_c   7,0x13
/* I/O port YY data register */
#define gLCD_EN_c         6,0x10 /* clear LCD EN */
#define gLCD_RS_c         7,0x10  /* set LCD RS */


/* I/O port XX data direction register
   Setup the XX Port (4-7 data bits, 3 R/W )
   data is output (default), r/w is output */
#define gLCD_DATA_SETUP_bit3_c  3,0x47
#define gLCD_DATA_SETUP_bit4_c  4,0x47
#define gLCD_DATA_SETUP_bit5_c  5,0x47
#define gLCD_DATA_SETUP_bit6_c  6,0x47
#define gLCD_DATA_SETUP_bit7_c  7,0x47
/* I/O port XX data register */
#define gLCD_DATAbit3_c         7,0x44
#define gLCD_DATAbit2_c         6,0x44
#define gLCD_DATAbit1_c         5,0x44
#define gLCD_DATAbit0_c         4,0x44
#define gLCD_RW_c               3,0x44	 /* LCD R/W */

#endif /* gTargetAxiomGB60_d */

#if (gTargetMC13213NCB_d == 1)
/* I/O port YY data direction register */
/* Setup Bits 6-7 as outputs (EN & RS) (PTEDD) */
#define gLCD_SetupEnable_c     0,0x07
#define gLCD_SETUP_EN_c   1,0x07
#define gLCD_SETUP_RS_c   2,0x07
/* I/O port YY data register */
#define gLCD_Enable_c     0,0x04
#define gLCD_EN_c         1,0x04 /* clear LCD EN */
#define gLCD_RS_c         2,0x04  /* set LCD RS */


/* I/O port XX data direction register
   Setup the XX Port (4-7 data bits, 3 R/W )
   data is output (default), r/w is output */
#define gLCD_DATA_SETUP_bit3_c  3,0x07
#define gLCD_DATA_SETUP_bit4_c  4,0x07
#define gLCD_DATA_SETUP_bit5_c  5,0x07
#define gLCD_DATA_SETUP_bit6_c  6,0x07
#define gLCD_DATA_SETUP_bit7_c  7,0x07
/* I/O port XX data register */
#define gLCD_DATAbit3_c         7,0x04
#define gLCD_DATAbit2_c         6,0x04
#define gLCD_DATAbit1_c         5,0x04
#define gLCD_DATAbit0_c         4,0x04
#define gLCD_RW_c               3,0x04	 /* LCD R/W */

#endif /* gTargetMC13213NCB_d */

#if (gTargetQE128EVB_d == 1)
/* I/O port YY data direction register */
/* Setup Bits 5-7 as outputs (RS ,RW & EN) (PTDDD) */
#define gLCD_SETUP_RS_c         5,0x07
#define gLCD_SETUP_EN_c         6,0x07
#define gLCD_SETUP_RW_c         7,0x07
/* I/O port YY data register */
#define gLCD_RS_c         5,0x06 /* set LCD RS */
#define gLCD_EN_c         6,0x06 /* clear LCD EN */
#define gLCD_RW_c         7,0x06 /* LCD R/W */


/* I/O port XX data direction register
   Setup the XX Port (4-7 data bits)
   data is output (default) */
#define gLCD_DATA_SETUP_bit4_c  4,0x0B
#define gLCD_DATA_SETUP_bit5_c  5,0x0B
#define gLCD_DATA_SETUP_bit6_c  6,0x0B
#define gLCD_DATA_SETUP_bit7_c  7,0x0B
/* I/O port XX data register */
#define gLCD_DATAbit4_c         4,0x0A
#define gLCD_DATAbit5_c         5,0x0A
#define gLCD_DATAbit6_c         6,0x0A
#define gLCD_DATAbit7_c         7,0x0A

#endif /* gTargetQE128EVB_d */


/* ASM code for HCS08 */
#if ((gTargetAxiomGB60_d == 1) || (gTargetMC13213NCB_d == 1))

/* This function setup Bits 6-7 as outputs (EN & RS) (PTEDD) */
/* Enable Bit 0 on PORTB when NCB target */ 
#if (gTargetMC13213NCB_d == 1)
#define Setup_EN_RS   { asm bset   gLCD_SETUP_EN_c; \
                        asm bset   gLCD_SETUP_RS_c; \
                        asm bset   gLCD_SetupEnable_c;\
                        asm bclr   gLCD_Enable_c;\
                      }
#endif

/* This function setup Bits 6-7 as outputs (EN & RS) (PTEDD) */
#if (gTargetAxiomGB60_d == 1)
#define Setup_EN_RS   { asm bset   gLCD_SETUP_EN_c; \
                        asm bset   gLCD_SETUP_RS_c; \
                      }
#endif

/*  Setup the XX Port (4-7 data bits, 3 R/W ) (PTGDD) */
#define SetupDataBit  { asm bset   gLCD_DATA_SETUP_bit3_c;\
                        asm bset   gLCD_DATA_SETUP_bit4_c;\
                        asm bset   gLCD_DATA_SETUP_bit5_c;\
                        asm bset   gLCD_DATA_SETUP_bit6_c;\
                        asm bset   gLCD_DATA_SETUP_bit7_c;\
                      }
                      
/*  Initialize data port */
#define InitDataPort  { asm bclr   gLCD_DATAbit3_c;\
                        asm bclr   gLCD_DATAbit2_c;\
                        asm bclr   gLCD_DATAbit1_c;\
                        asm bclr   gLCD_DATAbit0_c;\
                      }
/* Setup the R/W for writing (PTGD) Initialize EN and RS to 0 */
#define Setup_R_W_Write { asm bclr   gLCD_RW_c;\
                          asm bclr   gLCD_EN_c;\
                        }
/* Initialize EN and RS to 0 */
#define Init_EN_RS  { asm bclr   gLCD_EN_c;\
                      asm bclr   gLCD_RS_c;\
                    }
/* Set display ready for clearing */
#define SetupLCDClearBit { asm bclr   gLCD_RW_c;\
                           asm bclr   gLCD_RS_c;\
                           asm bset   gLCD_EN_c;\
                         }
/* Toggle LCD EN */
#define LCDToggleEN     { asm bset   gLCD_RS_c;\
                          asm bclr   gLCD_RS_c;\
                        }
/* Set LCD Data bit 3 */
#define SetLCDDataBit3  { asm bset gLCD_DATAbit3_c; }
/* Set LCD Data bit 2 */
#define SetLCDDataBit2  { asm bset gLCD_DATAbit2_c; }
/* Set LCD Data bit 1 */
#define SetLCDDataBit1  { asm bset gLCD_DATAbit1_c; }
/* Set LCD Data bit 0 */
#define SetLCDDataBit0  { asm bset gLCD_DATAbit0_c; }
/* Clear LCD Data bit 3 */
#define ClearLCDDataBit3  { asm bclr gLCD_DATAbit3_c; }
/* Clear LCD Data bit 2 */
#define ClearLCDDataBit2  { asm bclr gLCD_DATAbit2_c; }
/* Clear LCD Data bit 1 */
#define ClearLCDDataBit1  { asm bclr gLCD_DATAbit1_c; }
/* Clear LCD Data bit 0 */
#define ClearLCDDataBit0  { asm bclr gLCD_DATAbit0_c; }

#endif    /* defined TARGET_AXIOM_GB60 || defined TARGET_TOROWEAP */


#if (gTargetQE128EVB_d == 1)

/* This function setup Bits 5-7 as outputs (RS, EN & R_W) (PTDDD) */
#define Setup_EN_RS_R_W{ asm bset   gLCD_SETUP_EN_c; \
                         asm bset   gLCD_SETUP_RS_c; \
                         asm bset   gLCD_SETUP_RW_c; \
                       }

/*  Setup the XX Port (4-7 data bits) (PTFDD) */
#define SetupDataBit  { asm bset   gLCD_DATA_SETUP_bit4_c;\
                        asm bset   gLCD_DATA_SETUP_bit5_c;\
                        asm bset   gLCD_DATA_SETUP_bit6_c;\
                        asm bset   gLCD_DATA_SETUP_bit7_c;\
                      }
/*  Initialize data port */
#define InitDataPort  { asm bclr   gLCD_DATAbit7_c;\
                        asm bclr   gLCD_DATAbit6_c;\
                        asm bclr   gLCD_DATAbit5_c;\
                        asm bclr   gLCD_DATAbit4_c;\
                      }
/* Setup the R/W for writing (PTDD) Initialize EN and RS to 0 */
#define Setup_R_W_Write { asm bclr   gLCD_RW_c;\
                          asm bclr   gLCD_RS_c;\
                        }

/* Write data into internal RAM */
#define SetupLCDClearBit{ asm bclr   gLCD_RW_c;\
                          asm bset   gLCD_RS_c;\
                          asm bclr   gLCD_EN_c;\
                         }
                        
                      
/* Toggle LCD EN */
#define LCDToggleEN     { asm bset   gLCD_EN_c;\
                          asm bclr   gLCD_EN_c;\
                        }
                        
/* Set LCD Data bit 3 */
#define SetLCDDataBit3  { asm bset gLCD_DATAbit7_c; }
/* Set LCD Data bit 2 */
#define SetLCDDataBit2  { asm bset gLCD_DATAbit6_c; }
/* Set LCD Data bit 1 */
#define SetLCDDataBit1  { asm bset gLCD_DATAbit5_c; }
/* Set LCD Data bit 0 */
#define SetLCDDataBit0  { asm bset gLCD_DATAbit4_c; }
/* Clear LCD Data bit 3 */
#define ClearLCDDataBit3  { asm bclr gLCD_DATAbit7_c; }
/* Clear LCD Data bit 2 */
#define ClearLCDDataBit2  { asm bclr gLCD_DATAbit6_c; }
/* Clear LCD Data bit 1 */
#define ClearLCDDataBit1  { asm bclr gLCD_DATAbit5_c; }
/* Clear LCD Data bit 0 */
#define ClearLCDDataBit0  { asm bclr gLCD_DATAbit4_c; }

#endif /* defined TARGET_QE128EVB */


	#if(gTargetMC1323xRCM_d == 1)
		#include "Display_MC1323x_RCM.h"
		
		/* Define for compatibility */
		#define LCD_ClearDisplay()              MC1323xRCM_LCD_ClearDisplay(NULL)
		#define LCD_Init()                      (void)MC1323xRCM_LCD_Init(NULL)
		#define LCD_WriteString(line, pstr) 	(void)MC1323xRCM_LCD_WriteString(line, pstr, NULL)
	  /* Functions which are not available for MC1323xRCM target */	
		#define LCD_WriteStringValue(pstr, value, line, numberFormat)
		#define LCD_WriteBytes(pstr, value, line, length)
		
	#else /* gTargetMC1323x_RCMd == 0 */
		/******************************************************************************
		*******************************************************************************
		* Public type definitions
		*******************************************************************************
		******************************************************************************/
		typedef enum {
		    gLCD_HexFormat_c,
		    gLCD_DecFormat_c
		}LCD_t;

		/* a list of all hex digits 0-F */
		extern const char gaHexValue[];

		/******************************************************************************
		*******************************************************************************
		* Public prototypes
		*******************************************************************************
		******************************************************************************/
		/******************************************************************************
		* This function clear the display
		*
		* Interface assumptions:
		*
		*
		* Return value:
		* None
		*
		******************************************************************************/
		extern void LCD_ClearDisplay
		  (
		  void
		  );

		/******************************************************************************
		* This function initialize the display
		*
		* Interface assumptions:
		*
		*
		* Return value:
		* None
		*
		******************************************************************************/
		extern void LCD_Init
		  (
		  void
		  );

		/******************************************************************************
		* This function writes a string to the display
		*
		* Interface assumptions:
		*
		*
		* Return value:
		* None
		*
		*
		******************************************************************************/
		extern void LCD_WriteString
		  (
		  uint8_t line,  /* IN: Line in display */
		  uint8_t *pstr	 /* IN: Pointer to text string */
		  );

		/******************************************************************************
		* This function write a string and a value in decimal or hexdecimal
		* to the display
		*
		* Interface assumptions:
		*
		*
		* Return value:
		* None
		*
		*
		******************************************************************************/
		extern void LCD_WriteStringValue
		  (
		  uint8_t *pstr, 		 /* IN: Pointer to text string */
		  uint16_t value, 	 /* IN: Value */
		  uint8_t line, 		 /* IN: Line in display. */ 
		  LCD_t numberFormat /* IN: Value to show in HEX or DEC */
		  );

		/*-------------------- LCD_WriteBytes --------------------
			This functions allows to write raw bytes to the LCD, the maximum number of bytes
			capable per line is 8, this functions transfors every hex simbol in a byte to a char.

			IN: The pointer to the label to print with the bytes.
			IN: The bytes to print.
			IN: The line in the LCD where the bytes with the label.
			IN: The number of bytes to print in the LCD.

			OUT: NONE
		*/
		extern void LCD_WriteBytes
		(
			uint8_t   *pstr,   /* IN: The pointer to the label to print with the bytes. */
			uint8_t   *value,  /* IN: IN: The bytes to print. */
			uint8_t   line,    /* IN: The line in the LCD where the bytes with the label. */
			uint8_t   length   /* IN: The number of bytes to print in the LCD. */
		);
	#endif
	
#else   /* (gLCDSupported_d == 1) */
  #define LCD_Init()
  
  #define LCD_ClearDisplay()  
  #define LCD_WriteString(line, pstr)
  #define LCD_WriteStringValue(pstr, value, line, display)
  #define LCD_WriteBytes(pstr, value, line, length)  

#endif  /* (gLCDSupported_d == 1) */
/******************************************************************************
*******************************************************************************
* Public Memory Declarations
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
/* None */
 
#endif  /* _DISPLAY_H_ */
