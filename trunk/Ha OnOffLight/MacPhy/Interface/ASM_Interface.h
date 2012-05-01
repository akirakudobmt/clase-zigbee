/************************************************************************************
* This header file is provided as part of the interface to the Freescale 802.15.4
* MAC and PHY layer.
*
* The file covers the interface to the Advanced Security Module (ASM).
*
* (c) Copyright 2007, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _ASM_INTERFACE_H_
#define _ASM_INTERFACE_H_

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/* This data type enumerates the ASM API calls return values */
typedef enum
{
  gAsmErrNoError_c,
  gAsmSelfTestFail_c,
  gAsmSelfTestNotPerformed_c,
  gAsmInternalTimeout_c,
  gAsmNullPointer_c,
  gAsmErrMax_c
}AsmErr_t;


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* This function is called to initialize the ASM driver, ASM control registers and to 
*  perform the AES self-test.
*   
* Interface assumptions:
*   None
*   
* Return value:
*    gAsmErrNoError_c;
*    gAsmSelfTestFail_c;
*    gAsmInternalTimeout_c;
*
************************************************************************************/
AsmErr_t Asm_Init(void);


/************************************************************************************
* This function performs AES CTR encryption/decryption. 
*   
* Interface assumptions:
*   pKey     - pointer to key used for encryption/decryption or NULL if no key should be loaded
*   pCounter - pointer to counter used for encryption/decryption or NULL if no counter should be loaded
*   pData    - pointer to data to encrypt/decrypt
*   pResult  - pointer to memory location where the result will be stored
*   
* Return value:
*    gAsmErrNoError_c;
*    gAsmNullPointer_c
*    gAsmSelfTestNotPerformed_c;
*    gAsmInternalTimeout_c;
*
************************************************************************************/
AsmErr_t Asm_CtrEncryptDecrypt(  uint8_t* pKey, 
                                 uint8_t* pCounter, 
                                 uint8_t* pData,
                                 uint8_t* pResult);

/************************************************************************************
* This function performs AES CBC-MAC generation. 
*   
* Interface assumptions:
*   pKey        - pointer to key used for CBC-MAC generation or NULL if no key should be loaded
*   pMacPreload - pointer to the MAC value used used as initial value or NULL if no MAC initial value should be loaded
*   pData       - pointer to data used for MAC calculation
*   pCbcMacVal  - pointer to memory location where the CBC-MAC result will be stored
*   
* Return value:
*    gAsmErrNoError_c;
*    gAsmNullPointer_c
*    gAsmSelfTestNotPerformed_c;
*    gAsmInternalTimeout_c;
*
************************************************************************************/
AsmErr_t Asm_CbcMacEncryptDecrypt(  uint8_t* pKey, 
                                    uint8_t* pMacPreload, 
                                    uint8_t* pData, 
                                    uint8_t* pCbcMacVal);



/************************************************************************************
* This function performs AES CCM encryption/decryption. 
*   
* Interface assumptions:
*   pKey        - pointer to key used for encryption/decryption or NULL if no key should be loaded
*   pCounter    - pointer to counter used for encryption/decryption or NULL if no counter should be loaded
*   pMacPreload - pointer to the MAC value used used as initial value or NULL if no MAC initial value should be loaded
*   pData       - pointer to data used for encrypt/decrypt
*   pCbcMacVal  - pointer to memory location where the CBC-MAC result will be stored
*   pResult     - pointer to memory location where the encryption/decryption result will be stored
*   
* Return value:
*    gAsmErrNoError_c;
*    gAsmNullPointer_c
*    gAsmSelfTestNotPerformed_c;
*    gAsmInternalTimeout_c;
*
************************************************************************************/
AsmErr_t Asm_CcmEncryptDecrypt(  uint8_t* pKey,
                                 uint8_t* pCounter,
                                 uint8_t* pMacPreload, 
                                 uint8_t* pData, 
                                 uint8_t* pCbcMacVal,
                                 uint8_t* pResult);


/************************************************************************************
* This function performs plain AES encryption/decryption. 
*   
* Interface assumptions:
*   pKey     - pointer to key used for encryption/decryption or NULL if no key should be loaded
*   pData    - pointer to data to encrypt/decrypt
*   pResult  - pointer to memory location where the result will be stored
*   
* Return value:
*    gAsmErrNoError_c;
*    gAsmNullPointer_c
*    gAsmSelfTestNotPerformed_c;
*    gAsmInternalTimeout_c;
*
************************************************************************************/
AsmErr_t Asm_AesEncryptDecrypt(  uint8_t* pKey,  
                                 uint8_t* pData,
                                 uint8_t* pResult);                                 

#endif /* _ASM_INTERFACE_H_ */
