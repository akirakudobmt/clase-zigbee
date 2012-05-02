/************************************************************************************
* This file contains the Freescale HCS08 MCU register map for:
* - MC908HCS08QE128
*
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/
#ifndef _IOCONFIG_S08QE128_H_
#define _IOCONFIG_S08QE128_H_

#define     PTAD        (*((volatile unsigned char *)(0x0000)))	/* I/O port A data register */
#define     PTAPE       (*((volatile unsigned char *)(0x1840)))	/* I/O port A pull-up enable register */
#define     PTASE       (*((volatile unsigned char *)(0x1841)))	/* I/O port A slew rate enable register */
#define     PTADS       (*((volatile unsigned char *)(0x1842)))	/* I/O port A drive strength register */
#define     PTADD       (*((volatile unsigned char *)(0x0001)))	/* I/O port A data direction register */

#define     PTBD        (*((volatile unsigned char *)(0x0002))) /* I/O port B data register */
#define     PTBPE       (*((volatile unsigned char *)(0x1844))) /* I/O port B pull-up enable register */
#define     PTBSE       (*((volatile unsigned char *)(0x1845))) /* I/O port B slew rate enable register */
#define     PTBDS       (*((volatile unsigned char *)(0x1846)))	/* I/O port B drive strength register */
#define     PTBDD       (*((volatile unsigned char *)(0x0003))) /* I/O port B data direction register */

#define     PTCD        (*((volatile unsigned char *)(0x0004))) /* I/O port C data register */	
#define     PTCPE       (*((volatile unsigned char *)(0x1848)))	/* I/O port C pull-up enable register */
#define     PTCSE       (*((volatile unsigned char *)(0x1849)))	/* I/O port C slew rate enable register */
#define     PTCDS       (*((volatile unsigned char *)(0x184A)))	/* I/O port C drive strength register */
#define     PTCDD       (*((volatile unsigned char *)(0x0005))) /* I/O port C data direction register */
#define     PTCSET      (*((volatile unsigned char *)(0x1878))) /* I/O port C set register */
#define     PTCCLR      (*((volatile unsigned char *)(0x187A))) /* I/O port C clear register */
#define     PTCTOG      (*((volatile unsigned char *)(0x187C))) /* I/O port C toggle register */

#define     PTDD        (*((volatile unsigned char *)(0x0006)))	/* I/O port D data register */
#define     PTDPE       (*((volatile unsigned char *)(0x184C))) /* I/O port D pull-up enable register */
#define     PTDSE       (*((volatile unsigned char *)(0x184D)))	/* I/O port D slew rate enable register */
#define     PTDDS       (*((volatile unsigned char *)(0x184E)))	/* I/O port D drive strength register */
#define     PTDDD       (*((volatile unsigned char *)(0x0007)))	/* I/O port D data direction register */

#define     PTED        (*((volatile unsigned char *)(0x0008)))	/* I/O port E data register */	
#define     PTEPE       (*((volatile unsigned char *)(0x1850)))	/* I/O port E pull-up enable register */
#define     PTESE       (*((volatile unsigned char *)(0x1851)))	/* I/O port E slew rate enable register */
#define     PTEDS       (*((volatile unsigned char *)(0x1852)))	/* I/O port E drive strength register */
#define     PTEDD       (*((volatile unsigned char *)(0x0009)))	/* I/O port E data direction register */
#define     PTESET      (*((volatile unsigned char *)(0x1879))) /* I/O port E set register */
#define     PTECLR      (*((volatile unsigned char *)(0x187B))) /* I/O port E clear register */
#define     PTETOG      (*((volatile unsigned char *)(0x187D))) /* I/O port E toggle register */

#define     PTFD        (*((volatile unsigned char *)(0x000A)))	/* I/O port F data register */
#define     PTFPE       (*((volatile unsigned char *)(0x1854)))	/* I/O port F pull-up enable register */
#define     PTFSE       (*((volatile unsigned char *)(0x1855)))	/* I/O port F slew rate enable register */
#define     PTFDS       (*((volatile unsigned char *)(0x1856)))	/* I/O port F drive strength register */
#define     PTFDD       (*((volatile unsigned char *)(0x000B)))	/* I/O port F data direction register */

