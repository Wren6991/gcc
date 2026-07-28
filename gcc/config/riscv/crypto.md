;; Machine description for RISC-V Scalar Cryptography extensions.
;; Copyright (C) 2023-2026 Free Software Foundation, Inc.

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

(define_c_enum "unspec" [
    ;; Zbkb unspecs
    UNSPEC_BREV8
    UNSPEC_ZIP
    UNSPEC_UNZIP
    UNSPEC_PACK
    UNSPEC_PACKH
    UNSPEC_PACKW

    ;; Zbkx unspecs
    UNSPEC_XPERM8
    UNSPEC_XPERM4

    ;; Zknd unspecs
    UNSPEC_AES_DSI
    UNSPEC_AES_DSMI
    UNSPEC_AES_DS
    UNSPEC_AES_DSM
    UNSPEC_AES_IM
    UNSPEC_AES_KS1I
    UNSPEC_AES_KS2

    ;; Zkne unspecs
    UNSPEC_AES_ES
    UNSPEC_AES_ESM
    UNSPEC_AES_ESI
    UNSPEC_AES_ESMI

    ;; Zknh unspecs
    UNSPEC_SHA_256_SIG0
    UNSPEC_SHA_256_SIG1
    UNSPEC_SHA_256_SUM0
    UNSPEC_SHA_256_SUM1
    UNSPEC_SHA_512_SIG0
    UNSPEC_SHA_512_SIG0H
    UNSPEC_SHA_512_SIG0L
    UNSPEC_SHA_512_SIG1
    UNSPEC_SHA_512_SIG1H
    UNSPEC_SHA_512_SIG1L
    UNSPEC_SHA_512_SUM0
    UNSPEC_SHA_512_SUM0R
    UNSPEC_SHA_512_SUM1
    UNSPEC_SHA_512_SUM1R

    ;; Zksh unspecs
    UNSPEC_SM3_P0
    UNSPEC_SM3_P1

    ;; Zksed unspecs
    UNSPEC_SM4_ED
    UNSPEC_SM4_KS
])

;; ZBKB extension
(define_insn "riscv_brev8_<mode>"
  [(set (match_operand:GPR 0 "register_operand" "=r")
        (unspec:GPR [(match_operand:GPR 1 "register_operand" "r")]
                  UNSPEC_BREV8))]
  "TARGET_ZBKB"
  "brev8\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_zip"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                  UNSPEC_ZIP))]
  "TARGET_ZBKB && !TARGET_64BIT"
  "zip\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_unzip"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                  UNSPEC_UNZIP))]
  "TARGET_ZBKB && !TARGET_64BIT"
  "unzip\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_pack_<X:mode><HISI:mode>"
  [(set (match_operand:X 0 "register_operand" "=r")
        (unspec:X [(match_operand:HISI 1 "register_operand" "r")
                  (match_operand:HISI 2 "register_operand" "r")]
                  UNSPEC_PACK))]
  "TARGET_ZBKB"
  "pack\t%0,%1,%2"
  [(set_attr "type" "crypto")])

;; This is slightly more complex than the other pack patterns
;; that fully expose the RTL as it needs to self-adjust to
;; rv32 and rv64.  But it's not that hard.
(define_insn "riscv_xpack_<X:mode>_<HX:mode>_2"
  [(set (match_operand:X 0 "register_operand" "=r")
	(ior:X (ashift:X (match_operand:X 1 "register_operand" "r")
			 (match_operand 2 "immediate_operand" "n"))
	       (zero_extend:X
		 (match_operand:HX 3 "register_operand" "r"))))]
  "TARGET_ZBKB && INTVAL (operands[2]) == BITS_PER_WORD / 2"
  "pack\t%0,%3,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_packh_<mode>"
  [(set (match_operand:X 0 "register_operand" "=r")
        (unspec:X [(match_operand:QI 1 "register_operand" "r")
                  (match_operand:QI 2 "register_operand" "r")]
                  UNSPEC_PACKH))]
  "TARGET_ZBKB"
  "packh\t%0,%1,%2"
  [(set_attr "type" "crypto")])

