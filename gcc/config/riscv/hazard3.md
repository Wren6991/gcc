;; Machine description for Hazard3 custom extensions
;; Copyright (C) 2026 Free Software Foundation, Inc.

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; GCC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.

;; Xh3bextm: h3.bextm and h3.bextmi
;;
;; h3.bextm rd, rs1, rs2, nbits:
;;   rd = (rs1 >> rs2[4:0]) & ((1 << nbits) - 1)
;;   nbits is a compile-time constant in the range 1..8.
;;
;; h3.bextmi rd, rs1, shamt, nbits:
;;   rd = (rs1 >> shamt) & ((1 << nbits) - 1)
;;   shamt is a compile-time constant in the range 0..31.
;;   nbits is a compile-time constant in the range 1..8.

;; h3.bextm matching the canonical (zero_extract ...) form with a
;; register variable bit position.
(define_insn "*h3_bextmsi3_extract"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(zero_extract:SI (match_operand:SI 1 "register_operand" "r")
			 (match_operand 2 "const_int_operand" "n")
			 (match_operand:QI 3 "register_operand" "r")))]
  "TARGET_XH3BEXTM
   && IN_RANGE (INTVAL (operands[2]), 1, 8)"
  "h3.bextm\t%0,%1,%3,%2"
  [(set_attr "type" "bitmanip")
   (set_attr "mode" "SI")])

;; h3.bextm matching the (and (lshiftrt ...) mask) form with a
;; register variable bit position.  The mask must be a contiguous
;; low-bit mask of the form (1 << n) - 1 with n in 1..8.
(define_insn "*h3_bextmsi3_and"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(and:SI (lshiftrt:SI (match_operand:SI 1 "register_operand" "r")
			     (match_operand:QI 2 "register_operand" "r"))
		(match_operand 3 "const_int_operand" "n")))]
  "TARGET_XH3BEXTM
   && IN_RANGE (INTVAL (operands[3]), 1, 255)
   && (UINTVAL (operands[3]) & (UINTVAL (operands[3]) + 1)) == 0"
{
  operands[3] = GEN_INT (exact_log2 (UINTVAL (operands[3]) + 1));
  return "h3.bextm\t%0,%1,%2,%3";
}
  [(set_attr "type" "bitmanip")
   (set_attr "mode" "SI")])

;; h3.bextmi matching the canonical (zero_extract ...) form with a
;; constant bit position.
(define_insn "*h3_bextmisi3_extract"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(zero_extract:SI (match_operand:SI 1 "register_operand" "r")
			 (match_operand 2 "const_int_operand" "n")
			 (match_operand 3 "const_int_operand" "n")))]
  "TARGET_XH3BEXTM
   && IN_RANGE (INTVAL (operands[2]), 1, 8)
   && IN_RANGE (INTVAL (operands[3]), 0, 31)"
  "h3.bextmi\t%0,%1,%3,%2"
  [(set_attr "type" "bitmanip")
   (set_attr "mode" "SI")])

;; h3.bextmi matching the (and (lshiftrt ...) mask) form with a
;; constant bit position.  The mask must be a contiguous low-bit mask
;; of the form (1 << n) - 1 with n in 1..8.
(define_insn "*h3_bextmisi3_and"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(and:SI (lshiftrt:SI (match_operand:SI 1 "register_operand" "r")
			     (match_operand 2 "const_int_operand" "n"))
		(match_operand 3 "const_int_operand" "n")))]
  "TARGET_XH3BEXTM
   && IN_RANGE (INTVAL (operands[2]), 0, 31)
   && IN_RANGE (INTVAL (operands[3]), 1, 255)
   && (UINTVAL (operands[3]) & (UINTVAL (operands[3]) + 1)) == 0"
{
  operands[3] = GEN_INT (exact_log2 (UINTVAL (operands[3]) + 1));
  return "h3.bextmi\t%0,%1,%2,%3";
}
  [(set_attr "type" "bitmanip")
   (set_attr "mode" "SI")])

;; Xh3sfx: h3.xorsign
;;
;; h3.xorsign rd, rs1, rs2:
;;   rd = rs1 < 0 ? -rs2 : rs2
;;   A conditional two's-complement negation of rs2 based on the sign
;;   bit of rs1.  When rs1 == rs2 this computes abs(rs1).

(define_insn "h3_xorsignsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(if_then_else:SI
	  (lt:SI (match_operand:SI 1 "register_operand" "r") (const_int 0))
	  (neg:SI (match_operand:SI 2 "register_operand" "r"))
	  (match_dup 2)))]
  "TARGET_XH3SFX"
  "h3.xorsign\t%0,%1,%2"
  [(set_attr "type" "bitmanip")
   (set_attr "mode" "SI")])
