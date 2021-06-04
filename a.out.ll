; ModuleID = "examples/main.csp"
source_filename = "examples/main.csp"
target triple = "x86_64-pc-linux-gnu"

@global = external global i32

define i32 @main(i32 %argc, i8 addrspace(1)* addrspace(8)* %argv) {
entry:
  %0 = alloca i32, align 4
  store i32 %argc, i32* %0, align 4
  %1 = alloca i8 addrspace(1)* addrspace(8)*, align 8
  store i8 addrspace(1)* addrspace(8)* %argv, i8 addrspace(1)* addrspace(8)** %1, align 8
  ret i32 %argc
}
