/************************************************************************************
* This is the source file for LCD Driver.
*
*
* (c) Copyright 2006, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/

#include "EmbeddedTypes.h"
#include "Display.h"


#if (gLCDSupported_d == 1)
/******************************************************************************
*******************************************************************************
* Public Macros
*******************************************************************************
******************************************************************************/
/* None */

/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
static void LCD_WriteChar( int8_t ch );
static void LCDWaitLong( int16_t w );
static void LCDWaitShort( int16_t w );
static void LCDLine( uint8_t line );
static void LCD_Write4bits( uint8_t bdata );
static uint8_t GetStrlen( const uint8_t *pString );

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/* Wait long defines */
#define Wait15mSec   15
#define Wait5mSec     5
#define Wait3mSec     3
#define Wait2mSec     2
#define Wait1mSec     1


/* Wait short defines */
#define Wait105uSec 7
#define Wait75uSec  5
#define Wait60uSec  4
#define Wait45uSec  3
#define Wait30uSec  2
#define Wait15uSec  1

/* Line in display */
#define LineOne 0x80
#define LineTwo 0xC0

/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/
const uint8_t gaHexValue[] = "0123456789ABCDEF";

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
/******************************************************************************
* This function initialize the display
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
#if ((gTargetAxiomGB60_d == 1) || (gTargetMC13213NCB_d == 1))
void LCD_Init
  (
  void
  )
{
  LCDWaitLong( Wait5mSec ); 
  /* This function setup Bits 6-7 as outputs (EN & RS) (PTEDD) */
  Setup_EN_RS;
  /* Setup the XX Port (4-7 data bits, 3 R/W ) (PTGDD)
     data is output (default), r/w is output */
  SetupDataBit;
  /* Initialize data port */
  InitDataPort;
  /* Setup the R/W for writing (PTGD) */
  Setup_R_W_Write;
  /* Initialize EN and RS to 0 */
  Init_EN_RS;

  /* Send the reset sequence */
  LCD_Write4bits( 0x30 );
  LCDToggleEN;

  LCDWaitLong( Wait5mSec );

  LCD_Write4bits( 0x30 );
  LCDToggleEN;

  LCDWaitShort( Wait30uSec );

  LCD_Write4bits( 0x30 );
  LCDToggleEN;

  LCDWaitShort( Wait30uSec );

  LCD_Write4bits( 0x20 );
  LCDToggleEN;

  LCDWaitShort( Wait15uSec );

  /* Function, 4 bit data length */
  LCD_Write4bits( 0x20 );
  LCDToggleEN;

  LCDWaitShort( Wait15uSec );

  /* 2 lines, 5x7 dot format */
  LCD_Write4bits( 0x80 );
  LCDToggleEN;

  LCDWaitShort( Wait60uSec );

  /* Entry Mode Inc, No Shift */
  LCD_Write4bits( 0x00 );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  LCD_Write4bits( 0x60 );
  LCDToggleEN;
  LCDWaitShort( Wait75uSec );

  /* Display ON/OFF Control - Display On, Cursor Off, Blink Off */
  LCD_Write4bits( 0x00 );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  LCD_Write4bits( 0xC0 );
  LCDToggleEN;
  LCDWaitShort( Wait75uSec );

  /* Display Clear */
  LCD_ClearDisplay();

  LCDLine(LineOne);
}

#endif


#if (gTargetQE128EVB_d == 1)

void LCD_Init
  (
  void
  )
{
   /* Wait 15 ms */
  LCDWaitLong( Wait15mSec ); 
  
  /* This function setup Bits 5-7 as outputs (RS, EN & R/W) (PTDDD) */
  Setup_EN_RS_R_W;
  /* Setup the XX Port (4-7 data bits) (PTFDD)
     data is output (default) */
  SetupDataBit;
  /* Initialize data port */
  InitDataPort; 
  
  /* Send the reset sequence */
  
  Setup_R_W_Write;
  LCDToggleEN;
  
  /* Function, 8 bit data length */
  LCD_Write4bits( 0x30 );
  LCDToggleEN;  
  
  /* Wait more than 4.1 ms */
  LCDWaitLong( Wait5mSec );
  
  /* Function, 8 bit data length */
  LCD_Write4bits( 0x30 );
  LCDToggleEN;

  /* Wait more than 100 us */
  LCDWaitShort( Wait105uSec );

  /* Function, 8 bit data length */
  LCD_Write4bits( 0x30 );           
  LCDToggleEN;
  LCDWaitShort( Wait45uSec );
  
  /* Function, 4 bit data length */
  LCD_Write4bits( 0x20 );
  LCDToggleEN;
  LCDWaitShort( Wait45uSec );
  
  /* Function, 4 bit data length */
  LCD_Write4bits( 0x20 );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  /* 2 lines, 5x8 dot format */
  LCD_Write4bits( 0x80 );
  LCDToggleEN;
  LCDWaitShort( Wait45uSec );
  
  /* Display on */
  LCD_Write4bits( 0x00 );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  LCD_Write4bits( 0xC0 );
  LCDToggleEN;
  LCDWaitShort( Wait45uSec );
  
  /* Display clear */
  LCD_Write4bits( 0x00 );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  LCD_Write4bits( 0x10 );
  LCDToggleEN;
  LCDWaitLong( Wait2mSec );
  
  /* Entry mode set */
  LCD_Write4bits( 0x00 );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  LCD_Write4bits( 0x60 );
  LCDToggleEN;
  LCDWaitShort( Wait45uSec );
  

}