;; So this is both a useful pattern unto itself and a bridge to the
;; general packh pattern below.
(define_insn "*riscv_packh_<mode>_2"
  [(set (match_operand:X 0 "register_operand" "=r")
	(and:X (ashift:X (match_operand:X 1 "register_operand" "r")
			 (const_int 8))
	       (const_int 65280)))]
 "TARGET_ZBKB"
 "packh\t%0,x0,%1"
 [(set_attr "type" "crypto")])

;; While the two operands of the IOR could be swapped, this appears
;; to be the canonical form.  The other form doesn't seem to trigger.
(define_insn "*riscv_packh_<mode>_3"
  [(set (match_operand:X 0 "register_operand" "=r")
	(ior:X (and:X (ashift:X (match_operand:X 1 "register_operand" "r")
				(const_int 8))
		      (const_int 65280))
	       (zero_extend:X (match_operand:QI 2 "register_operand" "r"))))]
 "TARGET_ZBKB"
 "packh\t%0,%2,%1"
 [(set_attr "type" "crypto")])

(define_insn "riscv_packw"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:HI 1 "register_operand" "r")
                  (match_operand:HI 2 "register_operand" "r")]
                  UNSPEC_PACKW))]
  "TARGET_ZBKB && TARGET_64BIT"
  "packw\t%0,%1,%2"
  [(set_attr "type" "crypto")])

;; Implemented as a splitter for initial recognition.  It generates
;; new RTL with the extension moved to the outer position.  This
;; allows later code to eliminate subsequent explicit sign extensions.
(define_split
  [(set (match_operand:DI 0 "register_operand")
	(ior:DI (ashift:DI
		  (sign_extend:DI (match_operand:HI 1 "register_operand"))
		  (const_int 16))
		(zero_extend:DI (match_operand:HI 2 "register_operand"))))]
  "TARGET_ZBKB && TARGET_64BIT"
  [(set (match_dup 0)
	(sign_extend:DI (ior:SI (ashift:SI (match_dup 1) (const_int 16))
				(zero_extend:SI (match_dup 2)))))]
  "operands[1] = gen_lowpart (SImode, operands[1]);")

(define_split
  [(set (match_operand:DI 0 "register_operand")
	(ior:DI (zero_extend:DI (match_operand:HI 1 "register_operand"))
		(ashift:DI
		  (sign_extend:DI (match_operand:HI 2 "register_operand"))
		  (const_int 16))))]
  "TARGET_ZBKB && TARGET_64BIT"
  [(set (match_dup 0)
	(sign_extend:DI (ior:SI (ashift:SI (match_dup 2) (const_int 16))
				(zero_extend:SI (match_dup 1)))))]
  "operands[2] = gen_lowpart (SImode, operands[2]);")

(define_split
  [(set (match_operand:DI 0 "register_operand")
	(ior:DI (sign_extend:DI
		  (ashift:SI (match_operand:SI 1 "register_operand")
			     (const_int 16)))
		(zero_extend:DI (match_operand:HI 2 "register_operand"))))]
  "TARGET_ZBKB && TARGET_64BIT"
  [(set (match_dup 0)
	(sign_extend:DI (ior:SI (ashift:SI (match_dup 1) (const_int 16))
				(zero_extend:SI (match_dup 2)))))])

(define_split
  [(set (match_operand:DI 0 "register_operand")
	(ior:DI (zero_extend:DI (match_operand:HI 1 "register_operand"))
		(sign_extend:DI
		  (ashift:SI (match_operand:SI 2 "register_operand")
			     (const_int 16)))))]
  "TARGET_ZBKB && TARGET_64BIT"
  [(set (match_dup 0)
	(sign_extend:DI (ior:SI (ashift:SI (match_dup 2) (const_int 16))
				(zero_extend:SI (match_dup 1)))))])

;; And this patches the result of the splitter above.
(define_insn "*riscv_packw_2"
  [(set (match_operand:DI 0 "register_operand" "=r")
	(sign_extend:DI
	  (ior:SI
	    (ashift:SI (match_operand:SI 1 "register_operand" "r")
		       (const_int 16))
	    (zero_extend:SI (match_operand:HI 2 "register_operand" "r")))))]
  "TARGET_ZBKB && TARGET_64BIT"
  "packw\t%0,%2,%1"
  [(set_attr "type" "crypto")])

