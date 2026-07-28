/* { dg-do compile } */
/* { dg-options "-O2 -march=rv32gc_zbkb -mabi=ilp32" } */
/* { dg-skip-if "" { *-*-* } { "-g" "-flto" "-O0" } } */

#include <stdint.h>

/* Byte-aligned load: optimised to 4x lbu + 2x packh + pack.  */

/* LE32 load.  */
uint32_t f(uint8_t *p) {
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
	       (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/* LE32 signed load: same as the unsigned case */
int32_t i(uint8_t *p) {
	return (int32_t)p[0] | (int32_t)p[1] << 8 |
	       (int32_t)p[2] << 16 | (int32_t)p[3] << 24;
}

/* { dg-final { scan-assembler-times "\\slbu\\s" 8 } } */
/* { dg-final { scan-assembler-times "\\spackh\\s" 4 } } */
/* { dg-final { scan-assembler-times "\\spack\\s" 2 } } */
/* { dg-final { scan-assembler-not "\\sor\\s" } } */
/* { dg-final { scan-assembler-not "\\sslli\\s" } } */
