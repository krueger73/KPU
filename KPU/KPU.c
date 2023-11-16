/* KPU - Krueger Didactical CPU */

#include "kconfig.h"
#include "kpu.h"
#include "kdecode.h"

/* KPU flag ZERO position */
#define KFZERO    0
/* KPU flag CARRY/overflow position */
#define KFCARRY   1
/* KPU flag BORROW/underflow position */
#define KFPARITY  2

/* flag uty: TBD */
#define KFLAGMSK(flag)    ((kword_t)1u<<(flag))
#define KFLAGINVMSK(flag) ~(KFLAGMSK(flag))

/* Instruction Manager function pointer, receives CPU and Memory pointers */
typedef void(*KPU_FMgm_t)(kpu_t*, kword_t*);

/* instructions managers prototypes */
static void KI_ADDMgm(kpu_t* k, kword_t* m);
static void KI_ADDIMgm(kpu_t* k, kword_t* m);
static void KI_NANDMgm(kpu_t* k, kword_t* m);
static void KI_LUIMgm(kpu_t* k, kword_t* m);
static void KI_SWMgm(kpu_t* k, kword_t* m);
static void KI_LWMgm(kpu_t* k, kword_t* m);
static void KI_BEQMgm(kpu_t* k, kword_t* m);
static void KI_JALRMgm(kpu_t* k, kword_t* m);

/* uty prototypes */
/* fetch generic data word from memory */
static kword_t KPU_FetchWord(kword_t* m, kaddr_t a);
/* store generic data word to memory */
static void KPU_StoreWord(kword_t* m, kaddr_t a, kword_t w);

/* fetch instruction word from memory [PC] -> IR */
static void KPU_FetchInstr(kword_t* m, kpu_t* k);
/* execute instruction in IR, update PC */
static void KPU_ExecInstr(kword_t* m, kpu_t* k);


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

static void KPU_FetchInstr(kword_t* m, kpu_t* k) {
  char s[KINSTRMAXLEN];
  /* load next instruction in IR - explicit cast */
  k->IR = (kinstr_t)KPU_FetchWord(m, k->PC);  
  KPU_DecodeInstr(k->PC, k->IR, s);
#ifdef K_DEBUG
  printf("%04X: %04X %s\n", k->PC, k->IR, s);
#endif
}

static kword_t KPU_FetchWord(kword_t* m, kaddr_t a) {
  kword_t locRes = 0xDEAD;
  kaddr_t locAddr = a;
  
  /* simple fetch 1 word from m[a] */
  locRes = m[locAddr]; 

  return locRes;
}

static void KPU_StoreWord(kword_t* m, kaddr_t a, kword_t w) {
  kword_t locData = w;
  kaddr_t locAddr = a;

  m[locAddr] = locData;
}

static void KPU_ExecInstr(kword_t* m, kpu_t* k) {
  int opcode = (k->IR >> 13);

  if (K_InstMgr[opcode] == NULL) {
    K_DEBUG1ARG("KPU_ExecInstr: Error. Unknown instruction opcode 0x%02X\n", k->IR);
  }
  else {
    /* instruction validation done inside the Mgr function */
    K_InstMgr[opcode](k, m);
  }
}

/* instructions managers */
static void KI_ADDMgm(kpu_t* k, kword_t* m) {
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
    K_DEBUG1ARG("KI_ADDMgm: instruction '0x%04X' invalid\n", k->IR);
  }
}

static void KI_ADDIMgm(kpu_t* k, kword_t* m) {
  /* ADDI rA, rB, X */
  int A, B, X;
  int locTempRes;

  A = KGETRA(k->IR);
  if (A != 0) {
    B = KGETRB(k->IR);
    X = KGETXRRI(k->IR);
    /* set X sign */
    X = ((X < 64) ? X : (X - 128));
    /* sum algebrically */
    locTempRes = (int)KGETREG(k, B) + X;
    /* cast result to unsigned kword.
       Result's final sign should be interpreted in context based way
     */
    KSETREG(k, A, (kword_t)locTempRes);
  }
  /* TODO: flags management */
}

static void KI_LUIMgm(kpu_t* k, kword_t* m) {
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

static void KI_JALRMgm(kpu_t* k, kword_t* m) {
  /* JALR rA, rB */
  int A, B, X;
  
  A = KGETRA(k->IR);
  B = KGETRB(k->IR);
  X = KGETXRRI(k->IR);
  /* check bits 0..6 == 0 */
  if (X == 0u) {
    /* regular JALR instruction */
    /* rA <- PC + 1 - save PC in rA */
    KSETREG(k, A, k->PC); /* PC punta già all'istruzione successiva */
    /* jump to rB */
    k->PC = (kaddr_t)KGETREG(k, B); /* WARNING? b può essere r0 -> PC == 0 */
    /* TODO: flags management */

  }  
  else if ((A == 0) && (B == 0)) {
      /* HALT instruction */
      /* DUMP registers & HALT */
      K_DEBUG1ARG("\nCPU dump: R0: 0x%04X ", k->R[0]);
      K_DEBUG1ARG("R1: 0x%04X ", k->R[1]);
      K_DEBUG1ARG("R2: 0x%04X ", k->R[2]);
      K_DEBUG1ARG("R3: 0x%04X ", k->R[3]);
      K_DEBUG1ARG("R4: 0x%04X ", k->R[4]);
      K_DEBUG1ARG("R5: 0x%04X ", k->R[5]);
      K_DEBUG1ARG("R6: 0x%04X ", k->R[6]);
      K_DEBUG1ARG("R7: 0x%04X ", k->R[7]);
      K_DEBUG1ARG("F: 0x%04X ", k->F);
      K_DEBUG1ARG("PC: 0x%04X\n", k->PC);
      K_DEBUGMSG("SYSTEM HALTED!\n");
      /* HALT 
         for now, an infinite loop 
       */
      for (;;);
  }    
  else {
    /* TODO: (X != 0) && not HALT  
       TBD 
     */
  }
}

static void KI_NANDMgm(kpu_t* k, kword_t* m) {
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
      locTempRes = ~((kdword_t)KGETREG(k, B) & KGETREG(k, C));
      KSETREG(k, A, locTempRes);
    }
    /* TODO: flags management */
  }
  else {
    /* TODO: eccezione, istruzione non valida */
    K_DEBUG1ARG("KI_NANDMgm: instruction '0x%04X' invalid\n", k->IR);    
  }
}

static void KI_SWMgm(kpu_t* k, kword_t* m) {
  /* SW rA, rB, X */
  int A, B, X;
  int locTempRes;

  A = KGETRA(k->IR);  
  B = KGETRB(k->IR);
  X = KGETXRRI(k->IR);
  /* correct X sign */
  X = ((X < 64) ? X : (X - 128));

  locTempRes = (int)KGETREG(k, B) + X;
  KPU_StoreWord(m, (kaddr_t)locTempRes, KGETREG(k, A));  
  /* TODO: flags management */
}

static void KI_LWMgm(kpu_t* k, kword_t* m) {
  /* LW rA, rB, X */
  int A, B, X;
  int locTempRes;

  A = KGETRA(k->IR);
  if (A != 0) {
    B = KGETRB(k->IR);
    X = KGETXRRI(k->IR);
    /* correct X sign */
    X = ((X < 64) ? X : (X - 128));

    locTempRes = (int)KGETREG(k, B) + X;
    KSETREG(k, A, KPU_FetchWord(m ,(kaddr_t)locTempRes));
  }
  /* TODO: flags management */
}

static void KI_BEQMgm(kpu_t* k, kword_t* m) {
  /* BEQ rA, rB, X */  
  int A, B, X;  

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
