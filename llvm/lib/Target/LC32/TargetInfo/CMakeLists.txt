add_llvm_component_library(LLVMLC32Info
  LC32TargetInfo.cpp

  LINK_COMPONENTS
  CodeGenTypes
  MC
  Support

  ADD_TO_COMPONENT
  LC32
  )