#endif

/******************************************************************************
* This function writes a string to the display
*
* Interface assumptions:
* The pstr must be zero-terminated.
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteString
  (
  uint8_t line,  /* IN: Line in display */
  uint8_t *pstr	 /* Pointer to text string */
  )
{
  uint8_t len;
  uint8_t x;
  uint8_t *error = "Wrong line 1 & 2";

  if( line == 2 ) {
    LCDLine(LineTwo);
  }
  else if ( line == 1 ) {
    LCDLine(LineOne);
    }
    else {
	  LCD_ClearDisplay();
	  LCDLine(LineOne);
      pstr = error;
    }

  len = GetStrlen(pstr);
  for ( x = 0; x < len; x++ ) {
    LCD_WriteChar( pstr[x] );
  }

  /* Clear the rest of the line */
  for ( ; x < gMAX_LCD_CHARS_c; x++ ) {
    LCD_WriteChar( ' ' );
  }
}

/*-------------------- LCD_WriteBytes --------------------
	This functions allows to write raw bytes to the LCD, the maximum number of bytes
	capable per line is 8, this functions transfors every hex simbol in a byte to a char.

	IN: The pointer to the label to print with the bytes.
	IN: The bytes to print.
	IN: The line in the LCD where the bytes with the label.
	IN: The number of bytes to print in the LCD.

	OUT: NONE
*/
void LCD_WriteBytes
(
	uint8_t   *pstr,   /* IN: The pointer to the label to print with the bytes. */
	uint8_t   *value,  /* IN: IN: The bytes to print. */
	uint8_t   line,    /* IN: The line in the LCD where the bytes with the label. */
	uint8_t   length   /* IN: The number of bytes to print in the LCD. */
)
{

	uint8_t i=0,counter=0, cIndex,auxIndex;
	uint8_t aString[17];

	uint8_t  hexIndex;
	uint8_t aHex[gMAX_LCD_CHARS_c]={'S','i','z','e',' ','N','o','t',' ','V','a','l','i','d','*','*'};

	counter=0;
	while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c )
	{
		aString[counter++]=*pstr;
		pstr++;
	}
	if ((((length*2)+counter) <= gMAX_LCD_CHARS_c) && ((length*2) > 0))
	{
		for (cIndex =0,auxIndex = 0; cIndex < length; cIndex++,auxIndex+=2)
		{
			hexIndex = value[cIndex]&0xf0;
			hexIndex = hexIndex>>4;
			aHex[auxIndex] = gaHexValue[hexIndex];
			hexIndex = value[cIndex] & 0x0f;
			aHex[auxIndex + 1] = gaHexValue[hexIndex];
		}
		aHex[(length * 2)]='\0';
	}
	else
		counter = 0;

	i=0;
	while (aHex[i] != '\0' && counter <gMAX_LCD_CHARS_c )
		aString[counter++]=aHex[i++];

	aString[counter]='\0';
	LCD_WriteString( line, aString );
}