#define     PTGD        (*((volatile unsigned char *)(0x001C)))	/* I/O port G data register */
#define     PTGPE       (*((volatile unsigned char *)(0x1858)))	/* I/O port G pull-up enable register */
#define     PTGSE       (*((volatile unsigned char *)(0x1859)))	/* I/O port G slew rate enable register */
#define     PTGDS       (*((volatile unsigned char *)(0x185A)))	/* I/O port G drive strength register */
#define     PTGDD       (*((volatile unsigned char *)(0x001D)))	/* I/O port G data direction register */

#define     PTHD        (*((volatile unsigned char *)(0x001E)))	/* I/O port H data register */
#define     PTHPE       (*((volatile unsigned char *)(0x185C)))	/* I/O port H pull-up enable register */
#define     PTHSE       (*((volatile unsigned char *)(0x185D)))	/* I/O port H slew rate enable register */
#define     PTHDS       (*((volatile unsigned char *)(0x185E)))	/* I/O port H drive strength register */
#define     PTHDD       (*((volatile unsigned char *)(0x001F)))	/* I/O port H data direction register */

#define     PTJD        (*((volatile unsigned char *)(0x002E)))	/* I/O port J data register */
#define     PTJPE       (*((volatile unsigned char *)(0x1860)))	/* I/O port J pull-up enable register */
#define     PTJSE       (*((volatile unsigned char *)(0x1861)))	/* I/O port J slew rate enable register */
#define     PTJDS       (*((volatile unsigned char *)(0x1862)))	/* I/O port J drive strength register */
#define     PTJDD       (*((volatile unsigned char *)(0x002F)))	/* I/O port J data direction register */

#define     KBISC       (*((volatile unsigned char *)(0x000C)))	/* KBI1 status and control register */
#define     KBIPE       (*((volatile unsigned char *)(0x000D)))	/* KBI1 pin enable controls */
#define     KBIES       (*((volatile unsigned char *)(0x000E)))	/* KBI1 edge select register */

#define     KBI2SC      (*((volatile unsigned char *)(0x003C)))	/* KBI2 status and control register */
#define     KBI2PE      (*((volatile unsigned char *)(0x003D)))	/* KBI2 pin enable controls */
#define     KBI2ES      (*((volatile unsigned char *)(0x003E)))	/* KBI2 edge select register */

#define     IRQSC       (*((volatile unsigned char *)(0x000F)))	/* Interrupt Status and Control Register */

#define     ADCSC1      (*((volatile unsigned char *)(0x0010)))	/* ADC Status and Control Register 1 */
#define     ADCSC2      (*((volatile unsigned char *)(0x0011)))	/* ADC Status and Control Register 2 */
#define     ADCRH       (*((volatile unsigned char *)(0x0012)))	/* ADC Data Result High Register */
#define     ADCRL       (*((volatile unsigned char *)(0x0013)))	/* ADC Data Result Low Register */
#define     ADCCVH      (*((volatile unsigned char *)(0x0014)))	/* ADC Compare Value High Register */
#define     ADCCVL      (*((volatile unsigned char *)(0x0015)))	/* ADC Compare Value Low Register */
#define     ADCCFG      (*((volatile unsigned char *)(0x0016)))	/* ADC Configuration Register */
#define     ADCCTL1     (*((volatile unsigned char *)(0x0017)))	/* ADC Pin Control 1 Register */
#define     ADCCTL2     (*((volatile unsigned char *)(0x0018)))	/* ADC Pin Control 2 Register */
#define     ADCCTL3     (*((volatile unsigned char *)(0x0019)))	/* ADC Pin Control 3 Register */

#define     ACMP1SC     (*((volatile unsigned char *)(0x001A)))	/* Analog Comparator 1 Status and Control Register */

#define     ACMP2SC     (*((volatile unsigned char *)(0x001B)))	/* Analog Comparator 2 Status and Control Register */