;; Unaligned SI load expanded using Zbkb pack/packh on little-endian RV32.
;;
;; The generic misaligned-load in expr.cc emits 4x lbu + 3x slli + 3x or (for a
;; 32-bit load on RV32), and combine cannot easily lower the result to
;; packh + pack.  Intercept the load and emit the optimal sequence directly.
;;
;; This routine handles *all* cases (including: RV64, big-endian, stores), since
;; there is no suitable hook to communicate detailed applicability information
;; to the caller. When no optimised expansion is available, it falls back to the
;; generic bitfield helpers for correctness.

(define_expand "movmisalignsi"
  [(set (match_operand:SI 0 "nonimmediate_operand")
	(match_operand:SI 1 "general_operand"))]
  ""
{
  bool store_p = MEM_P (operands[0]);
  rtx mem = store_p ? operands[0] : operands[1];
  rtx reg = store_p ? operands[1] : operands[0];

  /* Optimised RV32 little-endian load using Zbkb packh/pack.  */
  if (!store_p
      && TARGET_ZBKB
      && !TARGET_64BIT
      && !BYTES_BIG_ENDIAN
      && MEM_P (mem))
    {
      /* RV32 LE, hword-aligned word load: 2x lhu + pack */
      if (MEM_ALIGN (mem) >= 16)
	{
	  rtx h0 = gen_reg_rtx (SImode);
	  rtx h1 = gen_reg_rtx (SImode);
	  emit_insn (gen_zero_extendhisi2
	    (h0, adjust_address (mem, HImode, 0)));
	  emit_insn (gen_zero_extendhisi2
	    (h1, adjust_address (mem, HImode, 2)));
	  emit_insn (gen_riscv_pack_sihi (operands[0],
					  gen_lowpart (HImode, h0),
					  gen_lowpart (HImode, h1)));
	  DONE;
	}

      /* RV32 LE, byte-aligned word load: 4x lbu + 2x packh + pack.  packh is
         used to pair even/odd bytes, pack to pair even/odd halfwords. */
      rtx b[4];
      for (int i = 0; i < 4; ++i)
	{
	  rtx m = adjust_address (mem, QImode, i);
	  b[i] = gen_reg_rtx (SImode);
	  emit_insn (gen_zero_extendqisi2 (b[i], m));
	}
      rtx h0 = gen_reg_rtx (SImode);
      rtx h1 = gen_reg_rtx (SImode);
      emit_insn (gen_riscv_packh_si (h0,
				     gen_lowpart (QImode, b[0]),
				     gen_lowpart (QImode, b[1])));
      emit_insn (gen_riscv_packh_si (h1,
				     gen_lowpart (QImode, b[2]),
				     gen_lowpart (QImode, b[3])));
      emit_insn (gen_riscv_pack_sihi (operands[0],
				      gen_lowpart (HImode, h0),
				      gen_lowpart (HImode, h1)));
      DONE;
    }

  /* Safe fallback for all other cases: (RV64 || BE || store || non-SI).
     Delegate to the same generic bitfield routines as are used by
     movmisalign call sites when optab_handler returns nothing. */
  if (store_p)
    store_bit_field (mem, GET_MODE_BITSIZE (SImode), 0, 0, 0,
		     SImode, reg, false, false);
  else
    {
      rtx result = extract_bit_field (mem, GET_MODE_BITSIZE (SImode), 0,
				       1, NULL_RTX, SImode, SImode, false, NULL);
      emit_move_insn (reg, result);
    }
  DONE;
})

;; Unaligned HI load expanded using Zbkb packh on little-endian RV32.
;;
;; Similar to movmisalignsi above: replace byte-aligned halfword load with a
;; pattern that is compatible with our packh combine. Don't go directly to
;; UNSPEC packh, as that would prevent propagation of known-zero bits and
;; produce redundant zext.h instructions.

