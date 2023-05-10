//===- LC32.cpp -----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Symbols.h"
#include "Target.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Endian.h"
using namespace llvm;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class LC32 final : public TargetInfo {
public:
  LC32();
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
};
} // namespace

TargetInfo *elf::getLC32TargetInfo() {
  static LC32 target;
  return &target;
}

LC32::LC32() {
  // Set relocation types
  this->symbolicRel = R_LC_3_2_32;
  // Set other parameters
  this->defaultImageBase = 0x30000000;
  this->trapInstr = {0xff, 0xf0, 0xff, 0xf0};
}

RelExpr LC32::getRelExpr(RelType type, const Symbol &s,
                         const uint8_t *loc) const {
  switch (type) {
  case R_LC_3_2_NONE:
    return R_NONE;
  case R_LC_3_2_32:
    return R_ABS;
  default:
    error(getErrorLocation(loc) + "unknown relocation (" + Twine(type) +
          ") against symbol " + toString(s));
    return R_NONE;
  }
}

void LC32::relocate(uint8_t *loc, const Relocation &rel, uint64_t val) const {
  switch (rel.type) {
  case R_LC_3_2_32:
    write32le(loc, val);
    return;
  default:
    llvm_unreachable("Unknown relocation");
  }
}
