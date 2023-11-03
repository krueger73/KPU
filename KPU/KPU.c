/* KaPUt - Krueger Didactical CPU */

#include "kconfig.h"

/* KPU flag ZERO position */
#define KFZERO    0
/* KPU flag CARRY/overflow position */
#define KFCARRY   1
/* KPU flag BORROW/underflow position */
#define KFPARITY  2

/* flag uty: TBD */
#define KFLAGMSK(flag)    ((kword_t)1u<<(flag))
#define KFLAGINVMSK(flag) ~(KFLAGMSK(flag))

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

#ifdef K_DEBUG
const char* mnemonic[8] = {
  "ADD ", 
  "ADDI ",
  "NAND ",
  "LUI ",
  "SW ",
  "LW ",
  "BEQ ",
  "JALR "
};
#endif

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

/* Instruction Manager function pointer, receives CPU and Memory pointers */
typedef void(*KPU_FMgm_t)(kpu_t*, kword_t*);

/* instructions managers prototypes */
void KI_ADDMgm(kpu_t* k, kword_t* m);
void KI_ADDIMgm(kpu_t* k, kword_t* m);
void KI_NANDMgm(kpu_t* k, kword_t* m);
void KI_LUIMgm(kpu_t* k, kword_t* m);
void KI_SWMgm(kpu_t* k, kword_t* m);
void KI_LWMgm(kpu_t* k, kword_t* m);
void KI_BEQMgm(kpu_t* k, kword_t* m);
void KI_JALRMgm(kpu_t* k, kword_t* m);


/* globals */

/* cpu(s) */
kpu_t cpu0;
/* installed memory */
kword_t m[KPUMEMSIZE]; 

/* Instruction managers */
KPU_FMgm_t K_InstMgr[8] = {  
  KI_ADDMgm, 
  KI_ADDIMgm, 
  KI_NANDMgm, 
  KI_LUIMgm, 
  KI_SWMgm, 
  KI_LWMgm, 
  KI_BEQMgm, 
  KI_JALRMgm
};

/* uty prototypes */
/* cpu cold reset */
void KPU_CpuReset(kpu_t *k);
/* memory clean to 0xff */
void KPU_MemReset(kword_t*m);
/* CPU main loop fetch and execute */
void KPU_MainLoop(kpu_t* k, kword_t* m);

/* fetch generic data word from memory */
kword_t KPU_FetchWord(kword_t* m, kaddr_t a); 
/* store generic data word to memory */
void KPU_StoreWord(kword_t* m, kaddr_t a, kword_t w);

/* fetch instruction word from memory [PC] -> IR */
void KPU_FetchInstr(kword_t* m, kpu_t* k);
/* execute instruction in IR, update PC */
void KPU_ExecInstr(kword_t* m, kpu_t* k);

int main()
{
  int d = 0xA000;

  KPU_MemReset(m);
  m[KPUSTARTEXECADDR] = 0x6680; /* LUI r1, 0x280 ; 0x280 == 0xA000 >> 6 */
  m[KPUSTARTEXECADDR+1] = 0xE080; /* JARL r0, r1 ; jump r1 */
  
  /* TODO: memory init / ROM load / program load */
  m[d++] = 0x6B7A; /* LUI r2, 0x37A */
  m[d++] = 0x292D; /* ADDI r2, r2, 0x2D - 0xDEAD */
  m[d++] = 0x6EFB; /* LUI r3, 0x2FB */
  m[d++] = 0x2DAF; /* ADDI r3, r3, 0x2F - 0xBEEF */
  m[d++] = 0x0103; /* ADD r0, r2, r3 */
  m[d++] = 0x0503; /* ADD r1, r2, r3 */
  m[d++] = 0x0582; /* ADD r1, r3, r2 */
  m[d++] = 0x0800; /* ADD r2, r0, r0 */
  m[d++] = 0xE000; /* jalr r0, r0 ; HALT */

  KPU_CpuReset(&cpu0);
  KPU_MainLoop(&cpu0, m);

  return 0;
}