#define     SCI1BDH     (*((volatile unsigned char *)(0x0020)))	/* SCI1 baud rate register (high) */
#define     SCI1BDL     (*((volatile unsigned char *)(0x0021)))	/* SCI1 baud rate register (low byte) */
#define     SCI1C1      (*((volatile unsigned char *)(0x0022)))	/* SCI1 control register 1 */
#define     SCI1C2      (*((volatile unsigned char *)(0x0023)))	/* SCI1 control register 2 */
#define     SCI1S1      (*((volatile unsigned char *)(0x0024)))
#define     SCI1S2      (*((volatile unsigned char *)(0x0025)))
#define     SCI1C3      (*((volatile unsigned char *)(0x0026)))	/* SCI1 control register 3 */
#define     SCI1D       (*((volatile unsigned char *)(0x0027)))	/* SCI1 data register (low byte) */

#define     SCI2BDH     (*((volatile unsigned char *)(0x1870)))	/* SCI2 baud rate register (high) */
#define     SCI2BDL     (*((volatile unsigned char *)(0x1871)))	/* SCI2 baud rate register (low byte) */
#define     SCI2C1      (*((volatile unsigned char *)(0x1872)))	/* SCI2 control register 1 */
#define     SCI2C2      (*((volatile unsigned char *)(0x1873)))	/* SCI2 control register 2 */
#define     SCI2S1      (*((volatile unsigned char *)(0x1874)))	/* SCI2 status register 1 */
#define     SCI2S2      (*((volatile unsigned char *)(0x1875)))	/* SCI2 status register 2 */
#define     SCI2C3      (*((volatile unsigned char *)(0x1876)))	/* SCI2 control register 3 */
#define     SCI2D       (*((volatile unsigned char *)(0x1877)))	/* SCI2 data register (low byte) */

#define     SPIC1       (*((volatile unsigned char *)(0x0028)))	/* SPI1 control register 1 */
#define     SPIC2       (*((volatile unsigned char *)(0x0029)))	/* SPI1 control register 2 */
#define     SPIBR       (*((volatile unsigned char *)(0x002A)))	/* SPI1 baud rate select */
#define     SPIS        (*((volatile unsigned char *)(0x002B)))	/* SPI1 status register */
#define     SPID        (*((volatile unsigned char *)(0x002D)))	/* SPI1 data register */
                      
#define     SPI2C1      (*((volatile unsigned char *)(0x1838)))	/* SPI2 control register 1 */
#define     SPI2C2      (*((volatile unsigned char *)(0x1839)))	/* SPI2 control register 2 */
#define     SPI2BR      (*((volatile unsigned char *)(0x183A)))	/* SPI2 baud rate select */
#define     SPI2S       (*((volatile unsigned char *)(0x183B)))	/* SPI2 status register */
#define     SPI2D       (*((volatile unsigned char *)(0x183D)))	/* SPI2 data register */

#define     IIC1A       (*((volatile unsigned char *)(0x0030)))	/* IIC1 address register */
#define     IIC1F       (*((volatile unsigned char *)(0x0031)))	/* IIC1 frequency divider register */
#define     IIC1C1      (*((volatile unsigned char *)(0x0032)))	/* IIC1 control register 1 */
#define     IIC1S       (*((volatile unsigned char *)(0x0033)))	/* IIC1 status register */
#define     IIC1D       (*((volatile unsigned char *)(0x0034)))	/* IIC1 data I/O register */
#define     IIC1C2      (*((volatile unsigned char *)(0x0035)))	/* IIC1 control register 2 */

#define     IIC2A       (*((volatile unsigned char *)(0x1868)))	/* IIC2 address register */
#define     IIC2F       (*((volatile unsigned char *)(0x1869)))	/* IIC2 frequency divider register */
#define     IIC2C1      (*((volatile unsigned char *)(0x186A)))	/* IIC2 control register 1 */
#define     IIC2S       (*((volatile unsigned char *)(0x186B)))	/* IIC2 status register */
#define     IIC2D       (*((volatile unsigned char *)(0x186C)))	/* IIC2 data I/O register */
#define     IIC2C2      (*((volatile unsigned char *)(0x186D)))	/* IIC2 control register 2 */

