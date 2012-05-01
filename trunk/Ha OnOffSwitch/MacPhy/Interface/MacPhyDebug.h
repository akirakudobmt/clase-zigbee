

#ifndef _MAC_PHY_DEBUG_H_
#define _MAC_PHY_DEBUG_H_


#define  PDBG_RESERVED_EVENT          0x00
#define  PDBG_EXTENDED_EVENT          0x80
#define  PDBG_SIMPLE_EVENT            0x40

// a value of zero disables the logging of that event
#define  PDBG_MAC_FUNCTION_CALL       0 // PDBG_SIMPLE_EVENT


#if defined(PROCESSOR_MC1323X) && defined(MAC_PHY_DEBUG)

 typedef void (*pfDebugLogFunc_t)(void);
 void MacDebugLogFunc(pfDebugLogFunc_t pFunc);

 #define MAC_DEBUG_LOG(param) MacDebugLogFunc((pfDebugLogFunc_t)param)

#else  /* PROCESSOR_MC1323X */
 #define MAC_DEBUG_LOG(param)
#endif /* PROCESSOR_MC1323X */


#endif /* _MAC_PHY_DEBUG_H_ */
