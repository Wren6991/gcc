;; Machine description for Hazard3: custom extensions and scheduler model
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

;; DFA-based pipeline description for Hazard3.
;;
;; Hazard3 is a single-issue, in-order, 3-stage pipeline (fetch / execute /
;; memory).  With the RP2350 M-extension configuration assumed here
;; (MULDIV_UNROLL=2, MUL_FAST=1, MUL_FASTER=1, MULH_FAST=1), all integer ALU
;; and bitmanip operations -- including mul, mulh, cpop, clmul, and the
;; Xh3bextm/Xh3sfx custom extensions -- complete in a single cycle.  Divide
;; and remainder take 17 (unsigned) to 19 (signed, worst case) cycles.
;;
;; Loads have 1-cycle throughput and 2-cycle latency, with a 1-cycle
;; load-use bubble to any consumer except a store's data input (see the
;; bypass below).  Unaligned memory accesses are not supported in hardware.
;;
;; Branches execute in 1 cycle when correctly predicted and 2 cycles when
;; mispredicted; set cost to 1.
;; jal/jalr are always 2 cycles.

(define_automaton "hazard3")
(define_cpu_unit "h3_alu" "hazard3")
(define_cpu_unit "h3_muldiv" "hazard3")

;; Single-cycle ALU and bitmanip.  This covers every RV32I ALU op, the
;; Zba/Zbb/Zbc/Zbs/Zbkb instructions, and the Xh3 custom extensions (which
;; are all typed "bitmanip" above).  Also covers clz/ctz/cpop, which on
;; Hazard3 are 1-cycle -- unlike the generic model's 10-cycle imuldiv
;; reservation for cpop/clmul.
(define_insn_reservation "hazard3_alu" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "unknown,const,arith,shift,slt,multi,auipc,nop,logical,\
			move,bitmanip,min,max,minu,maxu,clz,ctz,rotate,crypto,mvpair"))
  "h3_alu")

;; condmove and zicond: Hazard3 has no hardware conditional-move, but if a
;; pattern of these types ever reaches scheduling it will be expanded from
;; a branch sequence; cost it as a branch.
(define_insn_reservation "hazard3_condmove" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "condmove,zicond"))
  "h3_alu")

;; Load: 2-cycle latency.  The scheduler will insert a bubble for any
;; dependent consumer, matching the load-use stall described in the
;; timings reference.  The bypass below removes the stall for the
;; load-data-to-store-data case, which the hardware does not stall.
(define_insn_reservation "hazard3_load" 2
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "load"))
  "h3_alu")

;; Store: 1-cycle throughput, 1-cycle latency.
(define_insn_reservation "hazard3_store" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "store"))
  "h3_alu")

;; Control transfer.  Conditional branches are 1 cycle when correctly
;; predicted and 2 when mispredicted; use 1 for scheduling.  jal/jalr are
;; always 2 cycles.
(define_insn_reservation "hazard3_branch" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "branch,jump,call,jalr,ret,trap"))
  "h3_alu")

;; Integer multiply: 1-cycle latency, single issue.  Covers mul, mulh,
;; mulhu, mulhsu (all 1-cycle with MUL_FASTER=1 and MULH_FAST=1).
(define_insn_reservation "hazard3_imul" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "imul"))
  "h3_alu")

;; Carry-less multiply (Zbc): 1-cycle latency, same as other ALU ops.
;; Separated from imul so that a future slow-clmul configuration can be
;; modelled by changing only this reservation.
(define_insn_reservation "hazard3_clmul" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "clmul"))
  "h3_alu")

;; Population count (Zbb): 1-cycle latency, same as other ALU ops.
;; Separated for the same reason as clmul above.
(define_insn_reservation "hazard3_cpop" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "cpop"))
  "h3_alu")

;; Integer divide and remainder: 17 cycles (unsigned) to 19 (signed, worst
;; case for sign correction).  Model as 17 for scheduling; the small
;; underestimate for signed div/rem has no effect on correctness of the
;; schedule, only on cycle estimates.
(define_insn_reservation "hazard3_idiv" 17
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "idiv"))
  "h3_muldiv*17")

;; Atomics (A extension).  lr.w/sc.w are 1-2 cycles; AMOs are 4+ cycles
;; (issued as a paired exclusive read/write, 4 cycles per attempt).  Use
;; 4 as a conservative single-attempt latency for AMOs; lr/sc are
;; approximated by the same reservation since they are typically used in
;; sequences where the surrounding scheduling matters more than the
;; individual latency.
(define_insn_reservation "hazard3_atomic" 4
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "atomic"))
  "h3_alu*4")