(define_expand "movmisalignhi"
  [(set (match_operand:HI 0 "nonimmediate_operand")
	(match_operand:HI 1 "general_operand"))]
  ""
{
  bool store_p = MEM_P (operands[0]);
  rtx mem = store_p ? operands[0] : operands[1];
  rtx reg = store_p ? operands[1] : operands[0];

  /* Optimised RV32 little-endian load using Zbkb packh.  */
  if (!store_p
      && TARGET_ZBKB
      && !TARGET_64BIT
      && !BYTES_BIG_ENDIAN
      && MEM_P (mem))
    {
      /* RV32 LE, byte-aligned halfword load: 2x lbu + packh.  */
      rtx b[2];
      for (int i = 0; i < 2; ++i)
	{
	  rtx m = adjust_address (mem, QImode, i);
	  b[i] = gen_reg_rtx (SImode);
	  emit_insn (gen_zero_extendqisi2 (b[i], m));
	}
      /* Emit the packh in its exposed (non-UNSPEC) form, matching
	 *riscv_packh_<mode>_3. */
      rtx h0 = gen_reg_rtx (SImode);
      emit_insn (gen_rtx_SET (h0,
	gen_rtx_IOR (SImode,
	  gen_rtx_AND (SImode,
	    gen_rtx_ASHIFT (SImode, b[1], GEN_INT (8)),
	    GEN_INT (65280)),
	  gen_rtx_ZERO_EXTEND (SImode,
	    gen_lowpart (QImode, b[0])))));
      emit_move_insn (operands[0], gen_lowpart (HImode, h0));
      DONE;
    }

  /* Safe fallback for all other cases: (RV64 || BE || store || aligned).
     Delegate to the same generic bitfield routines as are used by
     movmisalign call sites when optab_handler returns nothing. */
  if (store_p)
    store_bit_field (mem, GET_MODE_BITSIZE (HImode), 0, 0, 0,
		     HImode, reg, false, false);
  else
    {
      rtx result = extract_bit_field (mem, GET_MODE_BITSIZE (HImode), 0,
				       1, NULL_RTX, HImode, HImode, false, NULL);
      emit_move_insn (reg, result);
    }
  DONE;
})

;; ZBKX extension

(define_insn "riscv_xperm4_<mode>"
  [(set (match_operand:X 0 "register_operand" "=r")
        (unspec:X [(match_operand:X 1 "register_operand" "r")
                  (match_operand:X 2 "register_operand" "r")]
                  UNSPEC_XPERM4))]
  "TARGET_ZBKX"
  "xperm4\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_xperm8_<mode>"
  [(set (match_operand:X 0 "register_operand" "=r")
        (unspec:X [(match_operand:X 1 "register_operand" "r")
                  (match_operand:X 2 "register_operand" "r")]
                  UNSPEC_XPERM8))]
  "TARGET_ZBKX"
  "xperm8\t%0,%1,%2"
  [(set_attr "type" "crypto")])

;; ZKND extension

(define_insn "riscv_aes32dsi"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")
                   (match_operand:SI 3 "const_0_3_operand" "")]
                   UNSPEC_AES_DSI))]
  "TARGET_ZKND && !TARGET_64BIT"
  "aes32dsi\t%0,%1,%2,%3"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes32dsmi"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")
                   (match_operand:SI 3 "const_0_3_operand" "")]
                   UNSPEC_AES_DSMI))]
  "TARGET_ZKND && !TARGET_64BIT"
  "aes32dsmi\t%0,%1,%2,%3"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes64ds"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")
                   (match_operand:DI 2 "register_operand" "r")]
                   UNSPEC_AES_DS))]
  "TARGET_ZKND && TARGET_64BIT"
  "aes64ds\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes64dsm"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")
                   (match_operand:DI 2 "register_operand" "r")]
                   UNSPEC_AES_DSM))]
  "TARGET_ZKND && TARGET_64BIT"
  "aes64dsm\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes64im"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")]
                   UNSPEC_AES_IM))]
  "TARGET_ZKND && TARGET_64BIT"
  "aes64im\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes64ks1i"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")
                   (match_operand:SI 2 "const_0_10_operand" "")]
                   UNSPEC_AES_KS1I))]
  "(TARGET_ZKND || TARGET_ZKNE) && TARGET_64BIT"
  "aes64ks1i\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes64ks2"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")
                   (match_operand:DI 2 "register_operand" "r")]
                   UNSPEC_AES_KS2))]
  "(TARGET_ZKND || TARGET_ZKNE) && TARGET_64BIT"
  "aes64ks2\t%0,%1,%2"
  [(set_attr "type" "crypto")])

