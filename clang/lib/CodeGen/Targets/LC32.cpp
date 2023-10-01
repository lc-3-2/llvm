//===- LC32.cpp -----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// It seems we're not required to do a whole lot here. We're using the defaults
// for the ABI since we have only a minimal prior ABI to adhere to. The only
// thing we have to do is pass on function attributes.
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace clang;
using namespace clang::CodeGen;

namespace {

class LC32TargetCodeGenInfo : public TargetCodeGenInfo {
public:
  LC32TargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<DefaultABIInfo>(CGT)) {}
  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &M) const override;
};

} // namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createLC32TargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<LC32TargetCodeGenInfo>(CGM.getTypes());
}

void LC32TargetCodeGenInfo::setTargetAttributes(
    const Decl *D, llvm::GlobalValue *GV, CodeGen::CodeGenModule &M) const {
  // This was copued from AVRTargetCodeGenInfo and M68kTargetCodeGenInfo

  // Treat definitions as the source of truth. All the attributes have to do
  // with the implementation, after all.
  if (GV->isDeclaration())
    return;

  // Handle function attributes
  if (const auto *FD = dyn_cast_or_null<FunctionDecl>(D)) {
    auto *Fn = cast<llvm::Function>(GV);

    if (const auto *Attr = FD->getAttr<LC32UseR4Attr>())
      Fn->addFnAttr("use_r4", Attr->getRequest() ? "true" : "false");
    if (const auto *Attr = FD->getAttr<LC32UseR7Attr>())
      Fn->addFnAttr("use_r7", Attr->getRequest() ? "true" : "false");

    if (FD->getAttr<LC32UnsafeCMPAttr>())
      Fn->addFnAttr("unsafe_cmp");
    if (FD->getAttr<LC32UnsafeScavengingAttr>())
      Fn->addFnAttr("unsafe_scavenging");
  }
}
