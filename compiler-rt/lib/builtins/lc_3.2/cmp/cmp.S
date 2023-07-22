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
;; The logic in these functions are now generated inline, so these functions are
;; never called. They're kept here for convenience. Their logic should be
;; reflected in `LC32TargetLowering`.
;;
;; See: llvm/lib/Target/LC32/LC32ISelLoweringOps.cpp LC32TargetLowering::DoCmp
;; See: check.py
;;
;;===----------------------------------------------------------------------===;;

	.section .text.__cmpsi3, "ax", @progbits
	.align 2

; We have two cases here. In the first case, the operands have the same sign.
; Then, we can subtract without risk of overflow.
;
; In the second case, the operands have different signs. In that case, we want
; to return something that shares the sign of the LHS. To see this, break into
; cases:
; * If the LHS is negative, then LHS < RHS, and we need to return something
;   negative
; * If the LHS is positive, then LHS > RHS, and we need to return something
;   positive
;
; We can't just return the LHS since there is an edge case when LHS = 0. Then,
; we'll falsely report equality. Thus, we OR the LHS with 1 before returning.
;
; The code is:
; ```
; int32_t __cmpsi3(int32_t %lhs, int32_t %rhs) {
;   %det = %lhs ^ %rhs
;   if (%det < 0) {
;     %t0 = %lhs & -2
;     %retn = %t0 + 1
;   } else {
;     %t1 = ~%rhs
;     %t2 = %t1 + 1
;     %retp = %lhs + %t2
;   }
;   %ret = PHI %retn %retp
;   return %ret
; }
; ```
; Register allocation is:
; * R0: %ret %retn %retp %t0 %t1 %t2 %det
; * R1: %lhs
; * R2: %rhs
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
	LDW     R1, R5, 4
	LDW     R2, R5, 5

	; %det = %lhs ^ %rhs
	XOR     R0, R1, R2

	; if (%det < 0) {
	BRzp    .Ls_else

	; Set %retn = %lhs | 1
	; %t0 = %lhs & -2
	; %retn = %t0 + 1
	AND     R0, R1, -2
	ADD     R0, R0, 1

	; } else {
	BR      .Ls_endif
.Ls_else:

	; Set %retp = %lhs - %rhs
	; %t1 = ~%rhs
	; %t2 = %t1 + 1
	; %retp = %lhs + %t2
	NOT     R0, R2
	ADD     R0, R0, 1
	ADD     R0, R1, R0

	; }
.Ls_endif:

	; Stack teardown
	; %ret = PHI %retn %retp
	; return %ret
	STW     R0, R5, 3
	ADD     R6, R5, 12
	LDW     R7, R6, -1
	LDW     R5, R6, -2
	RET

;---

	.section .text.__ucmpsi3, "ax", @progbits
	.align 2

; Very similar idea to `__cmpsi3`. Again, we have two cases. If the two
; arguments have the same sign, this means they are in the same half of the
; range of unsigned integers. Subtracting them will result in a number of
; magnitude at most `2**31 - 1`, so we can subtract without worrying about
; overflow. If the two arguments have different signs, this means they are in
; different halves of the range, and we want to return the same sign as the RHS.
;
; The code is:
; ```
; int32_t __ucmpsi3(uint32_t lhs, uint32_t rhs) {
;   %det = lhs ^ rhs
;   if (%det < 0) {
;     %t0 = %rhs & -2
;     %retn = %t0 + 1
;   } else {
;     %t1 = ~%rhs
;     %t2 = %t1 + 1
;     %retp = %lhs + %t2
;   }
;   %ret = PHI %retn %retp
;   return %ret
; }
; ```
; Register allocation is:
; * R0: %ret %retn %retp %t0 %t1 %t2 %det
; * R1: %lhs
; * R2: %rhs
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
	LDW     R1, R5, 4
	LDW     R2, R5, 5

	; %det = %lhs ^ %rhs
	XOR     R0, R1, R2

	; if (%det < 0) {
	BRzp    .Lu_else

	; Set %retn = %rhs | 1
	; %t0 = %rhs & -2
	; %retn = %t0 + 1
	AND     R0, R2, -2
	ADD     R0, R0, 1

	; } else {
	BR      .Lu_endif
.Lu_else:

	; Set %retp = %lhs - %rhs
	; %t1 = ~%rhs
	; %t2 = %t1 + 1
	; %retp = %lhs + %t2
	NOT     R0, R2
	ADD     R0, R0, 1
	ADD     R0, R1, R0

	; }
.Lu_endif:

	; Stack teardown
	; %ret = PHI %retn %retp
	; return %ret
	STW     R0, R5, 3
	ADD     R6, R5, 12
	LDW     R7, R6, -1
	LDW     R5, R6, -2
	RET
