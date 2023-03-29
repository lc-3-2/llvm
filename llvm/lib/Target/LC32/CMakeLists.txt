add_llvm_component_group(LC32)

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
