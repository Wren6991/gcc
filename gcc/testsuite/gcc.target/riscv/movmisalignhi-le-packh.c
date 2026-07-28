/* { dg-do compile } */
/* { dg-options "-O2 -march=rv32gc_zbkb -mabi=ilp32" } */
/* { dg-skip-if "" { *-*-* } { "-g" "-flto" "-O0" } } */

#include <stdint.h>

/* Byte-aligned little-endian u16 load: optimised to lbu + lbu + packh.  */

/* LE16 load.  */
uint16_t f(uint8_t *p) {
	return (uint16_t)p[0] | (uint16_t)p[1] << 8;
}

/* LE16 signed load: the packh sequence plus a sign-extend of the result.  */
int16_t i(uint8_t *p) {
	return (int16_t)p[0] | (int16_t)p[1] << 8;
}

/* { dg-final { scan-assembler-times "\\slbu\\s" 4 } } */
/* { dg-final { scan-assembler-times "\\spackh\\s" 2 } } */
/* { dg-final { scan-assembler-times "\\ssext\\.h\\s" 1 } } */
/* { dg-final { scan-assembler-not "\\sor\\s" } } */
/* { dg-final { scan-assembler-not "\\sslli\\s" } } */
/* { dg-final { scan-assembler-not "\\szext\\.h\\s" } } */
