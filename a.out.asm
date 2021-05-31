    .file 1 "examples/main.csp"
    .globl main
    .text
    .type main, @function
main:
    push %rbp
    mov %rsp %rbp
    sub $0, %rsp
    mov %rsp, 0(%rbp)