;; ZKNE extension

(define_insn "riscv_aes32esi"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")
                   (match_operand:SI 3 "const_0_3_operand" "")]
                   UNSPEC_AES_ESI))]
  "TARGET_ZKNE && !TARGET_64BIT"
  "aes32esi\t%0,%1,%2,%3"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes32esmi"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")
                   (match_operand:SI 3 "const_0_3_operand" "")]
                   UNSPEC_AES_ESMI))]
  "TARGET_ZKNE && !TARGET_64BIT"
  "aes32esmi\t%0,%1,%2,%3"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes64es"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")
                   (match_operand:DI 2 "register_operand" "r")]
                   UNSPEC_AES_ES))]
  "TARGET_ZKNE && TARGET_64BIT"
  "aes64es\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_aes64esm"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")
                   (match_operand:DI 2 "register_operand" "r")]
                   UNSPEC_AES_ESM))]
  "TARGET_ZKNE && TARGET_64BIT"
  "aes64esm\t%0,%1,%2"
  [(set_attr "type" "crypto")])

;; ZKNH - SHA256

(define_int_iterator SHA256_OP [
  UNSPEC_SHA_256_SIG0 UNSPEC_SHA_256_SIG1
  UNSPEC_SHA_256_SUM0 UNSPEC_SHA_256_SUM1])
(define_int_attr sha256_op [
  (UNSPEC_SHA_256_SIG0 "sha256sig0") (UNSPEC_SHA_256_SIG1 "sha256sig1")
  (UNSPEC_SHA_256_SUM0 "sha256sum0") (UNSPEC_SHA_256_SUM1 "sha256sum1")])

(define_insn "*riscv_<sha256_op>_si"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                   SHA256_OP))]
  "TARGET_ZKNH && !TARGET_64BIT"
  "<sha256_op>\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_<sha256_op>_di_extended"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (sign_extend:DI
             (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                        SHA256_OP)))]
  "TARGET_ZKNH && TARGET_64BIT"
  "<sha256_op>\t%0,%1"
  [(set_attr "type" "crypto")])

(define_expand "riscv_<sha256_op>_si"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                   SHA256_OP))]
  "TARGET_ZKNH"
  {
    if (TARGET_64BIT)
      {
        rtx t = gen_reg_rtx (DImode);
        emit_insn (gen_riscv_<sha256_op>_di_extended (t, operands[1]));
        t = gen_lowpart (SImode, t);
        SUBREG_PROMOTED_VAR_P (t) = 1;
        SUBREG_PROMOTED_SET (t, SRP_SIGNED);
        emit_move_insn (operands[0], t);
        DONE;
      }
  }
  [(set_attr "type" "crypto")])

;; ZKNH - SHA512

(define_insn "riscv_sha512sig0h"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")]
                   UNSPEC_SHA_512_SIG0H))]
  "TARGET_ZKNH && !TARGET_64BIT"
  "sha512sig0h\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sig0l"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")]
                   UNSPEC_SHA_512_SIG0L))]
  "TARGET_ZKNH && !TARGET_64BIT"
  "sha512sig0l\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sig1h"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")]
                   UNSPEC_SHA_512_SIG1H))]
  "TARGET_ZKNH && !TARGET_64BIT"
  "sha512sig1h\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sig1l"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")]
                   UNSPEC_SHA_512_SIG1L))]
  "TARGET_ZKNH && !TARGET_64BIT"
  "sha512sig1l\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sum0r"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")]
                   UNSPEC_SHA_512_SUM0R))]
  "TARGET_ZKNH && !TARGET_64BIT"
  "sha512sum0r\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sum1r"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")]
                   UNSPEC_SHA_512_SUM1R))]
  "TARGET_ZKNH && !TARGET_64BIT"
  "sha512sum1r\t%0,%1,%2"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sig0"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")]
                   UNSPEC_SHA_512_SIG0))]
  "TARGET_ZKNH && TARGET_64BIT"
  "sha512sig0\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sig1"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")]
                   UNSPEC_SHA_512_SIG1))]
  "TARGET_ZKNH && TARGET_64BIT"
  "sha512sig1\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sum0"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")]
                   UNSPEC_SHA_512_SUM0))]
  "TARGET_ZKNH && TARGET_64BIT"
  "sha512sum0\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_sha512sum1"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (unspec:DI [(match_operand:DI 1 "register_operand" "r")]
                   UNSPEC_SHA_512_SUM1))]
  "TARGET_ZKNH && TARGET_64BIT"
  "sha512sum1\t%0,%1"
  [(set_attr "type" "crypto")])

 ;; ZKSH

