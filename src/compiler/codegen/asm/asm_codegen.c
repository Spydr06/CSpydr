#include "asm_codegen.h"
#include "../../io/log.h"
#include "../../io/io.h"
#include "../../ast/ast_iterator.h"
#include "../../platform/platform_bindings.h"
#include "../codegen_utils.h"
#include "../../error/error.h"

#include <stdio.h>

static char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *argreg16[] = {"%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
static char *argreg32[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
static char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

enum { I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, F80 };

// The table for type casts
static char i32i8[] = "movsbl %al, %eax";
static char i32u8[] = "movzbl %al, %eax";
static char i32i16[] = "movswl %ax, %eax";
static char i32u16[] = "movzwl %ax, %eax";
static char i32f32[] = "cvtsi2ssl %eax, %xmm0";
static char i32i64[] = "movsxd %eax, %rax";
static char i32f64[] = "cvtsi2sdl %eax, %xmm0";
static char i32f80[] = "mov %eax, -4(%rsp); fildl -4(%rsp)";

static char u32f32[] = "mov %eax, %eax; cvtsi2ssq %rax, %xmm0";
static char u32i64[] = "mov %eax, %eax";
static char u32f64[] = "mov %eax, %eax; cvtsi2sdq %rax, %xmm0";
static char u32f80[] = "mov %eax, %eax; mov %rax, -8(%rsp); fildll -8(%rsp)";

static char i64f32[] = "cvtsi2ssq %rax, %xmm0";
static char i64f64[] = "cvtsi2sdq %rax, %xmm0";
static char i64f80[] = "movq %rax, -8(%rsp); fildll -8(%rsp)";

static char u64f32[] = "cvtsi2ssq %rax, %xmm0";
static char u64f64[] =
  "test %rax,%rax; js 1f; pxor %xmm0,%xmm0; cvtsi2sd %rax,%xmm0; jmp 2f; "
  "1: mov %rax,%rdi; and $1,%eax; pxor %xmm0,%xmm0; shr %rdi; "
  "or %rax,%rdi; cvtsi2sd %rdi,%xmm0; addsd %xmm0,%xmm0; 2:";
static char u64f80[] =
  "mov %rax, -8(%rsp); fildq -8(%rsp); test %rax, %rax; jns 1f;"
  "mov $1602224128, %eax; mov %eax, -4(%rsp); fadds -4(%rsp); 1:";

static char f32i8[] = "cvttss2sil %xmm0, %eax; movsbl %al, %eax";
static char f32u8[] = "cvttss2sil %xmm0, %eax; movzbl %al, %eax";
static char f32i16[] = "cvttss2sil %xmm0, %eax; movswl %ax, %eax";
static char f32u16[] = "cvttss2sil %xmm0, %eax; movzwl %ax, %eax";
static char f32i32[] = "cvttss2sil %xmm0, %eax";
static char f32u32[] = "cvttss2siq %xmm0, %rax";
static char f32i64[] = "cvttss2siq %xmm0, %rax";
static char f32u64[] = "cvttss2siq %xmm0, %rax";
static char f32f64[] = "cvtss2sd %xmm0, %xmm0";
static char f32f80[] = "movss %xmm0, -4(%rsp); flds -4(%rsp)";

static char f64i8[] = "cvttsd2sil %xmm0, %eax; movsbl %al, %eax";
static char f64u8[] = "cvttsd2sil %xmm0, %eax; movzbl %al, %eax";
static char f64i16[] = "cvttsd2sil %xmm0, %eax; movswl %ax, %eax";
static char f64u16[] = "cvttsd2sil %xmm0, %eax; movzwl %ax, %eax";
static char f64i32[] = "cvttsd2sil %xmm0, %eax";
static char f64u32[] = "cvttsd2siq %xmm0, %rax";
static char f64i64[] = "cvttsd2siq %xmm0, %rax";
static char f64u64[] = "cvttsd2siq %xmm0, %rax";
static char f64f32[] = "cvtsd2ss %xmm0, %xmm0";
static char f64f80[] = "movsd %xmm0, -8(%rsp); fldl -8(%rsp)";

#define FROM_F80_1                                           \
  "fnstcw -10(%rsp); movzwl -10(%rsp), %eax; or $12, %ah; " \
  "mov %ax, -12(%rsp); fldcw -12(%rsp); "

#define FROM_F80_2 " -24(%rsp); fldcw -10(%rsp); "

static char f80i8[] = FROM_F80_1 "fistps" FROM_F80_2 "movsbl -24(%rsp), %eax";
static char f80u8[] = FROM_F80_1 "fistps" FROM_F80_2 "movzbl -24(%rsp), %eax";
static char f80i16[] = FROM_F80_1 "fistps" FROM_F80_2 "movzbl -24(%rsp), %eax";
static char f80u16[] = FROM_F80_1 "fistpl" FROM_F80_2 "movswl -24(%rsp), %eax";
static char f80i32[] = FROM_F80_1 "fistpl" FROM_F80_2 "mov -24(%rsp), %eax";
static char f80u32[] = FROM_F80_1 "fistpl" FROM_F80_2 "mov -24(%rsp), %eax";
static char f80i64[] = FROM_F80_1 "fistpq" FROM_F80_2 "mov -24(%rsp), %rax";
static char f80u64[] = FROM_F80_1 "fistpq" FROM_F80_2 "mov -24(%rsp), %rax";
static char f80f32[] = "fstps -8(%rsp); movss -8(%rsp), %xmm0";
static char f80f64[] = "fstpl -8(%rsp); movsd -8(%rsp), %xmm0";

static char *cast_table[][11] = {
  // i8   i16     i32     i64     u8     u16     u32     u64     f32     f64     f80
  {NULL,  NULL,   NULL,   i32i64, i32u8, i32u16, NULL,   i32i64, i32f32, i32f64, i32f80}, // i8
  {i32i8, NULL,   NULL,   i32i64, i32u8, i32u16, NULL,   i32i64, i32f32, i32f64, i32f80}, // i16
  {i32i8, i32i16, NULL,   i32i64, i32u8, i32u16, NULL,   i32i64, i32f32, i32f64, i32f80}, // i32
  {i32i8, i32i16, NULL,   NULL,   i32u8, i32u16, NULL,   NULL,   i64f32, i64f64, i64f80}, // i64

  {i32i8, NULL,   NULL,   i32i64, NULL,  NULL,   NULL,   i32i64, i32f32, i32f64, i32f80}, // u8
  {i32i8, i32i16, NULL,   i32i64, i32u8, NULL,   NULL,   i32i64, i32f32, i32f64, i32f80}, // u16
  {i32i8, i32i16, NULL,   u32i64, i32u8, i32u16, NULL,   u32i64, u32f32, u32f64, u32f80}, // u32
  {i32i8, i32i16, NULL,   NULL,   i32u8, i32u16, NULL,   NULL,   u64f32, u64f64, u64f80}, // u64

  {f32i8, f32i16, f32i32, f32i64, f32u8, f32u16, f32u32, f32u64, NULL,   f32f64, f32f80}, // f32
  {f64i8, f64i16, f64i32, f64i64, f64u8, f64u16, f64u32, f64u64, f64f32, NULL,   f64f80}, // f64
  {f80i8, f80i16, f80i32, f80i64, f80u8, f80u16, f80u32, f80u64, f80f32, f80f64, NULL},   // f80
};

static void generate_files(ASMCodegenData_T* cg);
static void asm_gen_file_descriptors(ASMCodegenData_T* cg);
static void asm_gen_data(ASMCodegenData_T* cg, List_T* objs);
static void asm_gen_text(ASMCodegenData_T* cg, List_T* objs);
static void asm_assign_lvar_offsets(ASMCodegenData_T* cg);

void init_asm_cg(ASMCodegenData_T* cg, ASTProg_T* ast)
{
    cg->ast = ast;
    cg->print = false;
    cg->silent = false;

    cg->code_buffer = open_memstream(&cg->buf, &cg->buf_len);
    cg->current_fn = NULL;
    cg->depth = 0;
}

static void println(ASMCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);
    fprintf(cg->code_buffer, "\n");
}

static void print(ASMCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
}

static void write_code(ASMCodegenData_T* cg, const char* target_bin)
{
    char* homedir = get_home_directory();
    char cache_dir[BUFSIZ] = {'\0'};
    sprintf(cache_dir, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS, homedir);

    if(make_dir(cache_dir))
    {
        LOG_ERROR("error creating cache directory `" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "`.\n");
        exit(1);
    }

    char c_file_path[BUFSIZ * 2] = {'\0'};
    sprintf(c_file_path, "%s" DIRECTORY_DELIMS "%s.a", cache_dir, target_bin);

    fclose(cg->code_buffer);

    FILE* out = open_file(c_file_path);
    fwrite(cg->buf, cg->buf_len, 1, out);
    fclose(out);
}

void asm_gen_code(ASMCodegenData_T* cg, const char* target)
{
    if(!cg->silent)
    {
        LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_BOLD_WHITE " Assembly" COLOR_RESET " code\n");
    }

    // generate the assembly code
    asm_gen_file_descriptors(cg);
    asm_assign_lvar_offsets(cg);
    asm_gen_data(cg, cg->ast->objs);
    asm_gen_text(cg, cg->ast->objs);

    write_code(cg, target);

    if(cg->print)
        LOG_INFO_F("%s", cg->buf);

    char asm_source_file[BUFSIZ] = {'\0'};
    sprintf(asm_source_file, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "%s.a", get_home_directory(), target);

    char obj_file[BUFSIZ] = {'\n'};
    sprintf(obj_file, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "%s.o", get_home_directory(), target);
    
    // run the assembler
    {
        List_T* args = init_list(sizeof(char*));
        list_push(args, "as");
        list_push(args, "-c");
        list_push(args, asm_source_file);
        list_push(args, "-o");
        list_push(args, obj_file);
        list_push(args, NULL);

        i32 exit_code = subprocess(args->items[0], (char* const*) args->items, false);

        free_list(args);
        if(exit_code != 0)
        {
            LOG_ERROR_F("error assembling code. (exit code %d)\n", exit_code);
            exit(1);
        }
    }

    // run the linker
    {
        if(!cg->silent)
        {
            LOG_OK_F(COLOR_BOLD_BLUE "  Linking    " COLOR_BOLD_WHITE "%s" COLOR_RESET "\n", target);
        }

        List_T* args = init_list(sizeof(char*));
        list_push(args, "ld");
        list_push(args, "-o");
        list_push(args, (void*) target);
        list_push(args, "-m");
        list_push(args, "elf_x86_64");
        list_push(args, "-L/usr/lib/x86_64-linux-gnu");
        list_push(args, "-L/usr/lib64");
        list_push(args, "-L/lib64");
        list_push(args, "-L/usr/lib/x86_64-linux-gnu");
        list_push(args, "-L/usr/lib/x86_64-pc-linux-gnu");
        list_push(args, "-L/usr/lib/x86_64-redhat-linux");
        list_push(args, "-L/usr/lib");
        list_push(args, "-L/lib");
        list_push(args, "-lc");
        //list_push(args, "-lgcc");
        list_push(args, "--as-needed");
        //list_push(args, "-lgcc_s");
        list_push(args, "--no-as-needed");
        list_push(args, NULL);

        i32 exit_code = subprocess(args->items[0], (char* const*) args->items, false);
        
        free_list(args);
        if(exit_code != 0)
        {
            LOG_ERROR_F("error linking code. (exit code %d)\n", exit_code);
            exit(1);
        }
    }
}

