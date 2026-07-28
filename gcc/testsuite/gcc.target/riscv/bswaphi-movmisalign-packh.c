/* { dg-do compile } */
/* { dg-options "-O2 -march=rv32gc_zbkb -mabi=ilp32" } */
/* { dg-skip-if "" { *-*-* } { "-g" "-flto" "-O0" } } */

#include <stdint.h>

/* Byte-swapped misaligned load: should expand to lbu + lbu + packh, with
  the byte swap implemented by swapping the packh operands. There should be no
  separate byte-swap instruction.  */

/* BE16 load.  */
uint16_t g(uint8_t *p) {
	return (uint16_t)p[1] | (uint16_t)p[0] << 8;
}

/* LE16 load with explicit swap.  */
uint16_t h(uint8_t *p) {
	return __builtin_bswap16((uint16_t)p[0] | (uint16_t)p[1] << 8);
}

/* { dg-final { scan-assembler-times "\\slbu\\s" 4 } } */
/* { dg-final { scan-assembler-times "\\spackh\\s" 2 } } */
/* No actual byte-swap should survive: neither rev8 nor an explicit
   shift-based swap sequence should appear.  */
/* { dg-final { scan-assembler-not "\\srev8\\s" } } */
/* { dg-final { scan-assembler-not "\\ssrli\\s" } } */
/* { dg-final { scan-assembler-not "\\sor\\s" } } */
/* { dg-final { scan-assembler-not "\\sslli\\s" } } */
/* { dg-final { scan-assembler-not "\\szext\\.h\\s" } } */
