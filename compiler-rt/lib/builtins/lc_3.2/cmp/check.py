"""Validate the algorithm for comparisons

I thought it would be fun to formally validate the algorithms we're using for
signed and unsigned comparisons, and in doing that I uncovered a bug. Therefore,
I'm keeping this file around to prevent future bugs. Either way, reasoning about
overflow is hard and this does that for us.

This file tests comparison algorithms and validates them against the actual
definitions. It checks to see if the algorithms we're using work or if they fail
on some inputs.

The algorithms we use should:
* return a positive number iff `lhs > rhs`
* return a negative number iff `lhs < rhs`
* return zero iff `lhs == rhs`
where comparisons are either signed or unsigned.

Note that this file only checks that we "did the right thing", not that we "did
the thing right". The implementation may still be wrong. As such, this file
should be kept up to date with the code in `cmp.S` in `compiler-rt` and with
that in `LC32ISelLoweringOps.cpp` in `llvm`.
"""

from z3 import *

def TEST(name: str, solver: Solver, width: int = 10):
    """Utility function to flag test cases

    This method calls `check` on the solver and asserts that the result is
    UNSAT. The solver should've already been given all the properties we want to
    check.

    As for why we check for UNSAT: Usually, we want to prove universal statement
    `P`. To do that, we give the solver `Not(P)` and ask it to find an example.
    If it finds one, that's a counterexample to `P`. If it can't find one, then
    `P` holds universally.

    :param name: Human-readable name of the test
    :param solver: Z3 solver to `check`
    :param width: Width of string to print
    """
    if solver.check() == unsat:
        print(f"{name: <{width}}: PASS")
    else:
        print(f"{name: <{width}}: FAIL - {solver.model()}")

lhs, rhs = BitVecs('lhs rhs', 32)
"""32-bit integers for the comparison"""

#-------------------------------------------------------------------------------
# Signed Comparison

def my_scmp(lhs, rhs):
    return If(lhs ^ rhs < 0, lhs | 1, lhs - rhs)

scmp = Solver()
scmp.add(Not(And(
    (lhs < rhs) == (my_scmp(lhs, rhs) < 0),
    (lhs > rhs) == (my_scmp(lhs, rhs) > 0),
    (lhs == rhs) == (my_scmp(lhs, rhs) == 0))))
TEST("SIGNED", scmp)

#-------------------------------------------------------------------------------
# Unsigned Comparison

def my_ucmp(lhs, rhs):
    return If(lhs ^ rhs < 0, rhs | 1, lhs - rhs)

ucmp = Solver()
ucmp.add(Not(And(
    ULT(lhs, rhs) == (my_ucmp(lhs, rhs) < 0),
    UGT(lhs, rhs) == (my_ucmp(lhs, rhs) > 0),
    (lhs == rhs) == (my_ucmp(lhs, rhs) == 0))))
TEST("UNSIGNED", ucmp)

#-------------------------------------------------------------------------------
# Or With 1
#
# Doing a bitwise-or with 1 is difficult on the LC-3. Therefore, we propose a
# faster way to do it that's equivalent. We can use that in the above algorithms
# with no change to correctness.

def my_or1(x):
    return (x & -2) + 1

or1 = Solver()
or1.add(Not(And(
    (lhs | 1) == my_or1(lhs),
    (rhs | 1) == my_or1(rhs))))
TEST("OR1", or1)