static void asm_gen_file_descriptors(ASMCodegenData_T* cg)
{
    println(cg, "  .file 0 \"%s\"", cg->ast->main_file_path);
}

static void asm_assign_lvar_offsets(ASMCodegenData_T* cg)
{

}

static void asm_gen_data(ASMCodegenData_T* cg, List_T* objs)
{
    for(size_t i = 0; i < objs->size; i++)
    {
        ASTObj_T* obj = objs->items[i];
        switch(obj->kind)
        {
            case OBJ_NAMESPACE:
                asm_gen_text(cg, obj->objs);
                break;
            
            case OBJ_FUNCTION:
                if(obj->is_extern)
                    continue;
                
                
                break;
            
            default:
                continue;
        }
    }
}

static void asm_gen_text(ASMCodegenData_T* cg, List_T* objs)
{
    for(size_t i = 0; i < objs->size; i++)
    {
        ASTObj_T* obj = objs->items[i];
        switch(obj->kind)
        {
            case OBJ_NAMESPACE:
                asm_gen_text(cg, obj->objs);
                break;
            
            case OBJ_FUNCTION:
                {
                    if(obj->is_extern) 
                        continue;

                    char* fn_name = gen_identifier(obj->id);
                    println(cg, "  .globl %s", fn_name);
                    println(cg, "  .text");
                    println(cg, "  .type %s, @function", fn_name);
                    println(cg, "%s:", fn_name);

                    cg->current_fn = obj;

                    // prologue
                    println(cg, "  push %%rbp");
                    println(cg, "  mov %%rsp, %%rbp");
                    //println(cg, "  sub $%d, %%rsp", obj->stack_size);
                    //println(cg, "  mov %%rsp, %d(%%rbp)", obj)
                }
                break;

            default:
                continue;
        }
    }
}

