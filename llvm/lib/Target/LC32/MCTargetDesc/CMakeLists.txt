add_llvm_component_library(LLVMLC32Desc
  LC32MCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  Support
  LC32Info

  ADD_TO_COMPONENT
  LC32
  )
