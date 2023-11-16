
#include "kconfig.h"
#include "KPU.h"

int main()
{
  /* kpu(s) */
  kpu_t kpu0;
  /* installed memory */
  kword_t m[KPUMEMSIZE];

  int d = 0xA000;

  KPU_MemReset(m);
  m[KPUSTARTEXECADDR] = 0x6680; /* LUI r1, 0xA000 ; 0x280 == 0xA000 >> 6 */
  m[KPUSTARTEXECADDR + 1] = 0xE080; /* JARL r0, r1 ; jump r1 */

  /* TODO: ROM load / program load */
  m[d++] = 0x6B7A; /* LUI r2, 0x37A . 0xDE80 */
  m[d++] = 0x292D; /* ADDI r2, r2, 0x2D - 0xDEAD */
  m[d++] = 0x6EFB; /* LUI r3, 0x2FB */
  m[d++] = 0x2DAF; /* ADDI r3, r3, 0x2F - 0xBEEF */
  m[d++] = 0x0103; /* ADD r0, r2, r3 */
  m[d++] = 0x0503; /* ADD r1, r2, r3 */
  m[d++] = 0x0582; /* ADD r1, r3, r2 */
  m[d++] = 0x0800; /* ADD r2, r0, r0 */
  m[d++] = 0xE001; /* jalr r0, r0 ; HALT */

  KPU_CpuReset(&kpu0);
  KPU_MainLoop(&kpu0, m);

  return 0;
}
