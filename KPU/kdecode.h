/*
   kdecode.h
   KPU - decode instruction into assembly 
*/

#ifndef KDECODE_H_
#define KDECODE_H_

#include "kconfig.h"

#define KINSTRMAXLEN 255

/* decode instruction in input */
extern void KPU_DecodeInstr(kaddr_t a, kinstr_t i, char* s);

#endif