static i32 get_type_id(ASTType_T *ty) {
    switch (ty->kind) {
        case TY_I8:
        case TY_CHAR:
            return I8;
        case TY_U8:
            return U8;
        case TY_I16:
            return I16;
        case TY_U16:
            return U16;
        case TY_I32:
            return I32;
        case TY_U32:
            return U32;
        case TY_I64:
            return I64;
        case TY_U64:
            return U64;
        case TY_F32:
            return F32;
        case TY_F64:
            return F64;
        case TY_F80:
            return F80;
        default:
            return U64;
    }
}

static u64 asm_count(void) 
{
    static u64 i = 1;
    return i++;
}

static void asm_push(ASMCodegenData_T* cg) 
{
    println(cg, "  push %%rax");
    cg->depth++;
}

static void asm_pop(ASMCodegenData_T* cg, char *arg) 
{
    println(cg, "  pop %s", arg);
    cg->depth--;
}

static void asm_pushf(ASMCodegenData_T* cg) 
{
    println(cg, "  sub $8, %%rsp");
    println(cg, "  movsd %%xmm0, (%%rsp)");
    cg->depth++;
}

static void asm_popf(ASMCodegenData_T* cg, i32 reg) 
{
    println(cg, "  movsd (%%rsp), %%xmm%d", reg);
    println(cg, "  add $8, %%rsp");
    cg->depth--;
}


