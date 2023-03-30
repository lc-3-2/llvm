//===- LC32Schedule.td - Describe Implementations ----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This section would usually be used the specify different implementations of
// the architecture, which may have different properties for instruction
// scheduling. We only have the one.
//
// Also, we don't handle instruction scheduling. It doesn't make much sense to
// do so since it is a microcoded implementation.
//
// Usually: LC32Schedule.td and LC32.td
//
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic", NoSchedModel, []>;
