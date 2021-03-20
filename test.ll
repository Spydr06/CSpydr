; ModuleID = 'test.bc'
source_filename = "examples/main.csp"

@test = global i8 0

define i32 @main(i32 %0, [0 x [0 x i8]] %1) {
entry:
  %msg = alloca [0 x i8], align 1
  ret i32 %0
}
