/************************************************************************************
* This file contains the Freescale HCS08 MCU register map
* - MC908HCS08GB60
* - MC908HCS08GT60
* - MC908HCS08GB32
* - MC908HCS08GT32
*
*
* (c) Copyright 2005, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/
#ifndef _IOCONFIG_S08_H_
#define _IOCONFIG_S08_H_

#define     PTAD        (*((volatile unsigned char *)(0x00)))	/* I/O port A data register */
#define     PTAPE       (*((volatile unsigned char *)(0x01)))	/* I/O port A pull-up enable register */
#define     PTASE       (*((volatile unsigned char *)(0x02)))	/* I/O port A slew rate enable register */
#define     PTADD       (*((volatile unsigned char *)(0x03)))	/* I/O port A data direction register */

#define     PTBD        (*((volatile unsigned char *)(0x04)))	/* I/O port B data register */
#define     PTBPE       (*((volatile unsigned char *)(0x05)))	/* I/O port B pull-up enable register */
#define     PTBSE       (*((volatile unsigned char *)(0x06)))	/* I/O port B slew rate enable register */
#define     PTBDD       (*((volatile unsigned char *)(0x07)))	/* I/O port B data direction register */

#define     PTCD        (*((volatile unsigned char *)(0x08)))	/* I/O port C data register */	
#define     PTCPE       (*((volatile unsigned char *)(0x09)))	/* I/O port C pull-up enable register */
#define     PTCSE       (*((volatile unsigned char *)(0x0A)))	/* I/O port C slew rate enable register */
#define     PTCDD       (*((volatile unsigned char *)(0x0B)))	/* I/O port C data direction register */

#define     PTDD        (*((volatile unsigned char *)(0x0C)))	/* I/O port D data register */
#define     PTDPE       (*((volatile unsigned char *)(0x0D)))	/* I/O port D pull-up enable register */
#define     PTDSE       (*((volatile unsigned char *)(0x0E)))	/* I/O port D slew rate enable register */
#define     PTDDD       (*((volatile unsigned char *)(0x0F)))	/* I/O port D data direction register */

#define     PTED        (*((volatile unsigned char *)(0x10)))	/* I/O port E data register */	
#define     PTEPE       (*((volatile unsigned char *)(0x11)))	/* I/O port E pull-up enable register */
#define     PTESE       (*((volatile unsigned char *)(0x12)))	/* I/O port E slew rate enable register */
#define     PTEDD       (*((volatile unsigned char *)(0x13)))	/* I/O port E data direction register */

#define     IRQSC       (*((volatile unsigned char *)(0x14)))	/* Interrupt Status and Control Register */

#define     KBISC       (*((volatile unsigned char *)(0x16)))	/* KBI status and control register */
#define     KBIPE       (*((volatile unsigned char *)(0x17)))	/* KBI pin enable controls */

#define     SCI1BDH     (*((volatile unsigned char *)(0x18)))	/* SCI1 baud rate register (high) */
#define     SCI1BDL     (*((volatile unsigned char *)(0x19)))	/* SCI1 baud rate register (low byte) */
#define     SCI1C1      (*((volatile unsigned char *)(0x1A)))	/* SCI1 control register 1 */
#define     SCI1C2      (*((volatile unsigned char *)(0x1B)))	/* SCI1 control register 2 */
#define     SCI1S1      (*((volatile unsigned char *)(0x1C)))
#define     SCI1S2      (*((volatile unsigned char *)(0x1D)))
#define     SCI1C3      (*((volatile unsigned char *)(0x1E)))	/* SCI1 control register 3 */
#define     SCI1D       (*((volatile unsigned char *)(0x1F)))	/* SCI1 data register (low byte) */

#define     SCI2BDH     (*((volatile unsigned char *)(0x20)))	/* SCI2 baud rate register (high) */
#define     SCI2BDL     (*((volatile unsigned char *)(0x21)))	/* SCI2 baud rate register (low byte) */
#define     SCI2C1      (*((volatile unsigned char *)(0x22)))	/* SCI2 control register 1 */
#define     SCI2C2      (*((volatile unsigned char *)(0x23)))	/* SCI2 control register 2 */
#define     SCI2S1      (*((volatile unsigned char *)(0x24)))	/* SCI2 status register 1 */
#define     SCI2S2      (*((volatile unsigned char *)(0x25)))	/* SCI2 status register 2 */
#define     SCI2C3      (*((volatile unsigned char *)(0x26)))	/* SCI2 control register 3 */
#define     SCI2D     (*((volatile unsigned char *)(0x27)))	/* SCI2 data register (low byte) */

#define     SPIC1      (*((volatile unsigned char *)(0x28)))	/* SPI control register 1 */
#define     SPIC2      (*((volatile unsigned char *)(0x29)))	/* SPI control register 2 */
#define     SPIBR      (*((volatile unsigned char *)(0x2A)))	/* SPI baud rate select */
#define     SPIS       (*((volatile unsigned char *)(0x2B)))	/* SPI status register */
#define     SPID       (*((volatile unsigned char *)(0x2D)))	/* SPI data register */

