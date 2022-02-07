.globl .L.str.0
  .data
  .type .L.str.0, @object
  .size .L.str.0, 16
  .align 16
.L.str.0:
  .ascii "Hello, World!\n\0"
  .globl .L.str.1
  .data
  .type .L.str.1, @object
  .size .L.str.1, 45
  .align 16
.L.str.1:
  .ascii "{h}:{m}:{s} on {D}.{M}.{Y}\nunix time {u}s\n\0"
  .globl std.OpenFlag.APPEND
  .data
  .type std.OpenFlag.APPEND, @object
  .size std.OpenFlag.APPEND, 4
  .align 4
std.OpenFlag.APPEND:
  .byte 0
  .byte 4
  .byte 0
  .byte 0
  .globl std.OpenFlag.ASYNC
  .data
  .type std.OpenFlag.ASYNC, @object
  .size std.OpenFlag.ASYNC, 4
  .align 4
std.OpenFlag.ASYNC:
  .byte 0
  .byte 32
  .byte 0
  .byte 0
  .globl std.OpenFlag.CLOEXEC
  .data
  .type std.OpenFlag.CLOEXEC, @object
  .size std.OpenFlag.CLOEXEC, 4
  .align 4
std.OpenFlag.CLOEXEC:
  .byte 0
  .byte 0
  .byte 8
  .byte 0
  .globl std.OpenFlag.CREAT
  .data
  .type std.OpenFlag.CREAT, @object
  .size std.OpenFlag.CREAT, 4
  .align 4
std.OpenFlag.CREAT:
  .byte 64
  .byte 0
  .byte 0
  .byte 0
  .globl std.OpenFlag.DIRECT
  .data
  .type std.OpenFlag.DIRECT, @object
  .size std.OpenFlag.DIRECT, 4
  .align 4
std.OpenFlag.DIRECT:
  .byte 0
  .byte 64
  .byte 0
  .byte 0
  .globl std.OpenFlag.DIRECTORY
  .data
  .type std.OpenFlag.DIRECTORY, @object
  .size std.OpenFlag.DIRECTORY, 4
  .align 4
std.OpenFlag.DIRECTORY:
  .byte 0
  .byte 0
  .byte 1
  .byte 0
  .globl std.OpenFlag.DSYNC
  .data
  .type std.OpenFlag.DSYNC, @object
  .size std.OpenFlag.DSYNC, 4
  .align 4
std.OpenFlag.DSYNC:
  .byte 0
  .byte 16
  .byte 0
  .byte 0
  .globl std.OpenFlag.EXCL
  .data
  .type std.OpenFlag.EXCL, @object
  .size std.OpenFlag.EXCL, 4
  .align 4
std.OpenFlag.EXCL:
  .byte 128
  .byte 0
  .byte 0
  .byte 0
  .globl std.OpenFlag.LARGEFILE
  .data
  .type std.OpenFlag.LARGEFILE, @object
  .size std.OpenFlag.LARGEFILE, 4
  .align 4
std.OpenFlag.LARGEFILE:
  .byte 0
  .byte 128
  .byte 0
  .byte 0
  .globl std.OpenFlag.NOATIME
  .data
  .type std.OpenFlag.NOATIME, @object
  .size std.OpenFlag.NOATIME, 4
  .align 4
std.OpenFlag.NOATIME:
  .byte 0
  .byte 0
  .byte 4
  .byte 0
  .globl std.OpenFlag.NOCTTY
  .data
  .type std.OpenFlag.NOCTTY, @object
  .size std.OpenFlag.NOCTTY, 4
  .align 4
std.OpenFlag.NOCTTY:
  .byte 0
  .byte 1
  .byte 0
  .byte 0
  .globl std.OpenFlag.NOFOLLOW
  .data
  .type std.OpenFlag.NOFOLLOW, @object
  .size std.OpenFlag.NOFOLLOW, 4
  .align 4
std.OpenFlag.NOFOLLOW:
  .byte 0
  .byte 0
  .byte 2
  .byte 0
  .globl std.OpenFlag.NONBLOCK
  .data
  .type std.OpenFlag.NONBLOCK, @object
  .size std.OpenFlag.NONBLOCK, 4
  .align 4
std.OpenFlag.NONBLOCK:
  .byte 0
  .byte 8
  .byte 0
  .byte 0
  .globl std.OpenFlag.NDELAY
  .data
  .type std.OpenFlag.NDELAY, @object
  .size std.OpenFlag.NDELAY, 4
  .align 4
std.OpenFlag.NDELAY:
  .byte 0
  .byte 8
  .byte 0
  .byte 0
  .globl std.OpenFlag.PATH
  .data
  .type std.OpenFlag.PATH, @object
  .size std.OpenFlag.PATH, 4
  .align 4
std.OpenFlag.PATH:
  .byte 0
  .byte 0
  .byte 32
  .byte 0
  .globl std.OpenFlag.SYNC
  .data
  .type std.OpenFlag.SYNC, @object
  .size std.OpenFlag.SYNC, 4
  .align 4
std.OpenFlag.SYNC:
  .byte 0
  .byte 16
  .byte 16
  .byte 0
  .globl std.OpenFlag.TMPFILE
  .data
  .type std.OpenFlag.TMPFILE, @object
  .size std.OpenFlag.TMPFILE, 4
  .align 4
std.OpenFlag.TMPFILE:
  .byte 0
  .byte 0
  .byte 65
  .byte 0
  .globl std.OpenFlag.TRUNC
  .data
  .type std.OpenFlag.TRUNC, @object
  .size std.OpenFlag.TRUNC, 4
  .align 4
std.OpenFlag.TRUNC:
  .byte 0
  .byte 2
  .byte 0
  .byte 0
  .globl std.Prot.NONE
  .data
  .type std.Prot.NONE, @object
  .size std.Prot.NONE, 4
  .align 4
std.Prot.NONE:
  .byte 0
  .byte 0
  .byte 0
  .byte 0
  .globl std.Prot.READ
  .data
  .type std.Prot.READ, @object
  .size std.Prot.READ, 4
  .align 4
std.Prot.READ:
  .byte 1
  .byte 0
  .byte 0
  .byte 0
  .globl std.Prot.WRITE
  .data
  .type std.Prot.WRITE, @object
  .size std.Prot.WRITE, 4
  .align 4
std.Prot.WRITE:
  .byte 2
  .byte 0
  .byte 0
  .byte 0
  .globl std.Prot.EXEC
  .data
  .type std.Prot.EXEC, @object
  .size std.Prot.EXEC, 4
  .align 4
std.Prot.EXEC:
  .byte 4
  .byte 0
  .byte 0
  .byte 0
  .globl std.Prot.GROWSUP
  .data
  .type std.Prot.GROWSUP, @object
  .size std.Prot.GROWSUP, 4
  .align 4
std.Prot.GROWSUP:
  .byte 0
  .byte 0
  .byte 0
  .byte 2
  .globl std.Prot.GROWSDOWN
  .data
  .type std.Prot.GROWSDOWN, @object
  .size std.Prot.GROWSDOWN, 4
  .align 4
std.Prot.GROWSDOWN:
  .byte 0
  .byte 0
  .byte 0
  .byte 1
  .globl std.Map.SHARED
  .data
  .type std.Map.SHARED, @object
  .size std.Map.SHARED, 4
  .align 4
std.Map.SHARED:
  .byte 1
  .byte 0
  .byte 0
  .byte 0
  .globl std.Map.SHARED_VALIDATE
  .data
  .type std.Map.SHARED_VALIDATE, @object
  .size std.Map.SHARED_VALIDATE, 4
  .align 4
std.Map.SHARED_VALIDATE:
  .byte 3
  .byte 0
  .byte 0
  .byte 0
  .globl std.Map.PRIVATE
  .data
  .type std.Map.PRIVATE, @object
  .size std.Map.PRIVATE, 4
  .align 4
