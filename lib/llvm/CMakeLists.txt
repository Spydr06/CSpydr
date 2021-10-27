add_library(llvm-c INTERFACE)
target_include_directories(llvm-c INTERFACE include)
set_target_properties(llvm-c PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(llvm-c INTERFACE -lLLVM-12)