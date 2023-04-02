add_llvm_component_group(LC32)

set(LLVM_TARGET_DEFINITIONS LC32.td)

tablegen(LLVM LC32GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM LC32GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM LC32GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM LC32GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM LC32GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM LC32GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM LC32GenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM LC32GenCallingConv.inc -gen-callingconv)
tablegen(LLVM LC32GenDAGISel.inc -gen-dag-isel)

add_public_tablegen_target(LC32CommonTableGen)

add_llvm_target(LC32CodeGen
  LC32TargetMachine.cpp

  LINK_COMPONENTS
  CodeGen
  Target
  Support
  LC32Info
  LC32Desc

  ADD_TO_COMPONENT
  LC32
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(Disassembler)
add_subdirectory(AsmParser)