std.Map.PRIVATE:
  .byte 2
  .byte 0
  .byte 0
  .byte 0
  .globl std.Map.ANONYMOUS
  .data
  .type std.Map.ANONYMOUS, @object
  .size std.Map.ANONYMOUS, 4
  .align 4
std.Map.ANONYMOUS:
  .byte 32
  .byte 0
  .byte 0
  .byte 0
  .globl std.Map.FIXED
  .data
  .type std.Map.FIXED, @object
  .size std.Map.FIXED, 4
  .align 4
std.Map.FIXED:
  .byte 16
  .byte 0
  .byte 0
  .byte 0
  .globl std.Map.GROWSDOWN
  .data
  .type std.Map.GROWSDOWN, @object
  .size std.Map.GROWSDOWN, 4
  .align 4
std.Map.GROWSDOWN:
  .byte 0
  .byte 0
  .byte 0
  .byte 1
  .globl std.mem.MMAP_FAILED
  .data
  .type std.mem.MMAP_FAILED, @object
  .size std.mem.MMAP_FAILED, 8
  .align 8
std.mem.MMAP_FAILED:
  .byte 255
  .byte 255
  .byte 255
  .byte 255
  .globl std.Syscall.READ
  .data
  .type std.Syscall.READ, @object
  .size std.Syscall.READ, 4
  .align 4
std.Syscall.READ:
  .byte 0
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.WRITE
  .data
  .type std.Syscall.WRITE, @object
  .size std.Syscall.WRITE, 4
  .align 4
std.Syscall.WRITE:
  .byte 1
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.OPEN
  .data
  .type std.Syscall.OPEN, @object
  .size std.Syscall.OPEN, 4
  .align 4
std.Syscall.OPEN:
  .byte 2
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.CLOSE
  .data
  .type std.Syscall.CLOSE, @object
  .size std.Syscall.CLOSE, 4
  .align 4
std.Syscall.CLOSE:
  .byte 3
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.STAT
  .data
  .type std.Syscall.STAT, @object
  .size std.Syscall.STAT, 4
  .align 4
std.Syscall.STAT:
  .byte 4
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.FSTAT
  .data
  .type std.Syscall.FSTAT, @object
  .size std.Syscall.FSTAT, 4
  .align 4
std.Syscall.FSTAT:
  .byte 5
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.LSTAT
  .data
  .type std.Syscall.LSTAT, @object
  .size std.Syscall.LSTAT, 4
  .align 4
std.Syscall.LSTAT:
  .byte 6
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.POLL
  .data
  .type std.Syscall.POLL, @object
  .size std.Syscall.POLL, 4
  .align 4
std.Syscall.POLL:
  .byte 7
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.LSEEK
  .data
  .type std.Syscall.LSEEK, @object
  .size std.Syscall.LSEEK, 4
  .align 4
std.Syscall.LSEEK:
  .byte 8
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.MMAP
  .data
  .type std.Syscall.MMAP, @object
  .size std.Syscall.MMAP, 4
  .align 4
std.Syscall.MMAP:
  .byte 9
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.MPROTECT
  .data
  .type std.Syscall.MPROTECT, @object
  .size std.Syscall.MPROTECT, 4
  .align 4
std.Syscall.MPROTECT:
  .byte 10
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.MUNMAP
  .data
  .type std.Syscall.MUNMAP, @object
  .size std.Syscall.MUNMAP, 4
  .align 4
std.Syscall.MUNMAP:
  .byte 11
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.BRK
  .data
  .type std.Syscall.BRK, @object
  .size std.Syscall.BRK, 4
  .align 4
std.Syscall.BRK:
  .byte 12
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.RT_SIGACTION
  .data
  .type std.Syscall.RT_SIGACTION, @object
  .size std.Syscall.RT_SIGACTION, 4
  .align 4
std.Syscall.RT_SIGACTION:
  .byte 13
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.RT_SIGPROCMASK
  .data
  .type std.Syscall.RT_SIGPROCMASK, @object
  .size std.Syscall.RT_SIGPROCMASK, 4
  .align 4
std.Syscall.RT_SIGPROCMASK:
  .byte 14
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.RT_SIGRETURN
  .data
  .type std.Syscall.RT_SIGRETURN, @object
  .size std.Syscall.RT_SIGRETURN, 4
  .align 4
std.Syscall.RT_SIGRETURN:
  .byte 15
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.IOCTL
  .data
  .type std.Syscall.IOCTL, @object
  .size std.Syscall.IOCTL, 4
  .align 4
std.Syscall.IOCTL:
  .byte 16
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.PREAD64
  .data
  .type std.Syscall.PREAD64, @object
  .size std.Syscall.PREAD64, 4
  .align 4
std.Syscall.PREAD64:
  .byte 17
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.PWRITE64
  .data
  .type std.Syscall.PWRITE64, @object
  .size std.Syscall.PWRITE64, 4
  .align 4
std.Syscall.PWRITE64:
  .byte 18
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.READV
  .data
  .type std.Syscall.READV, @object
  .size std.Syscall.READV, 4
  .align 4
std.Syscall.READV:
  .byte 19
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.WRITEV
  .data
  .type std.Syscall.WRITEV, @object
  .size std.Syscall.WRITEV, 4
  .align 4
std.Syscall.WRITEV:
  .byte 20
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.ACCESS
  .data
  .type std.Syscall.ACCESS, @object
  .size std.Syscall.ACCESS, 4
  .align 4
std.Syscall.ACCESS:
  .byte 21
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.PIPE
  .data
  .type std.Syscall.PIPE, @object
  .size std.Syscall.PIPE, 4
  .align 4
std.Syscall.PIPE:
  .byte 22
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.EXIT
  .data
  .type std.Syscall.EXIT, @object
  .size std.Syscall.EXIT, 4
  .align 4
std.Syscall.EXIT:
  .byte 60
  .byte 0
  .byte 0
  .byte 0
  .globl std.Syscall.TIME
  .data
  .type std.Syscall.TIME, @object
  .size std.Syscall.TIME, 4
  .align 4
std.Syscall.TIME:
  .byte 201
  .byte 0
  .byte 0
  .byte 0
  .globl .L.str.2
  .data
  .type .L.str.2, @object
  .size .L.str.2, 5
  .align 8
.L.str.2:
  .ascii "true\0"
  .globl .L.str.3
  .data
  .type .L.str.3, @object
  .size .L.str.3, 6
  .align 8
.L.str.3:
  .ascii "false\0"
  .globl _start
  .text
_start:
  xorl %ebp, %ebp
  popq %rdi
  movq %rsp, %rsi
  andq $~15, %rsp
  call main
  movq %rax, %rdi
  movq $60, %rax
  syscall
  .globl main
  .text
  .type main, @function
