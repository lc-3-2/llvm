add_llvm_component_library(LLVMLC32AsmParser
  LC32AsmParserRegistration.cpp
  LC32AsmParserImpl.cpp

  LINK_COMPONENTS
  MCParser
  MC
  Support
  LC32Desc
  LC32Info

  ADD_TO_COMPONENT
  LC32
  )