#define     ICSC1       (*((volatile unsigned char *)(0x0038)))	/* ICS Control Register 1 */
#define     ICSC2       (*((volatile unsigned char *)(0x0039)))	/* ICS Control Register 2 */
#define     ICSTRM      (*((volatile unsigned char *)(0x003A)))	/* ICS Trim Register */
#define     ICSSC       (*((volatile unsigned char *)(0x003B)))	/* ICS Status and Control Register */

#define     TPM1SC      (*((volatile unsigned char *)(0x0040)))	/* TPM1 status and control register */
#define     TPM1CNTH    (*((volatile unsigned char *)(0x0041)))	/* TPM1 counter (high half) */
#define     TPM1CNTL    (*((volatile unsigned char *)(0x0042)))	/* TPM1 counter (low half) */
#define     TPM1MODH    (*((volatile unsigned char *)(0x0043)))	/* TPM1 modulo register (high half) */
#define     TPM1MODL    (*((volatile unsigned char *)(0x0044)))	/* TPM1 modulo register(low half) */
#define     TPM1C0SC    (*((volatile unsigned char *)(0x0045)))	/* TPM1 channel 0 status and control */
#define     TPM1C0VH    (*((volatile unsigned char *)(0x0046)))	/* TPM1 channel 0 value register (high) */
#define     TPM1C0VL    (*((volatile unsigned char *)(0x0047)))	/* TPM1 channel 0 value register (low) */
#define     TPM1C1SC    (*((volatile unsigned char *)(0x0048)))	/* TPM1 channel 1 status and control */
#define     TPM1C1VH    (*((volatile unsigned char *)(0x0049)))	/* TPM1 channel 1 value register (high) */
#define     TPM1C1VL    (*((volatile unsigned char *)(0x004A)))	/* TPM1 channel 1 value register (low) */
#define     TPM1C2SC    (*((volatile unsigned char *)(0x004B)))	/* TPM1 channel 2 status and control */
#define     TPM1C2VH    (*((volatile unsigned char *)(0x004C)))	/* TPM1 channel 2 value register (high) */
#define     TPM1C2VL    (*((volatile unsigned char *)(0x004D)))	/* TPM1 channel 2 value register (low) */

#define     TPM2SC      (*((volatile unsigned char *)(0x0050)))	/* TPM2 status and control register */
#define     TPM2CNTH    (*((volatile unsigned char *)(0x0051)))	/* TPM2 counter (high half) */
#define     TPM2CNTL    (*((volatile unsigned char *)(0x0052)))	/* TPM2 counter (low half) */
#define     TPM2MODH    (*((volatile unsigned char *)(0x0053)))	/* TPM2 modulo register (high half) */
#define     TPM2MODL    (*((volatile unsigned char *)(0x0054)))	/* TPM2 modulo register(low half) */
#define     TPM2C0SC    (*((volatile unsigned char *)(0x0055)))	/* TPM2 channel 0 status and control */
#define     TPM2C0VH    (*((volatile unsigned char *)(0x0056)))	/* TPM2 channel 0 value register (high) */
#define     TPM2C0VL    (*((volatile unsigned char *)(0x0057)))	/* TPM2 channel 0 value register (low) */
#define	    TPM2C1SC	  (*((volatile unsigned char *)(0x0058))) /* TPM2 channel 1 status and control */
#define	    TPM2C1VH	  (*((volatile unsigned char *)(0x0059))) /* TPM2 channel 1 value register (high) */
#define	    TPM2C1VL	  (*((volatile unsigned char *)(0x005A))) /* TPM2 channel 1 value register (low) */
#define	    TPM2C2SC	  (*((volatile unsigned char *)(0x005B))) /* TPM2 channel 2 status and control */
#define	    TPM2C2VH	  (*((volatile unsigned char *)(0x005C))) /* TPM2 channel 2 value register (high) */
#define	    TPM2C2VL	  (*((volatile unsigned char *)(0x005D))) /* TPM2 channel 2 value register (low) */