void KPU_CpuReset(kpu_t* k) {
  K_DEBUGMSG("KPU_CpuReset.");
  k->F = 0;
  for (int i = 0; i < 8; i++) {
    k->R[i] = 0;
  }
  
  k->IR = 0;
  k->SP = KPUSTARTSTACKADDR;
  k->PC = KPUSTARTEXECADDR;
  K_DEBUGMSG("Done.\n");
}

void KPU_MemReset(kword_t* m) {
  int addr;
  kword_t* m0 = m;
  K_DEBUGMSG("KPU_MemReset.");

  for (addr = 0; addr < KPUMEMSIZE; addr++) {
    *(m0++) = 0xFFFFu; /* init RAM with 0xffff */
  }
  K_DEBUGMSG("Done.\n");
}

void KPU_MainLoop(kpu_t* k, kword_t* m) {  
  K_DEBUGMSG("KPU_MainLoop Started.\n");
  /* TODO: infinite loop? */
  for (;;) {
    /* fetch next instruction from memory [PC] -> IR */
    KPU_FetchInstr(m, k);    
    /* update PC */
    k->PC++;
    /* execute it */
    KPU_ExecInstr(m, k);    
  }
}

void KPU_FetchInstr(kword_t* m, kpu_t* k) {
  /* load next instruction in IR - esplicit cast */
  k->IR = (kinstr_t)KPU_FetchWord(m, k->PC);  
}

kword_t KPU_FetchWord(kword_t* m, kaddr_t a) {
  kword_t locRes = 0xDEAD;
  kaddr_t locAddr = a;
  
  /* simple fetch 8 bits word from m[a] */
  locRes = m[locAddr];
  return locRes;
}

void KPU_StoreWord(kword_t* m, kaddr_t a, kword_t w) {
  kword_t locData = w;
  kaddr_t locAddr = a;

  m[locAddr] = locData;
}

void KPU_ExecInstr(kword_t* m, kpu_t* k) {  
  int opcode = (k->IR >> 13);

  if (K_InstMgr[opcode] == NULL) {
    K_DEBUG1ARG("KPU_ExecInstr: Error. Unknown instruction opcode 0x%02X\n", k->IR);
  }
  else {
    /* instruction validation done in Mgr function */
    K_InstMgr[opcode](k, m);
  }
}

/* instructions managers */
void KI_ADDMgm(kpu_t* k, kword_t* m) {
  /* ADD rA, rB, rC */
  int A, B, C; 
  kdword_t locTempRes;

  /* check bits 3..6 == 0 */
  if ((k->IR & (0xfu << 3)) == 0u) {
    A = KGETRA(k->IR);
    if (A != 0) {
      B = KGETRB(k->IR);
      C = KGETRC(k->IR);
      locTempRes = (kdword_t)KGETREG(k, B) + KGETREG(k, C);
      KSETREG(k, A, (kword_t)locTempRes);
    }
    /* TODO: flags management */
  }
  else {
    /* TODO: eccezione, istruzione non valida */
    K_DEBUGMSG("KI_ADDMgm: instruction format invalid\n");
  }
}

void KI_ADDIMgm(kpu_t* k, kword_t* m) {
  /* ADDI rA, rB, X */
  int A, B, X;
  kdword_t locTempRes;

  A = KGETRA(k->IR);
  if (A != 0) {
    B = KGETRB(k->IR);
    X = KGETXRRI(k->IR);
    /* correct X sign */
    X = ((X < 64) ? X : (X - 128));
    locTempRes = (kdword_t)KGETREG(k, B) + X;
    KSETREG(k, A, (kword_t)locTempRes);
  }
  /* TODO: flags management */
}

void KI_LUIMgm(kpu_t* k, kword_t* m) {
  /* LUI rA, X */
  int A,  X;
  kdword_t locTempRes;

  A = KGETRA(k->IR);
  if (A != 0) {
    X = KGETXRI(k->IR);
    locTempRes = X << 6;
    KSETREG(k, A, (kword_t)locTempRes);
  }
}

