/*
   kpu.h
   KPU configuration, base types and macros
*/

#ifndef KPU_H_
#define KPU_H_

#include "kconfig.h"

/* registers uty - get */
#define KGETREG(cp, n)    ((n) == 0u ? 0u : cp->R[(n)])
#define KSETREG(cp, n, v) (cp->R[(n)] = ((n) == 0u ? 0u : (v))) 

#define KGETRA(instr)     (((instr)>>10) & (kinstr_t)0x7u)
#define KGETRB(instr)     (((instr)>>7) & (kinstr_t)0x7u)
#define KGETRC(instr)     ((instr) & (kinstr_t)0x7u)

#define KGETXRRI(instr)   ((instr) & (kinstr_t)0x7Fu) 
#define KGETXRI(instr)    ((instr) & (kinstr_t)0x3FFu)


/* opcodes prefixes */
#define KI_ADD  (0u)
#define KI_ADDI (1u)
#define KI_NAND (2u)
#define KI_LUI  (3u)
#define KI_SW   (4u)
#define KI_LW   (5u)
#define KI_BEQ  (6u)
#define KI_JALR (7u)


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

