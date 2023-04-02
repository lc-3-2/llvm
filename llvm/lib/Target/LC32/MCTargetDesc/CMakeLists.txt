add_llvm_component_library(LLVMLC32Desc
  LC32MCTargetDesc.cpp
  LC32MCAsmInfo.cpp
  LC32InstPrinter.cpp
  LC32ELFObjectTargetWriter.cpp

  LINK_COMPONENTS
  MC
  Support
  LC32Info

  ADD_TO_COMPONENT
  LC32
  )