#define     TPM1SC      (*((volatile unsigned char *)(0x30)))	/* TPM1 status and control register */
#define     TPM1CNTH    (*((volatile unsigned char *)(0x31)))	/* TPM1 counter (high half) */
#define     TPM1CNTL    (*((volatile unsigned char *)(0x32)))	/* TPM1 counter (low half) */
#define     TPM1MODH    (*((volatile unsigned char *)(0x33)))	/* TPM1 modulo register (high half) */
#define     TPM1MODL    (*((volatile unsigned char *)(0x34)))	/* TPM1 modulo register(low half) */

#define     TPM1C0SC    (*((volatile unsigned char *)(0x35)))	/* TPM1 channel 0 status and control */
#define     TPM1C0VH    (*((volatile unsigned char *)(0x36)))	/* TPM1 channel 0 value register (high) */
#define     TPM1C0VL    (*((volatile unsigned char *)(0x37)))	/* TPM1 channel 0 value register (low) */
#define     TPM1C1SC    (*((volatile unsigned char *)(0x38)))	/* TPM1 channel 0 status and control */
#define     TPM1C1VH    (*((volatile unsigned char *)(0x39)))	/* TPM1 channel 1 value register (high) */
#define     TPM1C1VL    (*((volatile unsigned char *)(0x3A)))	/* TPM1 channel 1 value register (low) */
#define     TPM1C2SC    (*((volatile unsigned char *)(0x3B)))	/* TPM1 channel 2 status and control */
#define     TPM1C2VH    (*((volatile unsigned char *)(0x3C)))	/* TPM1 channel 2 value register (high) */
#define     TPM1C2VL    (*((volatile unsigned char *)(0x3D)))	/* TPM1 channel 2 value register (low) */

#define     PTFD        (*((volatile unsigned char *)(0x40)))	/* I/O port F data register */
#define     PTFPE       (*((volatile unsigned char *)(0x41)))	/* I/O port F pull-up enable register */
#define     PTFSE       (*((volatile unsigned char *)(0x42)))	/* I/O port F slew rate enable register */
#define     PTFDD       (*((volatile unsigned char *)(0x43)))	/* I/O port F data direction register */

#define     PTGD        (*((volatile unsigned char *)(0x44)))	/* I/O port G data register */
#define     PTGPE       (*((volatile unsigned char *)(0x45)))	/* I/O port G pull-up enable register */
#define     PTGSE       (*((volatile unsigned char *)(0x46)))	/* I/O port G slew rate enable register */
#define     PTGDD       (*((volatile unsigned char *)(0x47)))	/* I/O port G data direction register */

#define     ICGC1       (*((volatile unsigned char *)(0x48)))	/* ICG Control Register 1 */
#define     ICGC2       (*((volatile unsigned char *)(0x49)))	/* ICG Control Register 2 */
#define     ICGS1       (*((volatile unsigned char *)(0x4A)))	/* ICG Status Register 1 */
#define     ICGS2       (*((volatile unsigned char *)(0x4B)))	/* ICG Status Register 2 */
#define     ICGFLTH     (*((volatile unsigned char *)(0x4C)))	/* ICG Upper Filter Register */
#define     ICGFLTL     (*((volatile unsigned char *)(0x4D)))	/* ICG Lower Filter Register */
#define     ICGTRM      (*((volatile unsigned char *)(0x4E)))	/* ICG Trim Register */

#define     ATDC        (*((volatile unsigned char *)(0x50)))	/* ATD Control Register */
#define     ATDSC       (*((volatile unsigned char *)(0x51)))	/* ATD Status/Control Register */
#define     ATDRH        (*((volatile unsigned char *)(0x52)))	/* ATD Result Register High */
#define     ATDRL        (*((volatile unsigned char *)(0x53)))	/* ATD Result Register Low */
#define     ATDPE        (*((volatile unsigned char *)(0x54)))	/* ATD Pin Enable */

#define     IICA         (*((volatile unsigned char *)(0x58)))	/* IIC-Bus Address Register */
#define     IICF        (*((volatile unsigned char *)(0x59)))	/* IIC-Bus Frequency Divider Register */
#define     IICC        (*((volatile unsigned char *)(0x5A)))	/* IIC-Bus Control Register */
#define     IICS        (*((volatile unsigned char *)(0x5B)))	/* IIC-Bus Status Register */
#define     IICD        (*((volatile unsigned char *)(0x5C)))	/* IIC-Bus Data I/O Register */