(define_int_iterator SM3_OP [UNSPEC_SM3_P0 UNSPEC_SM3_P1])
(define_int_attr sm3_op [(UNSPEC_SM3_P0 "sm3p0") (UNSPEC_SM3_P1 "sm3p1")])

(define_insn "*riscv_<sm3_op>_si"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                   SM3_OP))]
  "TARGET_ZKSH && !TARGET_64BIT"
  "<sm3_op>\t%0,%1"
  [(set_attr "type" "crypto")])

(define_insn "riscv_<sm3_op>_di_extended"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (sign_extend:DI
             (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                        SM3_OP)))]
  "TARGET_ZKSH && TARGET_64BIT"
  "<sm3_op>\t%0,%1"
  [(set_attr "type" "crypto")])

(define_expand "riscv_<sm3_op>_si"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")]
                   SM3_OP))]
  "TARGET_ZKSH"
  {
    if (TARGET_64BIT)
      {
        rtx t = gen_reg_rtx (DImode);
        emit_insn (gen_riscv_<sm3_op>_di_extended (t, operands[1]));
        t = gen_lowpart (SImode, t);
        SUBREG_PROMOTED_VAR_P (t) = 1;
        SUBREG_PROMOTED_SET (t, SRP_SIGNED);
        emit_move_insn (operands[0], t);
        DONE;
      }
  }
  [(set_attr "type" "crypto")])

;; ZKSED

(define_int_iterator SM4_OP [UNSPEC_SM4_ED UNSPEC_SM4_KS])
(define_int_attr sm4_op [(UNSPEC_SM4_ED "sm4ed") (UNSPEC_SM4_KS "sm4ks")])

(define_insn "*riscv_<sm4_op>_si"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")
                   (match_operand:SI 3 "const_0_3_operand" "")]
                   SM4_OP))]
  "TARGET_ZKSED && !TARGET_64BIT"
  "<sm4_op>\t%0,%1,%2,%3"
  [(set_attr "type" "crypto")])

(define_insn "riscv_<sm4_op>_di_extended"
  [(set (match_operand:DI 0 "register_operand" "=r")
        (sign_extend:DI
             (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                        (match_operand:SI 2 "register_operand" "r")
                        (match_operand:SI 3 "const_0_3_operand" "")]
                        SM4_OP)))]
  "TARGET_ZKSED && TARGET_64BIT"
  "<sm4_op>\t%0,%1,%2,%3"
  [(set_attr "type" "crypto")])

(define_expand "riscv_<sm4_op>_si"
  [(set (match_operand:SI 0 "register_operand" "=r")
        (unspec:SI [(match_operand:SI 1 "register_operand" "r")
                   (match_operand:SI 2 "register_operand" "r")
                   (match_operand:SI 3 "const_0_3_operand" "")]
                   SM4_OP))]
  "TARGET_ZKSED"
  {
    if (TARGET_64BIT)
      {
        rtx t = gen_reg_rtx (DImode);
        emit_insn (gen_riscv_<sm4_op>_di_extended (t, operands[1], operands[2], operands[3]));
        t = gen_lowpart (SImode, t);
        SUBREG_PROMOTED_VAR_P (t) = 1;
        SUBREG_PROMOTED_SET (t, SRP_SIGNED);
        emit_move_insn (operands[0], t);
        DONE;
      }
  }
  [(set_attr "type" "crypto")])
