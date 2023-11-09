/*
   kpu.h
   KPU configuration, base types and macros
*/

#ifndef KPU_H_
#define KPU_H_

#include "kconfig.h"

typedef struct {
  /* user registers */
  kword_t R[8]; /* Registers r0..r7 */
  /* CPU flags: TBD */
  kbyte_t F;
  /* CPU stack pointer: TBD */
  kaddr_t SP;
  /* internal instruction register */
  kinstr_t IR;
  /* Internal Program Counter */
  kaddr_t PC;
} kpu_t;


/* uty prototypes */
/* cpu cold reset */
extern void KPU_CpuReset(kpu_t* k);
/* memory clean to 0xff */
extern void KPU_MemReset(kword_t* m);
/* CPU main loop fetch and execute */
extern void KPU_MainLoop(kpu_t* k, kword_t* m);


#endif