#define     TPM3SC      (*((volatile unsigned char *)(0x0060)))	/* TPM3 status and control register */
#define     TPM3CNTH    (*((volatile unsigned char *)(0x0061)))	/* TPM3 counter (high half) */
#define     TPM3CNTL    (*((volatile unsigned char *)(0x0062)))	/* TPM3 counter (low half) */
#define     TPM3MODH    (*((volatile unsigned char *)(0x0063)))	/* TPM3 modulo register (high half) */
#define     TPM3MODL    (*((volatile unsigned char *)(0x0064)))	/* TPM3 modulo register(low half) */
#define     TPM3C0SC    (*((volatile unsigned char *)(0x0065)))	/* TPM3 channel 0 status and control */
#define     TPM3C0VH    (*((volatile unsigned char *)(0x0066)))	/* TPM3 channel 0 value register (high) */
#define     TPM3C0VL    (*((volatile unsigned char *)(0x0067)))	/* TPM3 channel 0 value register (low) */
#define	    TPM3C1SC	  (*((volatile unsigned char *)(0x0068))) /* TPM3 channel 1 status and control */
#define	    TPM3C1VH	  (*((volatile unsigned char *)(0x0069))) /* TPM3 channel 1 value register (high) */
#define	    TPM3C1VL	  (*((volatile unsigned char *)(0x006A))) /* TPM3 channel 1 value register (low) */
#define	    TPM3C2SC	  (*((volatile unsigned char *)(0x006B))) /* TPM3 channel 2 status and control */
#define	    TPM3C2VH	  (*((volatile unsigned char *)(0x006C))) /* TPM3 channel 2 value register (high) */
#define	    TPM3C2VL	  (*((volatile unsigned char *)(0x006D))) /* TPM3 channel 2 value register (low) */
#define	    TPM3C3SC	  (*((volatile unsigned char *)(0x006E))) /* TPM3 channel 3 status and control */
#define	    TPM3C3VH	  (*((volatile unsigned char *)(0x006F))) /* TPM3 channel 3 value register (high) */
#define	    TPM3C3VL	  (*((volatile unsigned char *)(0x0070))) /* TPM3 channel 3 value register (low) */
#define	    TPM3C4SC	  (*((volatile unsigned char *)(0x0071))) /* TPM3 channel 4 status and control */
#define	    TPM3C4VH	  (*((volatile unsigned char *)(0x0072))) /* TPM3 channel 4 value register (high) */
#define	    TPM3C4VL	  (*((volatile unsigned char *)(0x0073))) /* TPM3 channel 4 value register (low) */
#define	    TPM3C5SC	  (*((volatile unsigned char *)(0x0074))) /* TPM3 channel 5 status and control */
#define	    TPM3C5VH	  (*((volatile unsigned char *)(0x0075))) /* TPM3 channel 5 value register (high) */
#define	    TPM3C5VL	  (*((volatile unsigned char *)(0x0076))) /* TPM3 channel 5 value register (low) */

#define	    PPAGE   	  (*((volatile unsigned char *)(0x0078))) /* PPAGE Register */
#define     LAP2    	  (*((volatile unsigned char *)(0x0079))) /* LAP2 Register */
#define     LAP1    	  (*((volatile unsigned char *)(0x007A))) /* LAP1 Register */
#define     LAP0    	  (*((volatile unsigned char *)(0x007B))) /* LAP0 Register */
#define     LWP     	  (*((volatile unsigned char *)(0x007C))) /* LWP Register */
#define     LBP     	  (*((volatile unsigned char *)(0x007D))) /* LBP Register */
#define     LB      	  (*((volatile unsigned char *)(0x007E))) /* LB Register */
#define     LAPAB    	  (*((volatile unsigned char *)(0x007F))) /* LAPAB Register */

