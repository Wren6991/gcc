/* { dg-do compile } */
/* { dg-options "-O2 -march=rv32gc_zbkb -mabi=ilp32" } */
/* { dg-skip-if "" { *-*-* } { "-g" "-flto" "-O0" } } */

#include <stdint.h>

/* Unaligned byte-swapped load: should still optimise to 4x lbu + 2x packh + 1x
   pack, with permuted operands.  */

/* BE32 load.  */
uint32_t g(uint8_t *p) {
	return (uint32_t)p[3] | (uint32_t)p[2] << 8 |
	       (uint32_t)p[1] << 16 | (uint32_t)p[0] << 24;
}

/* LE32 load with explicit swap.  */
uint32_t h(uint8_t *p) {
	return __builtin_bswap32(
		(uint32_t)p[0] | (uint32_t)p[1] << 8 |
		(uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
}

/* { dg-final { scan-assembler-times "\\slbu\\s" 8 } } */
/* { dg-final { scan-assembler-times "\\spackh\\s" 4 } } */
/* { dg-final { scan-assembler-times "\\spack\\s" 2 } } */
/* No actual byte-swap should survive: neither rev8 nor an explicit
   shift-based swap sequence should appear.  */
/* { dg-final { scan-assembler-not "\\srev8\\s" } } */
/* { dg-final { scan-assembler-not "\\ssrli\\s" } } */
/* { dg-final { scan-assembler-not "\\sslli\\s" } } */
/* { dg-final { scan-assembler-not "\\sor\\s" } } */