void KI_JALRMgm(kpu_t* k, kword_t* m) {
  /* JALR rA, rB */
  int A, B;
  
  /* check bits 0..6 == 0 */
  if ((k->IR & 0x7fu) == 0u) {
    A = KGETRA(k->IR);
    B = KGETRB(k->IR);
    if ((A == 0) && (B == 0)) {
      /* HALT instruction */
      /* DUMP registers & HALT */
      K_DEBUGMSG("HALT!\n");
      K_DEBUG1ARG("CPU dump: R0: 0x%04X ", k->R[0]);
      K_DEBUG1ARG("R1: 0x%04X ", k->R[1]);
      K_DEBUG1ARG("R2: 0x%04X ", k->R[2]);
      K_DEBUG1ARG("R3: 0x%04X ", k->R[3]);
      K_DEBUG1ARG("R4: 0x%04X ", k->R[4]);
      K_DEBUG1ARG("R5: 0x%04X ", k->R[5]);
      K_DEBUG1ARG("R6: 0x%04X ", k->R[6]);
      K_DEBUG1ARG("R7: 0x%04X ", k->R[7]);
      K_DEBUG1ARG("F: 0x%04X ", k->F);
      K_DEBUG1ARG("PC: 0x%04X\n", k->PC);
      /* HALT */
      for (;;);
    }
    else {
      if (A != 0) {
        /* rA <- PC + 1 */
        KSETREG(k, A, k->PC); /* PC punta già all'istruzione successiva */
      }
      k->PC = KGETREG(k, B); /* WARNING? b può essere r0 -> PC == 0 */
      /* TODO: flags management */
    }
  }
  else {
    /* TODO: eccezione, istruzione non valida */
    K_DEBUGMSG("KI_JALRMgm: instruction format invalid\n");
  }
}

void KI_NANDMgm(kpu_t* k, kword_t* m) {
  /* NAND rA, rB, rC */
  int A, B, C;
  kword_t locTempRes;

  /* check bits 3..6 == 0 */
  if ((k->IR & (0xfu << 3)) == 0u) {
    A = KGETRA(k->IR);
    if (A != 0) {
      B = KGETRB(k->IR);
      C = KGETRC(k->IR);
      /* NAND bit a bit */
      locTempRes = ~(KGETREG(k, B) & KGETREG(k, C));
      KSETREG(k, A, locTempRes);
    }
    /* TODO: flags management */
  }
  else {
    /* TODO: eccezione, istruzione non valida */
    K_DEBUGMSG("KI_NANDMgm: instruction format invalid\n");
  }
}

void KI_SWMgm(kpu_t* k, kword_t* m) {
  /* SW rA, rB, X */
  int A, B, X;
  kdword_t locTempRes;

  A = KGETRA(k->IR);  
  B = KGETRB(k->IR);
  X = KGETXRRI(k->IR);
  /* correct X sign */
  X = ((X < 64) ? X : (X - 128));

  locTempRes = (kdword_t)KGETREG(k, B) + X;
  m[locTempRes] = KGETREG(k, A);

  /* TODO: flags management */
}

void KI_LWMgm(kpu_t* k, kword_t* m) {
  /* LW rA, rB, X */
  int A, B, X;
  kdword_t locTempRes;

  A = KGETRA(k->IR);
  if (A != 0) {
    B = KGETRB(k->IR);
    X = KGETXRRI(k->IR);
    /* correct X sign */
    X = ((X < 64) ? X : (X - 128));

    locTempRes = (kdword_t)KGETREG(k, B) + X;
    KSETREG(k, A, m[locTempRes]);    
  }
  /* TODO: flags management */
}

void KI_BEQMgm(kpu_t* k, kword_t* m) {
  /* BEQ rA, rB, X */  
  int A, B, X;
  kdword_t locTempRes;

  A = KGETRA(k->IR);
  B = KGETRB(k->IR);
  if (KGETREG(k, A) == KGETREG(k, B)) {
    X = KGETXRRI(k->IR);
    /* correct X sign */
    X = ((X < 64) ? X : (X - 128));
    /* calcola nuovo PC */
    X += k->PC;
    /* PC punta già alla prossima istruzione. Assegna */
    k->PC = (kaddr_t)X;
  }
  /* TODO: flags management */
}
