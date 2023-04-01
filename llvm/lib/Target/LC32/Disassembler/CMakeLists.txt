add_llvm_component_library(LLVMLC32Disassembler
  LC32DisassemblerRegistration.cpp
  LC32DisassemblerImpl.cpp

  LINK_COMPONENTS
  MCDisassembler
  MC
  Support
  LC32Info

  ADD_TO_COMPONENT
  LC32
  )
