;;===-- cmp.S - Implement comparison functions ----------------------------===;;
;;
;; Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
;; See https://llvm.org/LICENSE.txt for license information.
;; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
;;
;;===----------------------------------------------------------------------===;;
;;
;; Libcalls for potentially overflowing comparisons. These functions are not
;; standard, but they are generated nonetheless. These should return zero if
;; equal, a negative result if less than, and a positive result if greater than.
;; The former does signed comparison while the latter does unsigned.
;;
;; See: llvm/lib/Target/LC32/LC32ISelLoweringOps.cpp LC32TargetLowering::DoCmp
;;
;;===----------------------------------------------------------------------===;;

	.section .text.__cmpsi3, "ax", @progbits
	.align 2

; The idea here is that we want to perform a normal subtraction, but we might
; overflow. Subtraction can overflow if the operands have different signs, so we
; handle that case separately. If they have different signs, we can just return
; the LHS of the comparison. Otherwise, we subtract. The code is:
; ```
; int __cmpsi3(unsigned lhs, unsigned rhs) {
;   int t;
;   int ret = lhs;
;   t = lhs ^ rhs
;   if (t >= 0) {
;     t = ~rhs;
;     t += 1;
;     ret += t;
;   }
;   return ret;
; }
; ```
; Register allocation is:
; * R0: ret, lhs
; * R1: rhs
; * R2: t
	.global __cmpsi3
	.type __cmpsi3, @function
__cmpsi3:

	; Stack setup
	; No locals
	STW     R7, R6, -2
	STW     R5, R6, -3
	ADD     R5, R6, -16
	ADD     R6, R5, 0

	; Load parameters into registers
	; unsigned ret = lhs;
	LDW     R0, R5, 4
	LDW     R1, R5, 5

	; t = lhs ^ rhs;
	XOR     R2, R0, R1

	; if (t >= 0) {
	BRn     .Lsendif

	; Does ret -= rhs Using t
	; t = ~rhs;
	; t += 1;
	; ret += t;
	NOT     R2, R1
	ADD     R2, R2, 1
	ADD     R0, R0, R2

	; }
.Lsendif:

	; Stack teardown
	; return ret;
	STW     R0, R5, 3
	ADD     R6, R5, 12
	LDW     R7, R6, -1
	LDW     R5, R6, -2
	RET

;---

	.section .text.__ucmpsi3, "ax", @progbits
	.align 2

; Very similar ideas to __cmpsi3. If the MSBs are different, the greater one has
; the MSB set. This corresponds to returning the RHS in that case. If the MSBs
; are the same, we bring them down into the lower half of the range, then do
; normal subtraction. Code is:
; ```
; int __ucmpsi3(unsigned lhs, unsigned rhs) {
;   int t;
;   int ret = rhs;
;   t = lhs ^ rhs;
;   if (t >= 0) {
;     lhs <<= 1;
;     lhs >>= 1;
;     rhs <<= 1;
;     rhs >>= 1;
;     t = ~rhs;
;     t += 1;
;     ret = lhs + t;
;   }
;   return ret;
; }
; ```
; Register allocation is:
; * R0: ret, rhs
; * R1: lhs
; * R2: t
	.global __ucmpsi3
	.type __ucmpsi3, @function
__ucmpsi3:

	; Stack setup
	; No locals
	STW     R7, R6, -2
	STW     R5, R6, -3
	ADD     R5, R6, -16
	ADD     R6, R5, 0

	; Load parameters into registers
	; unsigned ret = rhs;
	LDW     R1, R5, 4
	LDW     R0, R5, 5

	; t = lhs ^ rhs;
	XOR     R2, R0, R1

	; if (t >= 0) {
	BRn     .Luendif

	; lhs <<= 1;
	LSHF    R1, R1, 1
	; lhs >>= 1;
	RSHFL   R1, R1, 1

	; rhs <<= 1;
	LSHF    R0, R0, 1
	; rhs >>= 1;
	RSHFL   R0, R0, 1

	; Does ret = lhs - rhs Using t
	; t = ~rhs;
	; t += 1;
	; ret = lhs + t;
	NOT     R2, R0
	ADD     R2, R2, 1
	ADD     R0, R1, R2

	; }
.Luendif:

	; Stack teardown
	; return ret;
	STW     R0, R5, 3
	ADD     R6, R5, 12
	LDW     R7, R6, -1
	LDW     R5, R6, -2
	RET
