;;===-- mul.S - Implement multiplication ----------------------------------===;;
;;
;; Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
;; See https://llvm.org/LICENSE.txt for license information.
;; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
;;
;;===----------------------------------------------------------------------===;;
;;
;; Implement multiplication libcall. This has to be done in assembly since
;; otherwise the compiler figures out what we're doing and optimizes it to a
;; libcall, leading to infinite recursion.
;;
;; The algorithm used here is a variation of "square-and-multiply", except we
;; double instead of squaring and we add instead of multiplying - i.e. we
;; operate in "log space". The code is
;; ```
;; unsigned __mulsi3(unsigned x, unsigned y) {
;;   unsigned t;
;;   unsigned r = 0;
;;   while (x != 0) {
;;     t = x & 1;
;;     if (t != 0)
;;       r += y;
;;     x >>= 1;
;;     y <<= 1;
;;   }
;;   return r;
;; }
;; ```
;; Register allocation is:
;; * R0: r
;; * R1: x
;; * R2: y
;; * R3: t
;; Remember that branches may clobber R3, so we can only really use it for
;; temporaries.
;;
;;===----------------------------------------------------------------------===;;

	.section .text.__mulsi3, "ax", @progbits
	.align 2

	.global __mulsi3
	.type __mulsi3, @function

__mulsi3:

	; Stack setup
	; No locals
	STW     R7, R6, -2
	STW     R5, R6, -3
	ADD     R5, R6, -16
	ADD     R6, R5, 0

	; Load parameters into registers
	LDW     R1, R5, 4
	LDW     R2, R5, 5

	; unsigned r = 0;
	AND     R0, R0, 0

	; while (x != 0) {
.Lwhile:

	; Check x != 0
	ADD     R1, R1, 0
	BRz     .Lendwhile

	; unsigned t = x & 1;
	AND     R3, R1, 0x1

	; if (t != 0) {
	BRz     .Lendif

	; r += y;
	ADD     R0, R0, R2

	; }
.Lendif:

	; x >>= 1;
	RSHFL   R1, R1, 1

	; y <<= 1;
	LSHF    R2, R2, 1

	; }
	BR      .Lwhile
.Lendwhile:

	; Stack teardown
	; return r;
	STW     R0, R5, 3
	ADD     R6, R5, 12
	LDW     R7, R6, -1
	LDW     R5, R6, -2
	RET
