from z3 import *

def TEST(name, solver, expected):
    if solver.check() == expected:
        print(f"{name}: PASS")
    else:
        print(f"{name}: FAIL - {solver.model()}")

lhs, rhs = BitVecs('lhs rhs', 32)

#-------------------------------------------------------------------------------

def my_scmp(lhs, rhs):
    return If(lhs ^ rhs < 0, lhs, lhs - rhs)

scmp = Solver()
scmp.add(Not(And(
    (lhs < rhs) == (my_scmp(lhs, rhs) < 0),
    (lhs > rhs) == (my_scmp(lhs, rhs) > 0),
    (lhs == rhs) == (my_scmp(lhs, rhs) == 0),
)))
TEST("SIGNED  ", scmp, unsat)

#-------------------------------------------------------------------------------

def my_ucmp(lhs, rhs):
    return If(lhs ^ rhs < 0, rhs, lhs - rhs)

ucmp = Solver()
ucmp.add(Not(And(
    ULT(lhs, rhs) == (my_ucmp(lhs, rhs) < 0),
    UGT(lhs, rhs) == (my_ucmp(lhs, rhs) > 0),
    (lhs == rhs) == (my_ucmp(lhs, rhs) == 0),
)))
TEST("UNSIGNED", ucmp, unsat)
