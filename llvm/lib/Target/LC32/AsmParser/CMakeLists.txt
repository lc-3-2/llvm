add_llvm_component_library(LLVMLC32AsmParser
  LC32AsmParserRegistration.cpp
  LC32AsmParserImpl.cpp
  operand/LC32OperandToken.cpp
  operand/LC32OperandReg.cpp
  operand/LC32OperandImm.cpp
  operand/LC32OperandExpr.cpp
  operand/LC32OperandNZP.cpp

  LINK_COMPONENTS
  CodeGenTypes
  MCParser
  MC
  Support
  LC32Desc
  LC32Info

  ADD_TO_COMPONENT
  LC32
  )
