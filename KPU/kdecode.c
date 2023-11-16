/* KPU - Krueger Didactical CPU 
   Kdecode.c 
 */

#include "kconfig.h"
#include "KPU.h"
#include "kdecode.h"

/* Instruction Manager function pointer, receives CPU and Memory pointers */
typedef void(*KPU_FDec_t)(kaddr_t, kinstr_t, char*);

/* instructions managers prototypes */
static void KI_ADDDec(kaddr_t a, kinstr_t i, char* s);
static void KI_ADDIDec(kaddr_t a, kinstr_t i, char* s);
static void KI_NANDDec(kaddr_t a, kinstr_t i, char* s);
static void KI_LUIDec(kaddr_t a, kinstr_t i, char* s);
static void KI_SWDec(kaddr_t a, kinstr_t i, char* s);
static void KI_LWDec(kaddr_t a, kinstr_t i, char* s);
static void KI_BEQDec(kaddr_t a, kinstr_t i, char* s);
static void KI_JALRDec(kaddr_t a, kinstr_t i, char* s);


/* Instruction managers */
KPU_FDec_t K_InstrDec[8] = {
  KI_ADDDec,
  KI_ADDIDec,
  KI_NANDDec,
  KI_LUIDec,
  KI_SWDec,
  KI_LWDec,
  KI_BEQDec,
  KI_JALRDec,
};

const char* mnemonic[8] = {
  "ADD",
  "ADDI",
  "NAND",
  "LUI",
  "SW",
  "LW",
  "BEQ",
  "JALR"
};

/* instruction buffer */
char instr[KINSTRMAXLEN];

void KPU_DecodeInstr(kaddr_t a, kinstr_t i, char* s) {
  int opcode = (i >> 13);

  if (K_InstrDec[opcode] == NULL) {
    K_DEBUG1ARG("KPU_DecodeInstr: Error. Unknown instruction opcode 0x%02X\n", i);
  }
  else {
    /* instruction validation done inside the Mgr function */
    K_InstrDec[opcode](a, i, s);
  }
}

/* instructions managers */
static void KI_ADDDec(kaddr_t a, kinstr_t i, char* s) {
  /* ADD rA, rB, rC */
  int A, B, C;  

  /* check bits 3..6 == 0 */
  if ((i & (0xfu << 3)) == 0u) {
    A = KGETRA(i);
    B = KGETRB(i);
    C = KGETRC(i);
    snprintf(s, KINSTRMAXLEN, "%s r%d, r%d, r%d ", mnemonic[KI_ADD], A, B, C);
  }
  else {
    /* TODO: eccezione, istruzione non valida */
    snprintf(s, KINSTRMAXLEN, "; Invalid '%s' Instruction '0x%04X' ", mnemonic[KI_ADD], i);    
  }
}

static void KI_ADDIDec(kaddr_t a, kinstr_t i, char* s) {
  /* ADDI rA, rB, X ; */
  int A, B, X;  

  A = KGETRA(i);  
  B = KGETRB(i);
  X = KGETXRRI(i);
  /* set X sign */
  X = ((X < 64) ? X : (X - 128));  
  snprintf(s, KINSTRMAXLEN, "%s r%d, r%d, %+d ", mnemonic[KI_ADDI], A, B, X);
}

static void KI_LUIDec(kaddr_t a, kinstr_t i, char* s) {
  /* LUI rA, X */
  int A, X;
  kdword_t locTempRes;

  A = KGETRA(i);  
  X = KGETXRI(i);
  locTempRes = X << 6;
  snprintf(s, KINSTRMAXLEN, "%s r%d, %04X ; (immediate %04X) ", mnemonic[KI_LUI], A, locTempRes, X);
}

static void KI_JALRDec(kaddr_t a, kinstr_t i, char* s) {
  /* JALR rA, rB */
  int A, B, X;

  A = KGETRA(i);
  B = KGETRB(i);
  X = KGETXRRI(i);
  /* check bits 0..6 == 0 */
  if (X == 0u) {
    /* regular JALR instruction */
    snprintf(s, KINSTRMAXLEN, "%s r%d, r%d ", mnemonic[KI_JALR], A, B);
  }
  else if ((A == 0) && (B == 0)) {
    /* HALT instruction */
    snprintf(s, KINSTRMAXLEN, "HALT");
  }
  else {
    snprintf(s, KINSTRMAXLEN, "; Invalid '%s' Instruction '0x%04X' ", mnemonic[KI_JALR], i);    
  }
}

static void KI_NANDDec(kaddr_t a, kinstr_t i, char* s) {
  /* NAND rA, rB, rC */
  int A, B, C;  

  /* check bits 3..6 == 0 */
  if ((i & (0xfu << 3)) == 0u) {
    A = KGETRA(i);    
    B = KGETRB(i);
    C = KGETRC(i);
    snprintf(s, KINSTRMAXLEN, "%s r%d, r%d, r%d ", mnemonic[KI_NAND], A, B, C);
  }
  else {
    /* TODO: eccezione, istruzione non valida */
    snprintf(s, KINSTRMAXLEN, "; Invalid '%s' Instruction '0x%04X' ", mnemonic[KI_NAND], i);    
  }
}

static void KI_SWDec(kaddr_t a, kinstr_t i, char* s) {
  /* SW rA, rB, X */
  int A, B, X;

  A = KGETRA(i);
  B = KGETRB(i);
  X = KGETXRRI(i);
  /* correct X sign */
  X = ((X < 64) ? X : (X - 128));  
  snprintf(s, KINSTRMAXLEN, "%s r%d, r%d, %d ; (relative address [r%d %+d])", mnemonic[KI_SW], A, B, X, B, X);
}

static void KI_LWDec(kaddr_t a, kinstr_t i, char* s) {
  /* LW rA, rB, X */
  int A, B, X;  

  A = KGETRA(i);
  B = KGETRB(i);
  X = KGETXRRI(i);
  /* correct X sign */
  X = ((X < 64) ? X : (X - 128));
  snprintf(s, KINSTRMAXLEN, "%s r%d, r%d, %d ; (relative address [r%d %+d])", mnemonic[KI_LW], A, B, X, B, X);
}

static void KI_BEQDec(kaddr_t a, kinstr_t i, char* s) {
  /* BEQ rA, rB, X */
  int A, B, X;
  int locTempRes = a;

  A = KGETRA(i);
  B = KGETRB(i);  
  X = KGETXRRI(i);
  /* correct X sign */
  X = ((X < 64) ? X : (X - 128));
  /* calcola nuovo PC */
  locTempRes += X;
  snprintf(s, KINSTRMAXLEN, "%s r%d, r%d, %+d ; brench to %04X if equal ", mnemonic[KI_BEQ], A, B, X, locTempRes);
}