/******************************************************************************
* This function write a string and a value in decimal or hexdecimal
* to the display
*
* Interface assumptions:
* The pstr must be zero-terminated.
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteStringValue
  (
  uint8_t *pstr,     /* IN: Pointer to text string */
  uint16_t value,    /* IN: Value */
  uint8_t line, 	   /* IN: Line in display */
  LCD_t numberFormat /* IN: Value to show in HEX or DEC */
  )
{

  int16_t divDec=10000, divHex=16;
  uint8_t loop=5, i=0,counter=0, aH[6], aHex[6];
  uint8_t aDec[6], aString[17];

 if(numberFormat == gLCD_DecFormat_c) {
    if(value < 100) {
      loop = 2;
      divDec = 10;
    }
    else if(value >= 100 && value <1000) {
      loop = 3;
      divDec = 100;
    }
    else if(value >= 1000 && value <9999) {
      loop = 4;
      divDec = 1000;
    }

    for(i=0; i<loop; i++) {
      if((value/divDec)!= 0) {
        aDec[counter++] = (value/divDec) + 0x30;
        value = value % divDec;
      }
      else {
        aDec[counter++] = 0x30;
      }
      divDec = divDec/10;
    }
    aDec[counter]='\0';
    counter=0;
    while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=*pstr;
      pstr++;
    }
    i=0;
    while (aDec[i] != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=aDec[i++];
    }
    aString[counter]='\0';
    LCD_WriteString( line, aString );
  }
  else if(numberFormat == gLCD_HexFormat_c) {
    do{
      aH[i]=gaHexValue[value % divHex];
      value=value / divHex;
      i++;
    }
    while(value > 15);
    aH[i]=gaHexValue[value];
    counter=0;
    while(i > 0){
      aHex[counter++]=aH[i--];
    }

    aHex[counter++]=aH[0];
    aHex[counter]='\0';

    counter=0;
    while (*pstr != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=*pstr;
      pstr++;
    }
    i=0;
    while (aHex[i] != '\0' && counter <gMAX_LCD_CHARS_c ) {
      aString[counter++]=aHex[i++];
    }
    aString[counter]='\0';
    LCD_WriteString( line, aString );
  }
  else {
    LCD_WriteString( line, "Format unknow" );
  }
}

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/
/******************************************************************************
* This function writes a byte to the display
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_WriteChar
  (
  int8_t ch /* IN: Char */
  )
{
  SetupLCDClearBit;

  LCD_Write4bits( ch );

  LCDToggleEN;

  LCD_Write4bits( ch << 4 );

  LCDToggleEN;

  LCDWaitShort( Wait15uSec );
}

/******************************************************************************
* This function makes a long delay before next instruction to the display
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCDWaitLong
  (
  int16_t w  /* IN: Value to wait */
  )
{
  int16_t x;
  int16_t y;

  for ( y = 0; y < w; y++ ) {
    /* 1ms wait routine ((1/CLK) * W * 2047 = X s) */
    for ( x = 0; x < 0x7FF; x++ );
  }
}

/******************************************************************************
* This function makes a short delay before next instruction to the display
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCDWaitShort
  (
  int16_t w  /* IN: Value to wait */
  )
{
  int16_t x;
  int16_t y;

  for ( y = 0; y < w; y++ ) {
    for ( x = 0; x < 0x001F; x++ );
  }
}

/******************************************************************************
* This function writes in the first line in the display
*
* Interface assumptions:
* Interface assumptions:
* line: 0x80 = line 1, 0xC0 = line 2
*
* Return value:
* None
*
*
******************************************************************************/
void LCDLine
  (
  uint8_t line /* IN: Line 1 = 0x80, Line 2 = 0xC0 */
  )
{
  Setup_R_W_Write;
  /* DD Ram Address Set - 1st Digit */
  LCD_Write4bits( line );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  LCD_Write4bits( 0x00 );
  LCDToggleEN;
  LCDWaitShort( Wait60uSec );
}

/******************************************************************************
* This function clear line 1 & 2 in display
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_ClearDisplay
  (
  void
  )
{
  Setup_R_W_Write
  LCD_Write4bits( 0x00 );
  LCDToggleEN;
  LCDWaitShort( Wait15uSec );
  LCD_Write4bits( 0x10 );
  LCDToggleEN;
  LCDWaitLong( Wait3mSec );
}

/******************************************************************************
* This function write 4 Bits to display
*
* Interface assumptions:
*
*
* Return value:
* None
*
*
******************************************************************************/
void LCD_Write4bits
  (
  uint8_t bdata /* IN: Data to write in display */
  )
{
  if ( bdata & 0x80 ) {
    SetLCDDataBit3;
  }
  else {
    ClearLCDDataBit3;
  }

  if ( bdata & 0x40 ) {
    SetLCDDataBit2;
  }
  else {
    ClearLCDDataBit2;
  }

  if ( bdata & 0x20 ) {
    SetLCDDataBit1;
  }
  else {
    ClearLCDDataBit1;
  }

  if ( bdata & 0x10 ) {
    SetLCDDataBit0;
  }
  else {
    ClearLCDDataBit0;
  }
}

/******************************************************************************
* This function gets the lenght of a string and return the lenght
*
* Interface assumptions:
*
*
* Return value:
* char
*
*
******************************************************************************/
uint8_t GetStrlen
  (
  const uint8_t *pString /* IN: Pointer to text string */
  )
{
  int8_t count=0, lenght=0;

  while (*pString != '\0' && count <gMAX_LCD_CHARS_c ) {
    count++;
    lenght++;
    pString++;
  }

  /* Check boundries */
  if ( lenght > gMAX_LCD_CHARS_c ) {
    lenght = gMAX_LCD_CHARS_c;
  }

  return lenght;
}


/******************************************************************************
*******************************************************************************
* Private Debug stuff
*******************************************************************************
******************************************************************************/
/* None */
#endif  /* gLCDSupported_d */