#define     TPM2SC      (*((volatile unsigned char *)(0x60)))	/* TPM2 status and control register */
#define     TPM2CNTH    (*((volatile unsigned char *)(0x61)))	/* TPM2 counter (high half) */
#define     TPM2CNTL    (*((volatile unsigned char *)(0x62)))	/* TPM2 counter (low half) */
#define     TPM2MODH    (*((volatile unsigned char *)(0x63)))	/* TPM2 modulo register (high half) */
#define     TPM2MODL    (*((volatile unsigned char *)(0x64)))	/* TPM2 modulo register(low half) */
#define     TPM2C0SC    (*((volatile unsigned char *)(0x65)))	/* TPM2 channel 0 status and control */
#define     TPM2C0VH    (*((volatile unsigned char *)(0x66)))	/* TPM2 channel 0 value register (high) */
#define     TPM2C0VL    (*((volatile unsigned char *)(0x67)))	/* TPM2 channel 0 value register (low) */
#define	    TPM2C1SC	(*((volatile unsigned char *)(0x68)))
#define	    TPM2C1VH	(*((volatile unsigned char *)(0x69)))
#define	    TPM2C1VL	(*((volatile unsigned char *)(0x6A)))
#define	    TPM2C2SC	(*((volatile unsigned char *)(0x6B)))
#define	    TPM2C2VH	(*((volatile unsigned char *)(0x6C)))
#define	    TPM2C2VL	(*((volatile unsigned char *)(0x6D)))
#define	    TPM2C3SC	(*((volatile unsigned char *)(0x6E)))
#define	    TPM2C3VH	(*((volatile unsigned char *)(0x6F)))
#define	    TPM2C3VL	(*((volatile unsigned char *)(0x70)))
#define	    TPM2C4SC	(*((volatile unsigned char *)(0x71)))
#define	    TPM2C4VH	(*((volatile unsigned char *)(0x72)))
#define	    TPM2C4VL	(*((volatile unsigned char *)(0x73)))

#define     SIMRS       (*((volatile unsigned char *)(0x1800)))	/* SIM reset status register */
#define     SIMC        (*((volatile unsigned char *)(0x1801)))	/* SIM control (BDC access only) */
#define     SIMOPT      (*((volatile unsigned char *)(0x1802)))	/* SIM options register (write once) */
#define     SIMIDH      (*((volatile unsigned char *)(0x1806)))	/* SIM Part I.D. high (read-only) */
#define     SIMIDL      (*((volatile unsigned char *)(0x1807)))	/* SIM Part I.D. low (read-only) */
#define     PMCRSC      (*((volatile unsigned char *)(0x1808)))	/* PMC Real-Time Interrupt Status & Control */
#define     PMCSC1      (*((volatile unsigned char *)(0x1809)))	/* PMC Status & Control 1 */
#define     PMCSC2      (*((volatile unsigned char *)(0x180A)))	/* PMC Status & Control 2 */
#define     PMCT        (*((volatile unsigned char *)(0x180B)))	/* PMC Test */

#define     DBGCAH      (*((volatile unsigned char *)(0x1810)))	/* DBG comparator register A (high) */
#define     DBGCAL      (*((volatile unsigned char *)(0x1811)))	/* DBG comparator register A (low) */
#define     DBGCBH      (*((volatile unsigned char *)(0x1812)))	/* DBG comparator register B (high) */
#define     DBGCBL      (*((volatile unsigned char *)(0x1813)))	/* DBG comparator register B (low) */
#define     DBGFH       (*((volatile unsigned char *)(0x1814)))	/* DBG FIFO register (high) */
#define     DBGFL       (*((volatile unsigned char *)(0x1815)))	/* DBG FIFO register (low) */
#define     DBGC        (*((volatile unsigned char *)(0x1816)))	/* DBG control register */
#define     DBGT        (*((volatile unsigned char *)(0x1817)))	/* DBG trigger register */
#define     DBGS        (*((volatile unsigned char *)(0x1818)))	/* DBG status register */

#define     FCDIV       (*((volatile unsigned char *)(0x1820)))	/* Flash/EE clock divider register */
#define     FOPT        (*((volatile unsigned char *)(0x1821)))	/* Flash/EE option register */
#define     FCNFG       (*((volatile unsigned char *)(0x1823)))	/* Flash/EE configuration register */
#define     FPROT       (*((volatile unsigned char *)(0x1824)))	/* Flash protection register */
#define     FSTAT       (*((volatile unsigned char *)(0x1825)))	/* Flash status register */
#define     FCMD        (*((volatile unsigned char *)(0x1826)))	/* Flash command register */

#define     NVBACKKEY   (*((volatile unsigned char *)(0xFFB0)))	/* 8-byte backdoor comparison key */
#define     NVPROT     	(*((volatile unsigned char *)(0xFFBD)))	/* NV flash protection byte */
#define     NVICGTRM    (*((volatile unsigned char *)(0xFFBE)))	/* ICG manual trim value*/
#define     NVOPT     	(*((volatile unsigned char *)(0xFFBF)))	/* NV flash options byte */

#endif /* _IOCONFIG_S08_H_ */