main:
  push %rbp
  mov %rsp, %rbp
  sub $128, %rsp
  mov %rsp, -8(%rbp)
  mov $100, %rcx
  lea -112(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $7, %rcx
  lea -119(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -128(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea .L.str.0(%rip), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea std.io.write(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -119(%rbp), %rax
  push %rax
  lea std.time.get(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -128(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -119(%rbp), %rax
  push %rax
  lea .L.str.1(%rip), %rax
  push %rax
  lea std.time.format(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -128(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea std.io.write(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  mov $0, %rax
  jmp .L.return.main
.L.return.main:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.io.write
  .text
  .type std.io.write, @function
std.io.write:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -24(%rbp)
  mov $0, %rax
  push %rax
  sub $8, %rsp
  mov $1, %rax
  push %rax
  sub $8, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  add %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -12(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  lea std.syscall.write(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  cmp %rdi, %rax
  setle %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.0
  jmp .L.end.0
.L.else.0:
.L.end.0:
.L.return.std.io.write:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.alloc
  .text
  .type std.mem.alloc, @function
std.mem.alloc:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $8, %rcx
  lea -24(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -24(%rbp), %rax
  push %rax
  mov $8, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  mov $0, %rax
  push %rax
  mov $0, %rax
  push %rax
  lea std.Map.ANONYMOUS(%rip), %rax
  movsxd (%rax), %rax
  push %rax
  lea std.Map.PRIVATE(%rip), %rax
  movsxd (%rax), %rax
  pop %rdi
  or %edi, %eax
  push %rax
  lea std.Prot.WRITE(%rip), %rax
  movsxd (%rax), %rax
  push %rax
  lea std.Prot.READ(%rip), %rax
  movsxd (%rax), %rax
  pop %rdi
  or %edi, %eax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $0, %rax
  push %rax
  lea std.syscall.mmap(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  pop %rcx
  pop %r8
  pop %r9
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea std.mem.MMAP_FAILED(%rip), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.1
  mov $0, %rax
  jmp .L.return.std.mem.alloc
  jmp .L.end.1
.L.else.1:
.L.end.1:
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  mov $8, %rax
  push %rax
  mov $1, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  jmp .L.return.std.mem.alloc
.L.return.std.mem.alloc:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.realloc
  .text
  .type std.mem.realloc, @function
std.mem.realloc:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.alloc(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.move(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.free(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -32(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.mem.realloc
.L.return.std.mem.realloc:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.free
  .text
  .type std.mem.free, @function
std.mem.free:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $8, %rcx
  lea -24(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.2
  jmp .L.return.std.mem.free
  jmp .L.end.2
.L.else.2:
.L.end.2:
  lea -24(%rbp), %rax
  push %rax
  mov $8, %rax
  push %rax
  mov $1, %rax
  neg %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.syscall.munmap(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.return.std.mem.free:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.copy
  .text
  .type std.mem.copy, @function
std.mem.copy:
  push %rbp
  mov %rsp, %rbp
  sub $64, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.ptr_overlap(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  movzx %al, %eax
  cmp $0, %eax
  je  .L.else.3
  mov $1, %rax
  neg %rax
  jmp .L.return.std.mem.copy
  jmp .L.end.3
.L.else.3:
.L.end.3:
  lea -40(%rbp), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -56(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.4:
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setb %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.4
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  mov %al, (%rdi)
.L.continue.4:
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.4
.L.break.4:
  mov $0, %rax
  jmp .L.return.std.mem.copy
.L.return.std.mem.copy:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.move
  .text
  .type std.mem.move, @function
std.mem.move:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -40(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.alloc(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.copy(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.copy(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.free(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.mem.move
.L.return.std.mem.move:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.eq
  .text
  .type std.mem.eq, @function
std.mem.eq:
  push %rbp
  mov %rsp, %rbp
  sub $64, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -40(%rbp), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -56(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.5:
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setb %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.5
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  push %rax
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.6
  mov $0, %rax
  jmp .L.return.std.mem.eq
  jmp .L.end.6
.L.else.6:
.L.end.6:
.L.continue.5:
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.5
.L.break.5:
  mov $1, %rax
  jmp .L.return.std.mem.eq
.L.return.std.mem.eq:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.set
  .text
  .type std.mem.set, @function
std.mem.set:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %esi, -20(%rbp)
  mov %rdx, -32(%rbp)
  lea -40(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.7:
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setb %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.7
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea -20(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  mov %al, (%rdi)
.L.continue.7:
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.7
.L.break.7:
.L.return.std.mem.set:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.zero
  .text
  .type std.mem.zero, @function
std.mem.zero:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.set(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.return.std.mem.zero:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.mem.ptr_overlap
  .text
  .type std.mem.ptr_overlap, @function
std.mem.ptr_overlap:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -40(%rbp), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setle %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.9
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setl %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.9
  mov $1, %rax
  jmp .L.end.9
.L.false.9:
  mov $0, %rax
.L.end.9:
  cmp $0, %eax
  jne .L.true.8
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setle %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.10
  lea -48(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setl %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.10
  mov $1, %rax
  jmp .L.end.10
.L.false.10:
  mov $0, %rax
.L.end.10:
  cmp $0, %eax
  jne .L.true.8
  mov $0, %rax
  jmp .L.end.8
.L.true.8:
  mov $1, %rax
.L.end.8:
  jmp .L.return.std.mem.ptr_overlap
.L.return.std.mem.ptr_overlap:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.option.init
  .text
  .type std.option.init, @function
std.option.init:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %dil, -9(%rbp)
  mov %rsi, -24(%rbp)
  lea -48(%rbp), %rax
  add $0, %rax
  push %rax
  lea -9(%rbp), %rax
  movsbl (%rax), %eax
  pop %rdi
  mov %al, (%rdi)
  lea -48(%rbp), %rax
  lea -48(%rbp), %rax
  add $8, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  mov %rax, %rdi
  mov $0, %rax
  shl $8, %rax
  mov 7(%rdi), %al
  shl $8, %rax
  mov 6(%rdi), %al
  shl $8, %rax
  mov 5(%rdi), %al
  shl $8, %rax
  mov 4(%rdi), %al
  shl $8, %rax
  mov 3(%rdi), %al
  shl $8, %rax
  mov 2(%rdi), %al
  shl $8, %rax
  mov 1(%rdi), %al
  shl $8, %rax
  mov 0(%rdi), %al
  mov $0, %rdx
  shl $8, %rdx
  mov 15(%rdi), %dl
  shl $8, %rdx
  mov 14(%rdi), %dl
  shl $8, %rdx
  mov 13(%rdi), %dl
  shl $8, %rdx
  mov 12(%rdi), %dl
  shl $8, %rdx
  mov 11(%rdi), %dl
  shl $8, %rdx
  mov 10(%rdi), %dl
  shl $8, %rdx
  mov 9(%rdi), %dl
  shl $8, %rdx
  mov 8(%rdi), %dl
  jmp .L.return.std.option.init
.L.return.std.option.init:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.option.some
  .text
  .type std.option.some, @function
std.option.some:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea std.option.init(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  mov %al, -32(%rbp)
  shr $8, %rax
  mov %al, -31(%rbp)
  shr $8, %rax
  mov %al, -30(%rbp)
  shr $8, %rax
  mov %al, -29(%rbp)
  shr $8, %rax
  mov %al, -28(%rbp)
  shr $8, %rax
  mov %al, -27(%rbp)
  shr $8, %rax
  mov %al, -26(%rbp)
  shr $8, %rax
  mov %al, -25(%rbp)
  shr $8, %rax
  mov %dl, -24(%rbp)
  shr $8, %rdx
  mov %dl, -23(%rbp)
  shr $8, %rdx
  mov %dl, -22(%rbp)
  shr $8, %rdx
  mov %dl, -21(%rbp)
  shr $8, %rdx
  mov %dl, -20(%rbp)
  shr $8, %rdx
  mov %dl, -19(%rbp)
  shr $8, %rdx
  mov %dl, -18(%rbp)
  shr $8, %rdx
  mov %dl, -17(%rbp)
  shr $8, %rdx
  lea -32(%rbp), %rax
  mov %rax, %rdi
  mov $0, %rax
  shl $8, %rax
  mov 7(%rdi), %al
  shl $8, %rax
  mov 6(%rdi), %al
  shl $8, %rax
  mov 5(%rdi), %al
  shl $8, %rax
  mov 4(%rdi), %al
  shl $8, %rax
  mov 3(%rdi), %al
  shl $8, %rax
  mov 2(%rdi), %al
  shl $8, %rax
  mov 1(%rdi), %al
  shl $8, %rax
  mov 0(%rdi), %al
  mov $0, %rdx
  shl $8, %rdx
  mov 15(%rdi), %dl
  shl $8, %rdx
  mov 14(%rdi), %dl
  shl $8, %rdx
  mov 13(%rdi), %dl
  shl $8, %rdx
  mov 12(%rdi), %dl
  shl $8, %rdx
  mov 11(%rdi), %dl
  shl $8, %rdx
  mov 10(%rdi), %dl
  shl $8, %rdx
  mov 9(%rdi), %dl
  shl $8, %rdx
  mov 8(%rdi), %dl
  jmp .L.return.std.option.some
.L.return.std.option.some:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.option.none
  .text
  .type std.option.none, @function
std.option.none:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov $0, %rax
  push %rax
  mov $0, %rax
  push %rax
  lea std.option.init(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  mov %al, -32(%rbp)
  shr $8, %rax
  mov %al, -31(%rbp)
  shr $8, %rax
  mov %al, -30(%rbp)
  shr $8, %rax
  mov %al, -29(%rbp)
  shr $8, %rax
  mov %al, -28(%rbp)
  shr $8, %rax
  mov %al, -27(%rbp)
  shr $8, %rax
  mov %al, -26(%rbp)
  shr $8, %rax
  mov %al, -25(%rbp)
  shr $8, %rax
  mov %dl, -24(%rbp)
  shr $8, %rdx
  mov %dl, -23(%rbp)
  shr $8, %rdx
  mov %dl, -22(%rbp)
  shr $8, %rdx
  mov %dl, -21(%rbp)
  shr $8, %rdx
  mov %dl, -20(%rbp)
  shr $8, %rdx
  mov %dl, -19(%rbp)
  shr $8, %rdx
  mov %dl, -18(%rbp)
  shr $8, %rdx
  mov %dl, -17(%rbp)
  shr $8, %rdx
  lea -32(%rbp), %rax
  mov %rax, %rdi
  mov $0, %rax
  shl $8, %rax
  mov 7(%rdi), %al
  shl $8, %rax
  mov 6(%rdi), %al
  shl $8, %rax
  mov 5(%rdi), %al
  shl $8, %rax
  mov 4(%rdi), %al
  shl $8, %rax
  mov 3(%rdi), %al
  shl $8, %rax
  mov 2(%rdi), %al
  shl $8, %rax
  mov 1(%rdi), %al
  shl $8, %rax
  mov 0(%rdi), %al
  mov $0, %rdx
  shl $8, %rdx
  mov 15(%rdi), %dl
  shl $8, %rdx
  mov 14(%rdi), %dl
  shl $8, %rdx
  mov 13(%rdi), %dl
  shl $8, %rdx
  mov 12(%rdi), %dl
  shl $8, %rdx
  mov 11(%rdi), %dl
  shl $8, %rdx
  mov 10(%rdi), %dl
  shl $8, %rdx
  mov 9(%rdi), %dl
  shl $8, %rdx
  mov 8(%rdi), %dl
  jmp .L.return.std.option.none
.L.return.std.option.none:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.option.is_some
  .text
  .type std.option.is_some, @function
std.option.is_some:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -32(%rbp)
  mov %rsi, -24(%rbp)
  lea -32(%rbp), %rax
  add $0, %rax
  movsbl (%rax), %eax
  jmp .L.return.std.option.is_some
.L.return.std.option.is_some:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.option.unwrap
  .text
  .type std.option.unwrap, @function
std.option.unwrap:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -32(%rbp)
  mov %rsi, -24(%rbp)
  lea -32(%rbp), %rax
  add $0, %rax
  movsbl (%rax), %eax
  cmp $0, %eax
  je .L.else.11
  lea -32(%rbp), %rax
  add $8, %rax
  mov (%rax), %rax
  jmp .L.end.11
  .L.else.11:
  mov $0, %rax
.L.end.11:
  jmp .L.return.std.option.unwrap
.L.return.std.option.unwrap:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.option.unwrap_or
  .text
  .type std.option.unwrap_or, @function
std.option.unwrap_or:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -32(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -40(%rbp)
  lea -32(%rbp), %rax
  add $0, %rax
  movsbl (%rax), %eax
  cmp $0, %eax
  je  .L.else.12
  lea -32(%rbp), %rax
  add $8, %rax
  mov (%rax), %rax
  jmp .L.return.std.option.unwrap_or
  jmp .L.end.12
.L.else.12:
  lea -40(%rbp), %rax
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  mov $0, %rax
  jmp .L.return.std.option.unwrap_or
.L.end.12:
.L.return.std.option.unwrap_or:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.init
  .text
  .type std.string.init, @function
std.string.init:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.13
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  mov $16, %rax
  push %rax
  lea std.mem.alloc(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  push %rax
  mov $1, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
  jmp .L.end.13
.L.else.13:
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -40(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $16, %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea std.mem.alloc(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  push %rax
  lea std.c_str.copy(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.end.13:
  mov $1, %rax
  push %rax
  mov $0, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  jmp .L.return.std.string.init
.L.return.std.string.init:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.free
  .text
  .type std.string.free, @function
std.string.free:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.string.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  push %rax
  lea std.mem.free(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.return.std.string.free:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.get_data
  .text
  .type std.string.get_data, @function
std.string.get_data:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $16, %rax
  push %rax
  mov $1, %rax
  neg %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  jmp .L.return.std.string.get_data
.L.return.std.string.get_data:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.concat
  .text
  .type std.string.concat, @function
std.string.concat:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  mov (%rax), %rax
  push %rax
  lea std.string.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -40(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  push %rax
  sub $8, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  sub $8, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  mov (%rax), %rax
  push %rax
  mov $16, %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.realloc(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea std.c_str.copy(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  mov $1, %rax
  push %rax
  sub $8, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $1, %rax
  push %rax
  mov $0, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.return.std.string.concat:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.copy
  .text
  .type std.string.copy, @function
std.string.copy:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.string.init(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.return.std.string.copy
.L.return.std.string.copy:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.contains
  .text
  .type std.string.contains, @function
std.string.contains:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov $0, %rax
  push %rax
  sub $8, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.strstr(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  cmp %rdi, %rax
  setne %al
  movzb %al, %rax
  jmp .L.return.std.string.contains
.L.return.std.string.contains:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.size
  .text
  .type std.string.size, @function
std.string.size:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.return.std.string.size
.L.return.std.string.size:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.string.substr
  .text
  .type std.string.substr, @function
std.string.substr:
  push %rbp
  mov %rsp, %rbp
  sub $64, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $0, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  cmp %rdi, %rax
  setl %al
  movzb %al, %rax
  cmp $0, %eax
  jne .L.true.15
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.string.size(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  cmp %rdi, %rax
  setle %al
  movzb %al, %rax
  cmp $0, %eax
  jne .L.true.15
  mov $0, %rax
  jmp .L.end.15
.L.true.15:
  mov $1, %rax
.L.end.15:
  cmp $0, %eax
  je  .L.else.14
  mov $0, %rax
  jmp .L.return.std.string.substr
  jmp .L.end.14
.L.else.14:
.L.end.14:
  lea -40(%rbp), %rax
  push %rax
  sub $8, %rsp
  mov $1, %rax
  push %rax
  mov $2, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  imul %edi, %eax
  push %rax
  lea std.mem.alloc(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -56(%rbp), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.16:
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setl %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.16
  mov $1, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  mov %al, (%rdi)
.L.continue.16:
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.16
.L.break.16:
  lea -48(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.string.init(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.free(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -48(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.string.substr
.L.return.std.string.substr:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.get_len
  .text
  .type std.c_str.get_len, @function
std.c_str.get_len:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $8, %rcx
  lea -24(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -24(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.17:
  mov $0, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.17
  mov $1, %rax
  push %rax
  lea -24(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
.L.continue.17:
  jmp .L.begin.17
.L.break.17:
  lea -24(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.get_len
.L.return.std.c_str.get_len:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.copy
  .text
  .type std.c_str.copy, @function
std.c_str.copy:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.move(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.return.std.c_str.copy:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.strstr
  .text
  .type std.c_str.strstr, @function
std.c_str.strstr:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -40(%rbp), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.18:
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.19
  mov $0, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.19
  mov $1, %rax
  jmp .L.end.19
.L.false.19:
  mov $0, %rax
.L.end.19:
  cmp $0, %eax
  je .L.break.18
  lea -40(%rbp), %rax
  mov (%rax), %rax
  movsbl (%rax), %eax
  push %rax
  mov $1, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  movsbl (%rax), %eax
  pop %rdi
  add %edi, %eax
  pop %rdi
  mov %al, (%rdi)
  pop %rdi
  sub %edi, %eax
  pop %rdi
  cmp %edi, %eax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.20
  mov $8, %rax
  push %rax
  mov $1, %rax
  pop %rdi
  imul %edi, %eax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  mov $1, %rax
  pop %rdi
  imul %edi, %eax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.end.20
.L.else.20:
.L.end.20:
  mov $0, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.21
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  cqo
  idiv %rdi
  jmp .L.return.std.c_str.strstr
  jmp .L.end.21
.L.else.21:
.L.end.21:
  mov $1, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  cqo
  idiv %rdi
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.22
  lea -40(%rbp), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  jmp .L.end.22
.L.else.22:
.L.end.22:
.L.continue.18:
  jmp .L.begin.18
.L.break.18:
  mov $0, %rax
  jmp .L.return.std.c_str.strstr
.L.return.std.c_str.strstr:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.strsep
  .text
  .type std.c_str.strsep, @function
std.c_str.strsep:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -32(%rbp), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  mov $0, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.23
  mov $0, %rax
  jmp .L.return.std.c_str.strsep
  jmp .L.end.23
.L.else.23:
.L.end.23:
  lea -40(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  add %edi, %eax
  pop %rdi
  mov %rax, (%rdi)
  mov $0, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.24
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  movsbl (%rax), %eax
  pop %rdi
  add %edi, %eax
  pop %rdi
  mov %al, (%rdi)
  pop %rdi
  sub %edi, %eax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  jmp .L.end.24
.L.else.24:
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.end.24:
  lea -32(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.strsep
.L.return.std.c_str.strsep:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.strspn
  .text
  .type std.c_str.strspn, @function
std.c_str.strspn:
  push %rbp
  mov %rsp, %rbp
  sub $80, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov $32, %rcx
  lea -64(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -72(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $32, %rax
  push %rax
  lea -64(%rbp), %rax
  push %rax
  lea std.mem.zero(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -72(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.25:
  mov $0, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.25
  mov $1, %rax
  push %rax
  mov $3, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  mov %rdi, %rcx
  shr %cl, %eax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -64(%rbp), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $8, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  mov $0, %edx
  div %edi
  mov %rdx, %rax
  push %rax
  mov $1, %rax
  pop %rdi
  mov %rdi, %rcx
  shl %cl, %eax
  push %rax
  mov $1, %rax
  push %rax
  mov $3, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  mov %rdi, %rcx
  shr %cl, %eax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -64(%rbp), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  or %edi, %eax
  pop %rdi
  mov %al, (%rdi)
.L.continue.25:
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.25
.L.break.25:
  lea -72(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.26:
  mov $1, %rax
  push %rax
  mov $8, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  mov $0, %edx
  div %edi
  mov %rdx, %rax
  push %rax
  mov $1, %rax
  push %rax
  mov $3, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  mov %rdi, %rcx
  shr %cl, %eax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -64(%rbp), %rax
  pop %rdi
  add %rdi, %rax
  movzbl (%rax), %eax
  pop %rdi
  mov %rdi, %rcx
  shr %cl, %eax
  pop %rdi
  and %edi, %eax
  cmp $0, %eax
  je .L.break.26
.L.continue.26:
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -72(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.26
.L.break.26:
  lea -72(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.strspn
.L.return.std.c_str.strspn:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.reverse
  .text
  .type std.c_str.reverse, @function
std.c_str.reverse:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $8, %rcx
  lea -24(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -24(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.get_len(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.27:
  mov $2, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setb %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.27
  mov $1, %rcx
  lea -33(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -33(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  mov %al, (%rdi)
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  sub %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  mov %al, (%rdi)
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  sub %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea -33(%rbp), %rax
  movsbl (%rax), %eax
  pop %rdi
  mov %al, (%rdi)
.L.continue.27:
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.27
.L.break.27:
.L.return.std.c_str.reverse:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.from_int
  .text
  .type std.c_str.from_int, @function
std.c_str.from_int:
  push %rbp
  mov %rsp, %rbp
  sub $64, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %edx, -28(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $1, %rcx
  lea -41(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -40(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -41(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %al, (%rdi)
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.28
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $48, %rax
  pop %rdi
  mov %al, (%rdi)
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.from_int
  jmp .L.end.28
.L.else.28:
.L.end.28:
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setl %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.30
  mov $10, %rax
  push %rax
  lea -28(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  cmp %edi, %eax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.30
  mov $1, %rax
  jmp .L.end.30
.L.false.30:
  mov $0, %rax
.L.end.30:
  cmp $0, %eax
  je  .L.else.29
  lea -41(%rbp), %rax
  push %rax
  mov $1, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -16(%rbp), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  neg %rax
  pop %rdi
  mov %rax, (%rdi)
  jmp .L.end.29
.L.else.29:
.L.end.29:
.L.begin.31:
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.31
  mov $8, %rcx
  lea -56(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -56(%rbp), %rax
  push %rax
  lea -28(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  mov %rdx, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -56(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $9, %rax
  pop %rdi
  cmp %rdi, %rax
  setl %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.32
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $97, %rax
  push %rax
  mov $10, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  add %edi, %eax
  pop %rdi
  mov %al, (%rdi)
  jmp .L.end.32
.L.else.32:
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $48, %rax
  push %rax
  lea -56(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %al, (%rdi)
.L.end.32:
  lea -16(%rbp), %rax
  push %rax
  lea -28(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  pop %rdi
  mov %rax, (%rdi)
.L.continue.31:
  jmp .L.begin.31
.L.break.31:
  lea -41(%rbp), %rax
  movsbl (%rax), %eax
  cmp $0, %eax
  je  .L.else.33
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $45, %rax
  pop %rdi
  mov %al, (%rdi)
  jmp .L.end.33
.L.else.33:
.L.end.33:
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.reverse(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.from_int
.L.return.std.c_str.from_int:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.from_uint
  .text
  .type std.c_str.from_uint, @function
std.c_str.from_uint:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %edx, -28(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -40(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.34
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $48, %rax
  pop %rdi
  mov %al, (%rdi)
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.from_uint
  jmp .L.end.34
.L.else.34:
.L.end.34:
.L.begin.35:
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.35
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -48(%rbp), %rax
  push %rax
  lea -28(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov $0, %rdx
  div %rdi
  mov %rdx, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $9, %rax
  pop %rdi
  cmp %rdi, %rax
  setb %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.36
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $97, %rax
  push %rax
  mov $10, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  add %edi, %eax
  pop %rdi
  mov %al, (%rdi)
  jmp .L.end.36
.L.else.36:
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $48, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %al, (%rdi)
.L.end.36:
  lea -16(%rbp), %rax
  push %rax
  lea -28(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  pop %rdi
  mov %rax, (%rdi)
.L.continue.35:
  jmp .L.begin.35
.L.break.35:
  mov $1, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.reverse(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.from_uint
.L.return.std.c_str.from_uint:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.c_str.from_bool
  .text
  .type std.c_str.from_bool, @function
std.c_str.from_bool:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %dil, -9(%rbp)
  mov %rsi, -24(%rbp)
  lea -9(%rbp), %rax
  movsbl (%rax), %eax
  cmp $0, %eax
  je .L.else.37
  lea .L.str.2(%rip), %rax
  jmp .L.end.37
  .L.else.37:
  lea .L.str.3(%rip), %rax
.L.end.37:
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.copy(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -24(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.c_str.from_bool
.L.return.std.c_str.from_bool:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.init
  .text
  .type std.vec.init, @function
std.vec.init:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov $8, %rcx
  lea -16(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -16(%rbp), %rax
  push %rax
  sub $8, %rsp
  mov $16, %rax
  push %rax
  lea std.mem.alloc(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  jmp .L.return.std.vec.init
.L.return.std.vec.init:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.free
  .text
  .type std.vec.free, @function
std.vec.free:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  push %rax
  lea std.mem.free(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.return.std.vec.free:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.get_data
  .text
  .type std.vec.get_data, @function
std.vec.get_data:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $16, %rax
  push %rax
  mov $1, %rax
  neg %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  jmp .L.return.std.vec.get_data
.L.return.std.vec.get_data:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.size
  .text
  .type std.vec.size, @function
std.vec.size:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  add $8, %rax
  mov (%rax), %rax
  jmp .L.return.std.vec.size
.L.return.std.vec.size:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.allocated
  .text
  .type std.vec.allocated, @function
std.vec.allocated:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  add $0, %rax
  mov (%rax), %rax
  jmp .L.return.std.vec.allocated
.L.return.std.vec.allocated:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.reallocate
  .text
  .type std.vec.reallocate, @function
std.vec.reallocate:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %sil, -17(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $0, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.38
  lea -32(%rbp), %rax
  push %rax
  mov $1, %rax
  pop %rdi
  mov %rax, (%rdi)
  jmp .L.end.38
.L.else.38:
  lea -32(%rbp), %rax
  push %rax
  mov $2, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.end.38:
  lea -48(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  mov $16, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.realloc(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.vec.reallocate
.L.return.std.vec.reallocate:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.has_space
  .text
  .type std.vec.has_space, @function
std.vec.has_space:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  mov (%rax), %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  cmp %rdi, %rax
  setb %al
  movzb %al, %rax
  jmp .L.return.std.vec.has_space
.L.return.std.vec.has_space:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.__internal_add
  .text
  .type std.vec.__internal_add, @function
std.vec.__internal_add:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %sil, -17(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.has_space(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  movzx %al, %eax
  cmp $0, %eax
  sete %al
  movzx %al, %rax
  cmp $0, %eax
  je  .L.else.39
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.reallocate(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  mov %rax, (%rdi)
  jmp .L.end.39
.L.else.39:
.L.end.39:
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  pop %rdi
  imul %edi, %eax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  jmp .L.return.std.vec.__internal_add
.L.return.std.vec.__internal_add:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.__internal_insert
  .text
  .type std.vec.__internal_insert, @function
std.vec.__internal_insert:
  push %rbp
  mov %rsp, %rbp
  sub $64, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %sil, -17(%rbp)
  mov %rdx, -32(%rbp)
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $4, %rcx
  lea -52(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -48(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -52(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.size(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %eax, (%rdi)
  lea -48(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.has_space(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  movzx %al, %eax
  cmp $0, %eax
  sete %al
  movzx %al, %rax
  cmp $0, %eax
  je  .L.else.40
  lea -48(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.reallocate(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  mov %rax, (%rdi)
  jmp .L.end.40
.L.else.40:
.L.end.40:
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  imul %edi, %eax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea std.mem.move(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  lea -52(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  mov $1, %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  jmp .L.return.std.vec.__internal_insert
.L.return.std.vec.__internal_insert:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.__internal_erase
  .text
  .type std.vec.__internal_erase, @function
std.vec.__internal_erase:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %sil, -17(%rbp)
  mov %rdx, -32(%rbp)
  mov %rcx, -40(%rbp)
  mov $8, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -48(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  imul %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  lea std.mem.move(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  mov (%rax), %rax
  pop %rdi
  sub %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.return.std.vec.__internal_erase:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.__internal_remove
  .text
  .type std.vec.__internal_remove, @function
std.vec.__internal_remove:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %sil, -17(%rbp)
  mov %rdx, -32(%rbp)
  mov $1, %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.__internal_erase(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  pop %rcx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.return.std.vec.__internal_remove:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.__internal_copy
  .text
  .type std.vec.__internal_copy, @function
std.vec.__internal_copy:
  push %rbp
  mov %rsp, %rbp
  sub $64, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %sil, -17(%rbp)
  mov $8, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $1, %rcx
  lea -41(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $8, %rcx
  lea -64(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -32(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -40(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -41(%rbp), %rax
  push %rax
  lea -17(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -40(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  mov $16, %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -64(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea -41(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea std.mem.alloc(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -41(%rbp), %rax
  movzbl (%rax), %eax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea -64(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.mem.copy(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  lea -64(%rbp), %rax
  mov (%rax), %rax
  add $16, %rax
  jmp .L.return.std.vec.__internal_copy
.L.return.std.vec.__internal_copy:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.vec.pop
  .text
  .type std.vec.pop, @function
std.vec.pop:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $-1, %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  add $8, %rax
  push %rax
  mov $-1, %rax
  push %rax
  sub $8, %rsp
  lea -16(%rbp), %rax
  mov (%rax), %rax
  push %rax
  lea std.vec.get_data(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  add $8, %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
.L.return.std.vec.pop:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.time.get
  .text
  .type std.time.get, @function
std.time.get:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov $8, %rcx
  lea -24(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $4, %rcx
  lea -28(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $4, %rcx
  lea -32(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $4, %rcx
  lea -36(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $4, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $4, %rcx
  lea -44(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $4, %rcx
  lea -48(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -24(%rbp), %rax
  push %rax
  sub $8, %rsp
  lea std.time.unix.secs(%rip), %rax
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  mov $0, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cmp %rdi, %rax
  setle %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.41
  jmp .L.return.std.time.get
  jmp .L.end.41
.L.else.41:
.L.end.41:
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $14, %rax
  push %rax
  mov $60, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  mov %rdx, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -24(%rbp), %rax
  push %rax
  mov $60, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $13, %rax
  push %rax
  mov $60, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  mov %rdx, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -24(%rbp), %rax
  push %rax
  mov $60, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  pop %rdi
  mov %rax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $12, %rax
  push %rax
  mov $24, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  mov %rdx, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -24(%rbp), %rax
  push %rax
  mov $24, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  cqo
  idiv %rdi
  pop %rdi
  mov %rax, (%rdi)
  lea -28(%rbp), %rax
  push %rax
  mov $15, %rax
  push %rax
  mov $146097, %rax
  push %rax
  mov $102032, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  push %rax
  mov $4, %rax
  pop %rdi
  imul %edi, %eax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  cdq
  idiv %edi
  pop %rdi
  add %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
  lea -32(%rbp), %rax
  push %rax
  mov $4, %rax
  push %rax
  lea -28(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  cdq
  idiv %edi
  push %rax
  lea -28(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  mov $2442113, %rax
  push %rax
  lea -24(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  add %edi, %eax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
  lea -36(%rbp), %rax
  push %rax
  mov $7305, %rax
  push %rax
  mov $2442, %rax
  push %rax
  lea -32(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  mov $20, %rax
  pop %rdi
  imul %edi, %eax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  cdq
  idiv %edi
  pop %rdi
  mov %eax, (%rdi)
  lea -40(%rbp), %rax
  push %rax
  mov $4, %rax
  push %rax
  lea -36(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  cdq
  idiv %edi
  push %rax
  lea -36(%rbp), %rax
  movsxd (%rax), %rax
  push %rax
  mov $365, %rax
  pop %rdi
  imul %edi, %eax
  push %rax
  lea -32(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
  lea -44(%rbp), %rax
  push %rax
  mov $30601, %rax
  push %rax
  mov $1000, %rax
  push %rax
  lea -40(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  imul %edi, %eax
  pop %rdi
  cdq
  idiv %edi
  pop %rdi
  mov %eax, (%rdi)
  lea -48(%rbp), %rax
  push %rax
  mov $1000, %rax
  push %rax
  mov $601, %rax
  push %rax
  lea -44(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  imul %edi, %eax
  pop %rdi
  cdq
  idiv %edi
  push %rax
  mov $30, %rax
  push %rax
  lea -44(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  imul %edi, %eax
  push %rax
  lea -40(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
  mov $13, %rax
  push %rax
  lea -44(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  cmp %edi, %eax
  setle %al
  movzb %al, %rax
  cmp $0, %eax
  je  .L.else.42
  lea -36(%rbp), %rax
  push %rax
  mov $4716, %rax
  push %rax
  lea -36(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
  lea -44(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -44(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
  jmp .L.end.42
.L.else.42:
  lea -36(%rbp), %rax
  push %rax
  mov $4715, %rax
  push %rax
  lea -36(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
  lea -44(%rbp), %rax
  push %rax
  mov $13, %rax
  push %rax
  lea -44(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  sub %edi, %eax
  pop %rdi
  mov %eax, (%rdi)
.L.end.42:
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  push %rax
  lea -36(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  mov %ax, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $10, %rax
  push %rax
  lea -44(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  mov %al, (%rdi)
  lea -16(%rbp), %rax
  mov (%rax), %rax
  add $11, %rax
  push %rax
  lea -48(%rbp), %rax
  movsxd (%rax), %rax
  pop %rdi
  mov %al, (%rdi)
.L.return.std.time.get:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.time.format
  .text
  .type std.time.format, @function
std.time.format:
  push %rbp
  mov %rsp, %rbp
  sub $96, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -32(%rbp)
  mov $8, %rcx
  lea -40(%rbp), %rdi
  mov $0, %al
  rep stosb
  lea -40(%rbp), %rax
  push %rax
  sub $8, %rsp
  mov $0, %rax
  push %rax
  lea std.string.init(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $8, %rsp
  pop %rdi
  mov %rax, (%rdi)
  lea -48(%rbp), %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %rax, (%rdi)
.L.begin.43:
  mov $0, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.break.43
  mov $123, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.46
  mov $0, %rax
  push %rax
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  setne %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.46
  mov $1, %rax
  jmp .L.end.46
.L.false.46:
  mov $0, %rax
.L.end.46:
  cmp $0, %eax
  je .L.false.45
  mov $125, %rax
  push %rax
  mov $1, %rax
  push %rax
  mov $2, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  cmp %edi, %eax
  sete %al
  movzb %al, %rax
  cmp $0, %eax
  je .L.false.45
  mov $1, %rax
  jmp .L.end.45
.L.false.45:
  mov $0, %rax
.L.end.45:
  cmp $0, %eax
  je  .L.else.44
  mov $32, %rcx
  lea -80(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $32, %rax
  push %rax
  mov $1, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea std.mem.zero(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  mov $1, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -48(%rbp), %rax
  push %rax
  mov $2, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  movsxd %eax, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  push %rax
  push %rax
  mov $89, %rax
  pop %rdi
  cmp %eax, %edi
  pop %rax
  je .L.case.0.47
  push %rax
  push %rax
  mov $77, %rax
  pop %rdi
  cmp %eax, %edi
  pop %rax
  je .L.case.1.47
  push %rax
  push %rax
  mov $68, %rax
  pop %rdi
  cmp %eax, %edi
  pop %rax
  je .L.case.2.47
  push %rax
  push %rax
  mov $104, %rax
  pop %rdi
  cmp %eax, %edi
  pop %rax
  je .L.case.3.47
  push %rax
  push %rax
  mov $109, %rax
  pop %rdi
  cmp %eax, %edi
  pop %rax
  je .L.case.4.47
  push %rax
  push %rax
  mov $115, %rax
  pop %rdi
  cmp %eax, %edi
  pop %rax
  je .L.case.5.47
  push %rax
  push %rax
  mov $117, %rax
  pop %rdi
  cmp %eax, %edi
  pop %rax
  je .L.case.6.47
  jmp .L.case.7.47
.L.case.0.47:
  mov $10, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $8, %rax
  movzwl (%rax), %eax
  push %rax
  lea std.c_str.from_uint(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.break.47
.L.case.1.47:
  mov $10, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $10, %rax
  movzbl (%rax), %eax
  push %rax
  lea std.c_str.from_uint(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.break.47
.L.case.2.47:
  mov $10, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $11, %rax
  movzbl (%rax), %eax
  push %rax
  lea std.c_str.from_uint(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.break.47
.L.case.3.47:
  mov $10, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $12, %rax
  movzbl (%rax), %eax
  push %rax
  lea std.c_str.from_uint(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.break.47
.L.case.4.47:
  mov $10, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $10, %rax
  movzbl (%rax), %eax
  push %rax
  lea std.c_str.from_uint(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.break.47
.L.case.5.47:
  mov $10, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $14, %rax
  movzbl (%rax), %eax
  push %rax
  lea std.c_str.from_uint(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.break.47
.L.case.6.47:
  mov $10, %rax
  push %rax
  lea -80(%rbp), %rax
  push %rax
  lea -32(%rbp), %rax
  mov (%rax), %rax
  add $0, %rax
  mov (%rax), %rax
  push %rax
  lea std.c_str.from_int(%rip), %rax
  pop %rdi
  pop %rsi
  pop %rdx
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.break.47
.L.case.7.47:
  mov $0, %rax
  jmp .L.return.std.time.format
  jmp .L.break.47
.L.break.47:
  lea -80(%rbp), %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  lea std.string.concat(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.end.44
.L.else.44:
  mov $2, %rcx
  lea -88(%rbp), %rdi
  mov $0, %al
  rep stosb
  mov $1, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -88(%rbp), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -16(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  movsbl (%rax), %eax
  pop %rdi
  mov %al, (%rdi)
  lea -88(%rbp), %rax
  mov $1, %rax
  push %rax
  mov $1, %rax
  pop %rdi
  imul %rdi, %rax
  push %rax
  lea -88(%rbp), %rax
  pop %rdi
  add %rdi, %rax
  push %rax
  mov $0, %rax
  pop %rdi
  mov %al, (%rdi)
  lea -88(%rbp), %rax
  lea -88(%rbp), %rax
  push %rax
  lea -40(%rbp), %rax
  push %rax
  lea std.string.concat(%rip), %rax
  pop %rdi
  pop %rsi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
.L.end.44:
.L.continue.43:
  mov $1, %rax
  push %rax
  lea -48(%rbp), %rax
  push %rax
  mov $1, %rax
  push %rax
  lea -48(%rbp), %rax
  mov (%rax), %rax
  pop %rdi
  add %rdi, %rax
  pop %rdi
  mov %rax, (%rdi)
  pop %rdi
  sub %rdi, %rax
  jmp .L.begin.43
.L.break.43:
  lea -40(%rbp), %rax
  mov (%rax), %rax
  jmp .L.return.std.time.format
.L.return.std.time.format:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.time.unix.secs
  .text
  .type std.time.unix.secs, @function
std.time.unix.secs:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov $0, %rax
  push %rax
  lea std.syscall.time(%rip), %rax
  pop %rdi
  mov %rax, %r10
  mov $0, %rax
  call *%r10
  add $0, %rsp
  jmp .L.return.std.time.unix.secs
.L.return.std.time.unix.secs:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.time.unix.millis
  .text
  .type std.time.unix.millis, @function
std.time.unix.millis:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
.L.return.std.time.unix.millis:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.read
  .text
  .type std.syscall.read, @function
std.syscall.read:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov -12(%rbp), %rdi;mov -24(%rbp), %rsi;mov -32(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.READ(%rip), %rax;syscall
.L.return.std.syscall.read:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.write
  .text
  .type std.syscall.write, @function
std.syscall.write:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov -12(%rbp), %rdi;mov -24(%rbp), %rsi;mov -32(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.WRITE(%rip), %rax;syscall
.L.return.std.syscall.write:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.open
  .text
  .type std.syscall.open, @function
std.syscall.open:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %esi, -20(%rbp)
  mov %edx, -24(%rbp)
  mov -16(%rbp), %rdi;mov -20(%rbp), %rsi;mov -24(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.OPEN(%rip), %rax;syscall
.L.return.std.syscall.open:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.close
  .text
  .type std.syscall.close, @function
std.syscall.close:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov -12(%rbp), %rdi;mov $0, %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.CLOSE(%rip), %rax;syscall
.L.return.std.syscall.close:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.stat
  .text
  .type std.syscall.stat, @function
std.syscall.stat:
  push %rbp
  mov %rsp, %rbp
  sub $128, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -128(%rbp)
  mov -16(%rbp), %rdi;mov -128(%rbp), %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.STAT(%rip), %rax;syscall
.L.return.std.syscall.stat:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.fstat
  .text
  .type std.syscall.fstat, @function
std.syscall.fstat:
  push %rbp
  mov %rsp, %rbp
  sub $128, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -128(%rbp)
  mov -12(%rbp), %rdi;mov -128(%rbp), %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.FSTAT(%rip), %rax;syscall
.L.return.std.syscall.fstat:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.lstat
  .text
  .type std.syscall.lstat, @function
std.syscall.lstat:
  push %rbp
  mov %rsp, %rbp
  sub $128, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -128(%rbp)
  mov -16(%rbp), %rdi;mov -128(%rbp), %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.LSTAT(%rip), %rax;syscall
.L.return.std.syscall.lstat:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.poll
  .text
  .type std.syscall.poll, @function
std.syscall.poll:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %esi, -20(%rbp)
  mov %edx, -24(%rbp)
  mov -16(%rbp), %rdi;mov -20(%rbp), %rsi;mov -24(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.POLL(%rip), %rax;syscall
.L.return.std.syscall.poll:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.lseek
  .text
  .type std.syscall.lseek, @function
std.syscall.lseek:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -24(%rbp)
  mov %edx, -28(%rbp)
  mov -12(%rbp), %rdi;mov -24(%rbp), %rsi;mov -28(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.LSEEK(%rip), %rax;syscall
.L.return.std.syscall.lseek:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.mmap
  .text
  .type std.syscall.mmap, @function
std.syscall.mmap:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %edx, -28(%rbp)
  mov %ecx, -32(%rbp)
  mov %r8d, -36(%rbp)
  mov %r9, -48(%rbp)
  mov -16(%rbp), %rdi;mov -24(%rbp), %rsi;mov -28(%rbp), %rdx;mov -32(%rbp), %r10;mov -36(%rbp), %r8;mov -48(%rbp), %r9;mov std.Syscall.MMAP(%rip), %rax;syscall
.L.return.std.syscall.mmap:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.mprotect
  .text
  .type std.syscall.mprotect, @function
std.syscall.mprotect:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov %edx, -28(%rbp)
  mov -16(%rbp), %rdi;mov -24(%rbp), %rsi;mov -28(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.MPROTECT(%rip), %rax;syscall
.L.return.std.syscall.mprotect:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.munmap
  .text
  .type std.syscall.munmap, @function
std.syscall.munmap:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %rsi, -24(%rbp)
  mov -16(%rbp), %rdi;mov -24(%rbp), %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.MUNMAP(%rip), %rax;syscall
.L.return.std.syscall.munmap:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.brk
  .text
  .type std.syscall.brk, @function
std.syscall.brk:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov -16(%rbp), %rdi;mov $0, %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.BRK(%rip), %rax;syscall
.L.return.std.syscall.brk:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.rt_sigreturn
  .text
  .type std.syscall.rt_sigreturn, @function
std.syscall.rt_sigreturn:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov -16(%rbp), %rdi;mov $0, %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.RT_SIGRETURN(%rip), %rax;syscall
.L.return.std.syscall.rt_sigreturn:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.ioctl
  .text
  .type std.syscall.ioctl, @function
std.syscall.ioctl:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %esi, -16(%rbp)
  mov %rdx, -24(%rbp)
  mov -12(%rbp), %rdi;mov -16(%rbp), %rsi;mov -24(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.IOCTL(%rip), %rax;syscall
.L.return.std.syscall.ioctl:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.pread64
  .text
  .type std.syscall.pread64, @function
std.syscall.pread64:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov %rcx, -40(%rbp)
  mov -12(%rbp), %rdi;mov -24(%rbp), %rsi;mov -32(%rbp), %rdx;mov -40(%rbp), %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.PREAD64(%rip), %rax;syscall
.L.return.std.syscall.pread64:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.pwrite64
  .text
  .type std.syscall.pwrite64, @function
std.syscall.pwrite64:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -24(%rbp)
  mov %rdx, -32(%rbp)
  mov %rcx, -40(%rbp)
  mov -12(%rbp), %rdi;mov -24(%rbp), %rsi;mov -32(%rbp), %rdx;mov -40(%rbp), %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.PWRITE64(%rip), %rax;syscall
.L.return.std.syscall.pwrite64:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.readv
  .text
  .type std.syscall.readv, @function
std.syscall.readv:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -32(%rbp)
  mov %edx, -36(%rbp)
  mov -12(%rbp), %rdi;mov -32(%rbp), %rsi;mov -36(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.READV(%rip), %rax;syscall
.L.return.std.syscall.readv:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.writev
  .text
  .type std.syscall.writev, @function
std.syscall.writev:
  push %rbp
  mov %rsp, %rbp
  sub $48, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov %rsi, -32(%rbp)
  mov %edx, -36(%rbp)
  mov -12(%rbp), %rdi;mov -32(%rbp), %rsi;mov -36(%rbp), %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.WRITEV(%rip), %rax;syscall
.L.return.std.syscall.writev:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.access
  .text
  .type std.syscall.access, @function
std.syscall.access:
  push %rbp
  mov %rsp, %rbp
  sub $32, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov %esi, -20(%rbp)
  mov -16(%rbp), %rdi;mov -20(%rbp), %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.ACCESS(%rip), %rax;syscall
.L.return.std.syscall.access:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.pipe
  .text
  .type std.syscall.pipe, @function
std.syscall.pipe:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov -16(%rbp), %rdi;mov $0, %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.PIPE(%rip), %rax;syscall
.L.return.std.syscall.pipe:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.exit
  .text
  .type std.syscall.exit, @function
std.syscall.exit:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %edi, -12(%rbp)
  mov -12(%rbp), %rdi;mov $0, %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.EXIT(%rip), %rax;syscall
.L.return.std.syscall.exit:
  mov %rbp, %rsp
  pop %rbp
  ret
  .globl std.syscall.time
  .text
  .type std.syscall.time, @function
std.syscall.time:
  push %rbp
  mov %rsp, %rbp
  sub $16, %rsp
  mov %rsp, -8(%rbp)
  mov %rdi, -16(%rbp)
  mov -16(%rbp), %rdi;mov $0, %rsi;mov $0, %rdx;mov $0, %r10;mov $0, %r8;mov $0, %r9;mov std.Syscall.TIME(%rip), %rax;syscall
.L.return.std.syscall.time:
  mov %rbp, %rsp
  pop %rbp
  ret