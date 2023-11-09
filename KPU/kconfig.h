/* 
   kconfig.h 
   KPU configuration, base types and macros 
*/

#ifndef KCONFIG_H_
#define KCONFIG_H_

#include <stdint.h>

#define K_DEBUG 1

#ifdef K_DEBUG
#include <stdio.h>
#define K_DEBUGMSG(msg) printf(msg) 
#define K_DEBUG1ARG(msg, arg) printf(msg, arg) 
#else
#define K_DEBUGMSG(msg) 
#define K_DEBUG1ARG(msg, arg)
#endif

/* base types */
typedef uint16_t  kword_t;  /* internal word base type */
typedef uint32_t  kdword_t; /* internal dword base type */
typedef uint8_t   kbyte_t;  /* UNUSED - 8 bits BYTE type */
typedef uint16_t  kaddr_t;  /* memory address base type */
typedef kword_t   kinstr_t; /* KPU instruction base type */


#define KPUWORDSIZE (sizeof(kword_t)) /* KPU word size in bytes */
#define KPUBITS     (KPUWORDSIZE<<3)  /* KPU word size in bits */
#define KPUMAXWORD  ((1u<<KPUBITS)-1) /* word MAX valut */

#define KPUPTRSIZE (sizeof(kaddr_t))  /* KPU Memory pointer size in bytes */
#define KPUADDRBITS (KPUPTRSIZE<<3)   /* address bus size in bits */
#define KPUMEMSIZE (1u<<KPUADDRBITS)  /* memory size in words  */

#define KPUMEMLASTWORD (KPUMEMSIZE-1) /* last word address */

#define KPUSTARTEXECADDR (KPUMEMSIZE-2)  /* 0xFFFE - PC init at reset */
#define KPUSTARTSTACKADDR (KPUMEMSIZE>>1)         /* stack initially placed at half mem size - 0x8000 - SP init at reset */

#endif
