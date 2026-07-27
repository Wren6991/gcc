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

;; Recognise the standard abs() idiom
;;
;;   neg t, a        ; t = -a
;;   max d, a, t     ; d = max(a, -a) = abs(a)
;;
;; and fold it into a single h3.xorsign, which computes abs(a) when its
;; rs1 and rs2 are the same register.  Both orderings of the smax
;; operands are accepted.
;;
;; Safety conditions:
;;   - neg must not be of the form "neg a, a", since that destroys its
;;     input and leaves the smax reading -a for both operands.
;;   - the temporary t must not survive the transform.  It is either
;;     clobbered by the smax (t == d, the case where the abs result is
;;     written straight back into the temp) or, when t is a separate
;;     register, it must be dead after the smax since h3.xorsign does
;;     not read it.
;;
;; SImode only: h3.xorsign is a 32-bit instruction and the
;; integer smax that feeds this pattern on RV32 is *smaxsi3.

(define_peephole2
  [(set (match_operand:SI 0 "register_operand")
	(neg:SI (match_operand:SI 1 "register_operand")))
   (set (match_operand:SI 2 "register_operand")
	(smax:SI (match_operand:SI 3 "register_operand")
		 (match_operand:SI 4 "register_operand")))]
  "TARGET_XH3SFX
   && REGNO (operands[0]) != REGNO (operands[1])
   && ((REGNO (operands[3]) == REGNO (operands[1])
	&& REGNO (operands[4]) == REGNO (operands[0]))
       || (REGNO (operands[4]) == REGNO (operands[1])
	   && REGNO (operands[3]) == REGNO (operands[0])))
   && (REGNO (operands[0]) == REGNO (operands[2])
       || peep2_reg_dead_p (2, operands[0]))"
  [(set (match_dup 2)
	(if_then_else:SI
	  (lt:SI (match_dup 1) (const_int 0))
	  (neg:SI (match_dup 1))
	  (match_dup 1)))])

;; Recognise the abs-of-difference idiom
;;
;;   sub t, a, b      ; t = a - b
;;   sub u, b, a      ; u = b - a
;;   max d, t, u      ; d = max(a - b, b - a) = abs(a - b)
;;
;; and fold it into sub + h3.xorsign, since h3.xorsign computes abs of
;; its rs1/rs2 when they are the same register.  The first sub is kept
;; and the second sub plus the smax are replaced by a single h3.xorsign
;; of the first sub's result.
;;
;; Both orderings of the smax operands are accepted (it is commutative)
;; and both orderings of the two subs are matched by swapping the a/b
;; bindings, so only one pattern is needed.
;;
;; Safety conditions:
;;   - the two sub destinations must be distinct, else the smax reads
;;     the same value twice and the result is not abs.
;;   - the first sub's destination must not alias either of its sources,
;;     otherwise the second sub reads a modified input and does not
;;     compute the opposite difference.
;;   - the second sub's destination (the value we drop) must not
;;     survive the transform: either the smax clobbers it (dst2 == d)
;;     or it is dead after the smax.

(define_peephole2
  [(set (match_operand:SI 0 "register_operand")
	(minus:SI (match_operand:SI 1 "register_operand")
		  (match_operand:SI 2 "register_operand")))
   (set (match_operand:SI 3 "register_operand")
	(minus:SI (match_dup 2) (match_dup 1)))
   (set (match_operand:SI 4 "register_operand")
	(smax:SI (match_operand:SI 5 "register_operand")
		 (match_operand:SI 6 "register_operand")))]
  "TARGET_XH3SFX
   && REGNO (operands[0]) != REGNO (operands[3])
   && REGNO (operands[0]) != REGNO (operands[1])
   && REGNO (operands[0]) != REGNO (operands[2])
   && ((REGNO (operands[5]) == REGNO (operands[0])
	&& REGNO (operands[6]) == REGNO (operands[3]))
       || (REGNO (operands[5]) == REGNO (operands[3])
	   && REGNO (operands[6]) == REGNO (operands[0])))
   && (REGNO (operands[3]) == REGNO (operands[4])
       || peep2_reg_dead_p (3, operands[3]))"
  [(set (match_dup 0)
	(minus:SI (match_dup 1) (match_dup 2)))
   (set (match_dup 4)
	(if_then_else:SI
	  (lt:SI (match_dup 0) (const_int 0))
	  (neg:SI (match_dup 0))
	  (match_dup 0)))])
