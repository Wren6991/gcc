/* { dg-do compile } */
/* { dg-options "-O2 -march=rv32gc_zbkb -mabi=ilp32" } */
/* { dg-skip-if "" { *-*-* } { "-g" "-flto" "-O0" } } */

#include <stdint.h>

/* Halfword-aligned little-endian u32 load: optimised to 2x lhu + pack.  */

uint32_t f(uint16_t *p) {
	return (uint32_t)p[0] | (uint32_t)p[1] << 16;
}

/* { dg-final { scan-assembler-times "\\slhu\\s" 2 } } */
/* { dg-final { scan-assembler-times "\\spack\\s" 1 } } */
/* { dg-final { scan-assembler-not "\\slbu\\s" } } */
/* { dg-final { scan-assembler-not "\\spackh\\s" } } */
/* { dg-final { scan-assembler-not "\\sor\\s" } } */
/* { dg-final { scan-assembler-not "\\sslli\\s" } } */