#define     SIMRS       (*((volatile unsigned char *)(0x1800)))	/* SIM reset status register */
#define     SIMC        (*((volatile unsigned char *)(0x1801)))	/* SIM control (BDC access only) */
#define     SIMOPT      (*((volatile unsigned char *)(0x1802)))	/* SIM options register (write once) */
#define     SIMOPT2     (*((volatile unsigned char *)(0x1803)))	/* SIM options register 2 (write once) */
#define     SIMIDH      (*((volatile unsigned char *)(0x1806)))	/* SIM Part I.D. high (read-only) */
#define     SIMIDL      (*((volatile unsigned char *)(0x1807)))	/* SIM Part I.D. low (read-only) */

#define     PMCSC1      (*((volatile unsigned char *)(0x1808)))	/* PMC Status & Control 1 */
#define     PMCSC2      (*((volatile unsigned char *)(0x1809)))	/* PMC Status & Control 2 */
#define     PMCSC3      (*((volatile unsigned char *)(0x180B)))	/* PMC Status & Control 3 */

#define     SCGC1       (*((volatile unsigned char *)(0x180E)))	/* System Clock Gating Control 1 */
#define     SCG2        (*((volatile unsigned char *)(0x180F)))	/* System Clock Gating Control 2 */


#define     DBGCAH      (*((volatile unsigned char *)(0x1810)))	/* DBG comparator register A (high) */
#define     DBGCAL      (*((volatile unsigned char *)(0x1811)))	/* DBG comparator register A (low) */
#define     DBGCBH      (*((volatile unsigned char *)(0x1812)))	/* DBG comparator register B (high) */
#define     DBGCBL      (*((volatile unsigned char *)(0x1813)))	/* DBG comparator register B (low) */
#define     DBGCCH      (*((volatile unsigned char *)(0x1814)))	/* DBG comparator register B (high) */
#define     DBGCCL      (*((volatile unsigned char *)(0x1815)))	/* DBG comparator register B (low) */
#define     DBGFH       (*((volatile unsigned char *)(0x1816)))	/* DBG FIFO register (high) */
#define     DBGFL       (*((volatile unsigned char *)(0x1817)))	/* DBG FIFO register (low) */
#define     DBGCAX      (*((volatile unsigned char *)(0x1818)))
#define     DBGCBX      (*((volatile unsigned char *)(0x1819)))
#define     DBGCCX      (*((volatile unsigned char *)(0x181A)))
#define     DBGFX       (*((volatile unsigned char *)(0x181B)))

#define     DBGC        (*((volatile unsigned char *)(0x181C)))	/* DBG control register */
#define     DBGT        (*((volatile unsigned char *)(0x181D)))	/* DBG trigger register */
#define     DBGS        (*((volatile unsigned char *)(0x181E)))	/* DBG status register */
#define     DBGCNT      (*((volatile unsigned char *)(0x181F)))

#define     FCDIV       (*((volatile unsigned char *)(0x1820)))	/* Flash/EE clock divider register */
#define     FOPT        (*((volatile unsigned char *)(0x1821)))	/* Flash/EE option register */
#define     FCNFG       (*((volatile unsigned char *)(0x1823)))	/* Flash/EE configuration register */
#define     FPROT       (*((volatile unsigned char *)(0x1824)))	/* Flash protection register */
#define     FSTAT       (*((volatile unsigned char *)(0x1825)))	/* Flash status register */
#define     FCMD        (*((volatile unsigned char *)(0x1826)))	/* Flash command register */

#define     RTCSC       (*((volatile unsigned char *)(0x1830)))	/* RTC Status and control register */
#define     RTCCNT      (*((volatile unsigned char *)(0x1831)))	/* RTC Count register */
#define     RTCMOD      (*((volatile unsigned char *)(0x1832)))	/* RTC Modulo register */

#define     NVBACKKEY   (*((volatile unsigned char *)(0xFFB0)))	/* 8-byte backdoor comparison key */
#define     NVPROT     	(*((volatile unsigned char *)(0xFFBD)))	/* NV flash protection byte */
#define     NVICSTRM    (*((volatile unsigned char *)(0xFFBE)))	/* ICS manual trim value*/
#define     NVOPT     	(*((volatile unsigned char *)(0xFFBF)))	/* NV flash options byte */

#endif /* _IOCONFIG_S08QE128_H_ */