find_package(LLVM 13 REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(LLVM_LINK_COMPONENTS
    Analysis
	BitReader
	Core
	ExecutionEngine
	InstCombine
	Interpreter
	MC
	MCDisassembler
	MCJIT
	Object
	OrcJIT
	RuntimeDyld
	ScalarOpts
	Support
	Target
	TransformUtils
	native
	nativecodegen
	AsmPrinter
	AArch64Info
	AArch64Desc
	AArch64AsmParser
	AArch64Disassembler
	AArch64CodeGen
	ARMInfo
	ARMDesc
	ARMAsmParser
	ARMDisassembler
	ARMCodeGen
	AMDGPUInfo
	AMDGPUAsmParser
	AMDGPUCodeGen
	AVRAsmParser
	AVRInfo
	AVRDesc
	AVRCodeGen
	BPFInfo
	BPFAsmParser
	BPFDisassembler
	BPFCodeGen
	HexagonInfo
	HexagonDesc
	HexagonAsmParser
	HexagonDisassembler
	HexagonCodeGen
	LanaiInfo
	LanaiDesc
	LanaiAsmParser
	LanaiDisassembler
	LanaiCodeGen
	MSP430Info
	MSP430Desc
	MSP430CodeGen
	MipsInfo
	MipsDesc
	MipsAsmParser
	MipsDisassembler
	MipsCodeGen
	NVPTXInfo
	NVPTXCodeGen
	PowerPCInfo
	PowerPCDesc
	PowerPCAsmParser
	PowerPCDisassembler
	PowerPCCodeGen
	SparcInfo
	SparcDesc
	SparcAsmParser
	SparcDisassembler
	SparcCodeGen
	SystemZInfo
	SystemZDesc
	SystemZAsmParser
	SystemZDisassembler
	SystemZCodeGen
	WebAssemblyInfo
	WebAssemblyDesc
	WebAssemblyAsmParser
	WebAssemblyDisassembler
	WebAssemblyCodeGen
	X86Info
	X86Desc
	X86AsmParser
	X86Disassembler
	X86CodeGen
	XCoreInfo
	XCoreDesc
	XCoreDisassembler
	XCoreCodeGen
)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

llvm_map_components_to_libnames(llvm_libs support core irreader ${LLVM_LINK_COMPONENTS})

add_library(llvm INTERFACE)
target_link_libraries(llvm INTERFACE ${llvm_libs})