;; Short forward branch ALU (used by the *mov<GPR:mode><X:mode>cc pattern
;; in sfb.md).  Hazard3 does not have the SFB target feature, so this
;; reservation is a placeholder for completeness -- the patterns are not
;; generated without TARGET_SFB_ALU.
(define_insn_reservation "hazard3_sfb_alu" 2
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "sfb_alu"))
  "h3_alu")

;; CSR access (Zicsr): 1-cycle, like other ALU ops.  Some CSR writes take
;; 3 cycles due to instruction-fetch flush ordering; those are not modelled
;; separately here as they are rare and have minimal scheduling impact.
(define_insn_reservation "hazard3_xfer" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "mfc,mtc"))
  "h3_alu")

;; Load data -> store data bypass.  A load feeds the data input of a
;; following store with no bubble, because the store consumes its data in
;; stage 3 (memory) -- the same stage in which the load produces it.  All
;; other load->consumer pairs see the 2-cycle latency above, matching the
;; load-use bubble.  riscv_store_data_bypass_p (riscv.cc) returns true iff
;; the consumer is a store whose data (not address) depends on the load.
(define_bypass 1 "hazard3_load" "hazard3_store" "riscv_store_data_bypass_p")

;; Catch-all reservation for every remaining "type" value; workaround for
;; what appears to be a bug in non-machine-specific code. Hazard3 is an
;; integer-only core and the FP/vector types below can never be emitted by
;; a -march that matches the hardware. They are reserved here solely so
;; that the DFA has an entry for every value of the "type" attribute, which
;; riscv_sched_variable_issue asserts via insn_has_dfa_reservation_p.
;;
;; Latency 1 on h3_alu matches what the mistyped soft-float moves actually
;; are (integer mv/lw/sw). For the genuinely unreachable vector/FP types
;; the cost is irrelevant.

(define_insn_reservation "hazard3_catchall" 1
  (and (eq_attr "tune" "hazard3")
       (eq_attr "type" "fadd,fcmp,fcvt,fcvt_f2i,fcvt_i2f,fdiv,fmadd,fmove,fmul,fpload,\
			fpstore,fsqrt,ghost,rdfrm,rdvl,rdvlenb,sf_vc,sf_vc_se,\
			sf_vfnrclip,sf_vqmacc,vaalu,vaesdf,vaesdm,vaesef,vaesem,vaeskf1,\
			vaeskf2,vaesz,vandn,vbrev,vbrev8,vclmul,vclmulh,vclz,vcompress,\
			vcpop,vctz,vector,vext,vfalu,vfclass,vfcmp,vfcvtftoi,vfcvtitof,\
			vfdiv,vfmerge,vfminmax,vfmov,vfmovfv,vfmovvf,vfmul,vfmuladd,\
			vfncvtbf16,vfncvtftof,vfncvtftoi,vfncvtitof,vfrecp,vfredo,\
			vfredu,vfsgnj,vfslide1down,vfslide1up,vfsqrt,vfwalu,vfwcvtbf16,\
			vfwcvtftof,vfwcvtftoi,vfwcvtitof,vfwmaccbf16,vfwmul,vfwmuladd,\
			vfwredo,vfwredu,vgather,vghsh,vgmul,vialu,vicalu,vicmp,vidiv,\
			vimerge,viminmax,vimov,vimovvx,vimovxv,vimul,vimuladd,vired,\
			vislide1down,vislide1up,viwalu,viwmul,viwmuladd,viwred,vlde,\
			vldff,vldm,vldox,vldr,vlds,vldux,vlsegde,vlsegdff,vlsegdox,\
			vlsegds,vlsegdux,vmalu,vmffs,vmidx,vmiota,vmov,vmpop,vmsfs,\
			vnclip,vnshift,vrev8,vrol,vror,vsalu,vsetvl,vsetvl_pre,vsha2ch,\
			vsha2cl,vsha2ms,vshift,vslidedown,vslideup,vsm3c,vsm3me,vsm4k,\
			vsm4r,vsmul,vssegte,vssegtox,vssegts,vssegtux,vsshift,vste,vstm,\
			vstox,vstr,vsts,vstux,vwsll,wrfrm,wrvxrm"))
  "h3_alu")