static char *reg_dx(i32 sz) 
{
    switch (sz) {
        case 1: return "%dl";
        case 2: return "%dx";
        case 4: return "%edx";
        case 8: return "%rdx";
    }
    unreachable();
    return 0;
}

static char *reg_ax(i32 sz) 
{
    switch (sz) {
        case 1: return "%al";
        case 2: return "%ax";
        case 4: return "%eax";
        case 8: return "%rax";
    }
    unreachable();
    return 0;
}

static void asm_cmp_zero(ASMCodegenData_T* cg, ASTType_T* ty)
{
    switch(ty->kind)
    {
        case TY_F32:
            println(cg, "  xorps %%xmm1, %%xmm1");
            println(cg, "  ucomiss %%xmm1, %%xmm0");
            return;
        case TY_F64:
            println(cg, "  xorpd %%xmm1, %%xmm1");
            println(cg, "  ucomisd %%xmm1, %%xmm0");
            return;
        case TY_F80:
            println(cg, "  fldz");
            println(cg, "  fucomip");
            println(cg, "  fstp %%st(0)");
            return;
        default:
            break;
    }

    if(is_integer(ty) && ty->size <= 4)
        println(cg, "  cmp $0, %%eax");
    else
        println(cg, "  cmp $0, %%rax");
}

static void asm_gen_cast(ASMCodegenData_T* cg, ASTType_T* from, ASTType_T* to) 
{
    if(to->kind == TY_VOID)
        return;
    
    if(to->kind == TY_BOOL)
    {
        asm_cmp_zero(cg, from);
        println(cg, "  setne %%al");
        println(cg, "  movzx %%al, %%eax");
        return;
    }

    i32 t1 = get_type_id(from);
    i32 t2 = get_type_id(to);

    if(cast_table[t1][t2])
        println(cg, "  %s", cast_table[t1][t2]);
}

// Store %rax to an address that the stack top is pointing to.
static void asm_store(ASMCodegenData_T* cg, ASTType_T *ty) {
    asm_pop(cg, "%rdi");

    switch (ty->kind) {
        case TY_STRUCT:
            for (i32 i = 0; i < ty->size; i++) {
                println(cg, "  mov %d(%%rax), %%r8b", i);
                println(cg, "  mov %%r8b, %d(%%rdi)", i);
            }
            return;
        case TY_F32:
            println(cg, "  movss %%xmm0, (%%rdi)");
            return;
        case TY_F64:
            println(cg, "  movsd %%xmm0, (%%rdi)");
            return;
        case TY_F80:
            println(cg, "  fstpt (%%rdi)");
            return;
        default:    
            break;
    }

    if(ty->size == 1)
        println(cg, "  mov %%al, (%%rdi)");
    else if(ty->size == 2)
        println(cg, "  mov %%ax, (%%rdi)");
    else if(ty->size == 4)
        println(cg, "  mov %%eax, (%%rdi)");
    else
        println(cg, "  mov %%rax, (%%rdi)");
}

// Load a value from where %rax is pointing to.
static void asm_load(ASMCodegenData_T* cg, ASTType_T *ty) {
    switch (ty->kind) {
        case TY_ARR:
        case TY_STRUCT:
        case TY_FN:
        case TY_VA_LIST:
            return;
        case TY_F32:
            println(cg, "  movss (%%rax), %%xmm0");
            return;
        case TY_F64:
            println(cg, "  movsd (%%rax), %%xmm0");
            return;
        case TY_F80:
            println(cg, "  fldt (%%rax)");
            return;
        default:
            break;
    }

    char *insn = is_unsigned(ty) ? "movz" : "movs";

    // extend short values to an entire register
    if (ty->size == 1)
      println(cg, "  %sbl (%%rax), %%eax", insn);
    else if (ty->size == 2)
      println(cg, "  %swl (%%rax), %%eax", insn);
    else if (ty->size == 4)
      println(cg, "  movsxd (%%rax), %%rax");
    else
      println(cg, "  mov (%%rax), %%rax");
}

