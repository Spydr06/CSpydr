#include "asm_codegen.h"
#include "io/log.h"
#include "io/io.h"
#include "ast/ast_iterator.h"
#include "optimizer/constexpr.h"
#include "platform/platform_bindings.h"
#include "../codegen_utils.h"
#include "error/error.h"
#include "mem/mem.h"
#include "ast/types.h"
#include "ast/ast.h"
#include "config.h"
#include "globals.h"
#include "list.h"
#include "../relocation.h"
#include "timer/timer.h"
#include "linker.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#define either(a, b) ((a) ? (a) : (b))

#define CSPC_ASM_EXTERN_FN_POSTFIX "@GOTPCREL"

enum { I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, F80, LAST };

static const char* asm_start_text[] = 
{
    [MFK_NO_ARGS] =
        "  .globl _start\n"
        "  .text\n"
        "_start:\n"
        "  call .main\n"
        "  movq %rax, %rdi\n"
        "  movq $60, %rax\n"
        "  syscall",

    [MFK_ARGV_PTR] =
        "  .globl _start\n"
        "  .text\n"
        "_start:\n"
        "  xorl %ebp, %ebp\n"
        "  popq %rdi\n"
        "  movq %rsp, %rdi\n"
        "  call .main\n"
        "  movq %rax, %rdi\n"
        "  movq $60, %rax\n"
        "  syscall",

    [MFK_ARGC_ARGV_PTR] =
        "  .globl _start\n"
        "  .text\n"
        "_start:\n"
        "  xorl %ebp, %ebp\n"
        "  popq %rdi\n"
        "  movq %rsp, %rsi\n"
        "  andq $~15, %rsp\n"
        "  call .main\n"
        "  movq %rax, %rdi\n"
        "  movq $60, %rax\n"
        "  syscall",
    
    [MFK_ARGS_ARRAY] =
        "  .globl _start\n"
        "  .text\n"
        "_start:\n"
        "  xorl %ebp, %ebp\n"
        "  popq %rdi\n"
        "  movq %rsp, %rsi\n",
        
};

static char* jmp_mode[TOKEN_EOF] = {
    [TOKEN_EQ] = "je",
    [TOKEN_NOT_EQ] = "jne",
    [TOKEN_LT] = "jl",
    [TOKEN_LT_EQ] = "jle",
    [TOKEN_GT] = "jg",
    [TOKEN_GT_EQ] = "jge",
};

static char* argreg8[]  = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char* argreg16[] = {"%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
static char* argreg32[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
static char* argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char call_reg[] = "%r10";
static char pipe_reg[] = "%r15";

// The table for type casts
static char i32i8[]  = "movsbl %al, %eax";
static char i32u8[]  = "movzbl %al, %eax";
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

static char f32i8[]  = "cvttss2sil %xmm0, %eax; movsbl %al, %eax";
static char f32u8[]  = "cvttss2sil %xmm0, %eax; movzbl %al, %eax";
static char f32i16[] = "cvttss2sil %xmm0, %eax; movswl %ax, %eax";
static char f32u16[] = "cvttss2sil %xmm0, %eax; movzwl %ax, %eax";
static char f32i32[] = "cvttss2sil %xmm0, %eax";
static char f32u32[] = "cvttss2siq %xmm0, %rax";
static char f32i64[] = "cvttss2siq %xmm0, %rax";
static char f32u64[] = "cvttss2siq %xmm0, %rax";
static char f32f64[] = "cvtss2sd %xmm0, %xmm0";
static char f32f80[] = "movss %xmm0, -4(%rsp); flds -4(%rsp)";

static char f64i8[]  = "cvttsd2sil %xmm0, %eax; movsbl %al, %eax";
static char f64u8[]  = "cvttsd2sil %xmm0, %eax; movzbl %al, %eax";
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

static char f80i8[]  = FROM_F80_1 "fistps" FROM_F80_2 "movsbl -24(%rsp), %eax";
static char f80u8[]  = FROM_F80_1 "fistps" FROM_F80_2 "movzbl -24(%rsp), %eax";
static char f80i16[] = FROM_F80_1 "fistps" FROM_F80_2 "movzbl -24(%rsp), %eax";
static char f80u16[] = FROM_F80_1 "fistpl" FROM_F80_2 "movswl -24(%rsp), %eax";
static char f80i32[] = FROM_F80_1 "fistpl" FROM_F80_2 "mov -24(%rsp), %eax";
static char f80u32[] = FROM_F80_1 "fistpl" FROM_F80_2 "mov -24(%rsp), %eax";
static char f80i64[] = FROM_F80_1 "fistpq" FROM_F80_2 "mov -24(%rsp), %rax";
static char f80u64[] = FROM_F80_1 "fistpq" FROM_F80_2 "mov -24(%rsp), %rax";
static char f80f32[] = "fstps -8(%rsp); movss -8(%rsp), %xmm0";
static char f80f64[] = "fstpl -8(%rsp); movsd -8(%rsp), %xmm0";

static char *cast_table[LAST][LAST] = {
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

static void asm_gen_file_descriptors(ASMCodegenData_T* cg);
static void asm_gen_data(ASMCodegenData_T* cg, List_T* objs);
static void asm_gen_text(ASMCodegenData_T* cg, List_T* objs);
static void asm_gen_addr(ASMCodegenData_T* cg, ASTNode_T* node);
static void asm_assign_lvar_offsets(ASMCodegenData_T* cg, List_T* objs);
static bool asm_has_flonum(ASTType_T* ty, i32 lo, i32 hi, i32 offset);
static bool asm_has_flonum_1(ASTType_T* ty);
static bool asm_has_flonum_2(ASTType_T* ty);
static void asm_store_fp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz);
static void asm_store_gp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz);
static void asm_gen_stmt(ASMCodegenData_T* cg, ASTNode_T* node);
static void asm_gen_expr(ASMCodegenData_T* cg, ASTNode_T* node);
static void asm_gen_lambda(ASMCodegenData_T* cg, ASTObj_T* lambda);
static void asm_load(ASMCodegenData_T* cg, ASTType_T *ty);
static void asm_gen_string_literals(ASMCodegenData_T* cg);

void init_asm_cg(ASMCodegenData_T* cg, ASTProg_T* ast)
{
    memset(cg, 0, sizeof(struct ASM_CODEGEN_DATA_STRUCT));

    cg->ast = ast;
    cg->embed_file_locations = global.embed_debug_info;
    cg->code_buffer = open_memstream(&cg->buf, &cg->buf_len);
    cg->string_literals = init_list();
}

void free_asm_cg(ASMCodegenData_T* cg)
{
    free_list(cg->string_literals);
    free(cg->buf);
}

#ifdef __GNUC__
__attribute((format(printf, 2, 3)))
#endif
static void asm_print(ASMCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);
}

#ifdef __GNUC__
__attribute((format(printf, 2, 3)))
#endif
static void asm_println(ASMCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);    
    fputc('\n', cg->code_buffer);
}

static void write_code(ASMCodegenData_T* cg, const char* target, bool cachefile)
{
    char file_path[BUFSIZ * 2] = {'\0'};
    
    if(cachefile)
    {
        char* homedir = get_home_directory();
        char cache_dir[BUFSIZ] = {'\0'};

        if(cachefile)
            sprintf(cache_dir, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS, homedir);

        if(make_dir(cache_dir))
        {
            LOG_ERROR("error creating cache directory `" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "`.\n");
            throw(global.main_error_exception);
        }
        sprintf(file_path, "%s" DIRECTORY_DELIMS "%s.s", cache_dir, target);
    }
    else
        sprintf(file_path, "%s.s", target);

    fclose(cg->code_buffer);

    FILE* out = open_file(file_path);
    fwrite(cg->buf, cg->buf_len, 1, out);
    fclose(out);
}

i32 asm_codegen_pass(ASTProg_T* ast)
{
    ASMCodegenData_T cg;
    init_asm_cg(&cg, ast);
    cg.silent = global.silent;
    cg.print = global.print_code;
    cg.link_exec = ast->entry_point != NULL;

    asm_gen_code(&cg, global.target);
    free_asm_cg(&cg);

    return 0;
}

void asm_gen_code(ASMCodegenData_T* cg, const char* target)
{
    timer_start("assembly code generation");
    char platform[1024] = { '\0' };
    get_build(platform);
    if(!cg->silent)
    {
        LOG_OK_F(COLOR_BOLD_BLUE "  Generating" COLOR_BOLD_WHITE " assembly" COLOR_RESET " for " COLOR_BOLD_WHITE "%s" COLOR_RESET "\n", platform);
    }

    // generate the assembly code
    if(cg->embed_file_locations)
        asm_gen_file_descriptors(cg);
    asm_assign_lvar_offsets(cg, cg->ast->objs);
    asm_gen_data(cg, cg->ast->objs);
    if(cg->ast->entry_point)
        asm_println(cg, "%s", asm_start_text[cg->ast->mfk]);
    asm_gen_text(cg, cg->ast->objs);
    asm_gen_string_literals(cg);
    write_code(cg, target, global.do_assemble);

    if(cg->print)
    {
        if(!cg->silent)
            LOG_INFO(COLOR_RESET);
        fprintf(OUTPUT_STREAM, "%s", cg->buf);
    }

    timer_stop();

    if(!global.do_assemble)
        return;

    timer_start("assembling");

    char asm_source_file[BUFSIZ] = {'\0'};
    get_cached_file_path(asm_source_file, target, ".s");

    char obj_file[BUFSIZ] = {'\0'};
    if(global.do_link)
        get_cached_file_path(obj_file, target, ".o");
    else
        sprintf(obj_file, "%s.o", target);

    // run the assembler
    {
        const char* args[] = {
            DEFAULT_ASSEMBLER,
            "-c",
            asm_source_file,
            "-o",
            obj_file,
            NULL,
            NULL
        };

        if(cg->embed_file_locations)
            args[LEN(args) - 2] = "-g";

        i32 exit_code = subprocess(args[0], (char* const*) args, false);

        if(exit_code != 0)
        {
            LOG_ERROR_F("error assembling code. (exit code %d)\n", exit_code);
            throw(global.main_error_exception);
        }
    }

    timer_stop();

    // run the linker
    if(global.do_link) 
        link_obj(target, obj_file, cg->silent, cg->link_exec);
}

static char* asm_gen_identifier(ASTIdentifier_T* id)
{
    char* str = gen_identifier(id, ".", ".");
    mem_add_ptr(str);
    return str;
}

static void asm_gen_file_descriptors(ASMCodegenData_T* cg)
{
    for(size_t i = 0; i < cg->ast->files->size; i++)
    {
        File_T* file = cg->ast->files->items[i];
        asm_println(cg, "  .file %d \"%s\"", file->file_no + 1, file->path);
    }
}

static void asm_assign_lvar_offsets(ASMCodegenData_T* cg, List_T* objs)
{
    for(size_t i = 0; i < objs->size; i++)
    {
        ASTObj_T* obj = objs->items[i];
        switch (obj->kind) 
        {
        case OBJ_NAMESPACE:
            asm_assign_lvar_offsets(cg, obj->objs);
            break;
        
        case OBJ_FUNCTION:
        {
            if(obj->is_extern)
                continue;

            i32 top = 16, bottom = 0;
            i32 gp = 0, fp = 0;

            // Assign offsets to pass-by-stack parameters
            for(size_t j = 0; j < obj->args->size; j++)
            {
                ASTObj_T* var = obj->args->items[j];
                ASTType_T* ty = unpack(var->data_type);
                switch(ty->kind)
                {
                case TY_STRUCT:
                    if(ty->size <= 16)
                    {
                        bool fp1 = asm_has_flonum(ty, 0, 8, 0);
                        bool fp2 = asm_has_flonum(ty, 8, 16, 8);
                        if(fp + fp1 + fp2 < ASM_FP_MAX && gp + !fp1 + !fp2 < ASM_GP_MAX)
                        {
                            fp = fp + fp1 + fp2;
                            gp = gp + !fp1 + fp2;
                            continue;
                        }
                    } break;
                
                case TY_F32:
                case TY_F64:
                    if(fp++ < ASM_FP_MAX)
                        continue;
                    break;
                case TY_F80:
                    break;
                default:
                    if(gp++ < ASM_GP_MAX)
                        continue;
                }

                top = align_to(top, 8);
                var->offset = top;
                top += var->data_type->size;
            }

            // va_area
            if(is_variadic(obj->data_type))
            {
                bottom += obj->va_area->data_type->size;
                bottom = align_to(bottom, obj->va_area->data_type->align);
                obj->va_area->offset = -bottom;
            }

            // alloca size
            {
                bottom += obj->alloca_bottom->data_type->size;
                bottom = align_to(bottom, obj->alloca_bottom->data_type->align);
                obj->alloca_bottom->offset = -bottom;
            }

            if(obj->return_ptr)
            {
                bottom += obj->return_ptr->data_type->size;
                bottom = align_to(bottom, obj->return_ptr->data_type->align);
                obj->return_ptr->offset = -bottom;
            }

            // assign function argument offsets
            for(size_t j = 0; j < obj->args->size; j++)
            {
                ASTObj_T* var = obj->args->items[j];
                ASTType_T* ty = unpack(var->data_type);
                if(var->offset)
                    continue;
                
                // AMD64 System V ABI has a special alignment rule for an array of
			    // length at least 16 bytes. We need to align such array to at least
			    // 16-byte boundaries. See p.14 of
			    // https://github.com/hjl-tools/x86-psABI/wiki/x86-64-psABI-draft.pdf.
			    
                i32 align = (ty->kind == TY_C_ARRAY || ty->kind == TY_ARRAY) && ty->size >= 16 ? MAX(16, ty->align) : ty->align;
                bottom += ty->size;
                bottom = align_to(bottom, align);
                var->offset = -bottom;
            }

            // assign local offsets
            if(!obj->is_extern)
                for(size_t j = 0; j < obj->objs->size; j++)
                {
                    ASTObj_T* var = obj->objs->items[j];
                    i32 align = (var->data_type->kind == TY_C_ARRAY || var->data_type->kind == TY_ARRAY) && var->data_type->size >= 16 ? MAX(16, var->data_type->align) : var->data_type->align;
                    bottom += var->data_type->size;
                    bottom = align_to(bottom, align);
                    var->offset = -bottom;
                }

            obj->stack_size = align_to(bottom, 16);
        } break;

        default:
            break;
        }
    }
}

static void asm_gen_relocation(ASMCodegenData_T* cg, ASTObj_T* var)
{
    ASTNode_T* value = var->value;
    size_t target_size = var->data_type->size;
    u8* buffer = calloc(target_size, sizeof(u8));
    gen_relocation(value, target_size, buffer);
    for(size_t i = 0; i < target_size; i++) 
        asm_println(cg, "  .byte %d", (int) buffer[i]);
    
    free(buffer);
}

static void asm_gen_data(ASMCodegenData_T* cg, List_T* objs)
{
    for(size_t i = 0; i < objs->size; i++)
    {
        ASTObj_T* obj = objs->items[i];
        switch(obj->kind)
        {
            case OBJ_NAMESPACE:
                asm_gen_data(cg, obj->objs);
                break;
            
            case OBJ_TYPEDEF:
            {
                ASTType_T* ty = unpack(obj->data_type);
                if(!ty)
                    continue;
                if(ty->kind == TY_ENUM)
                    for(size_t i = 0; i < ty->members->size; i++)
                    {
                        ASTObj_T* member = ty->members->items[i];
                        if(!should_emit(member))
                            continue;
                        char* id = asm_gen_identifier(member->id);
                        asm_println(cg, "  .globl %s", id);
                        asm_println(cg, "  .data");
                        asm_println(cg, "  .type %s, @object", id);
                        asm_println(cg, "  .size %s, 4", id);
                        asm_println(cg, "  .align 4");
                        asm_println(cg, "%s:", id);

                        asm_gen_relocation(cg, member); 
                    }
            } break;

            case OBJ_GLOBAL:
                if(obj->is_extern || !should_emit(obj))
                    continue;
                {
                    char* id = asm_gen_identifier(obj->id);
                    asm_println(cg, "  .globl %s", id);

                    i32 align = (obj->data_type->kind == TY_C_ARRAY || obj->data_type->kind == TY_ARRAY) && obj->data_type->size >= 16 ? MAX(16, obj->data_type->align) : obj->data_type->align;

                    if(obj->value)
                    {
                        asm_println(cg, "  .data");
                        asm_println(cg, "  .type %s, @object", id);
                        asm_println(cg, "  .size %s, %d", id, obj->data_type->size);
                        asm_println(cg, "  .align %d", align);
                        asm_println(cg, "%s:", id);

                        asm_gen_relocation(cg, obj);

                        continue;
                    }
                    else if(unpack(obj->data_type)->kind == TY_ARRAY)
                    {
                        asm_println(cg, "  .data");
                        asm_println(cg, "  .type %s, @object", id);
                        asm_println(cg, "  .size %s, %d", id, obj->data_type->size);
                        asm_println(cg, "  .align %d", align);
                        asm_println(cg, "%s:", id);

                        union {u64 num; u8 bytes[8];} length;
                        length.num = unpack(obj->data_type)->num_indices;

                        for(i8 i = 0; i < 8; i++)
                            asm_println(cg, "  .byte %d", length.bytes[i]);
                        asm_println(cg, "  .zero %d", obj->data_type->size - 8);
                    }
                    else
                    {
                        asm_println(cg, "  .bss");    
                        asm_println(cg, "  .align %d", align);
                        asm_println(cg, "%s:", id);
                        asm_println(cg, "  .zero %d", obj->data_type->size);
                    }
                } break;

            default:
                continue;
        }
    }
}

static void asm_push(ASMCodegenData_T* cg) 
{
    asm_println(cg, "  push %%rax");
    cg->depth++;
}

static void asm_pop(ASMCodegenData_T* cg, char *arg) 
{
    asm_println(cg, "  pop %s", arg);
    cg->depth--;
}

static void asm_gen_defer(ASMCodegenData_T* cg, List_T* deferred) 
{
    if(!deferred || deferred->size == 0)
        return;
    
    asm_push(cg);
    for(size_t i = 0; i < deferred->size; i++)
    {
        asm_gen_stmt(cg, deferred->items[i]);
    }
    asm_pop(cg, "%rax");
}

static void asm_gen_function_signature(ASMCodegenData_T* cg, const char* fn_name)
{
    asm_println(cg, "  .globl %s", fn_name);
    asm_println(cg, "  .text");
    asm_println(cg, "  .type %s, @function", fn_name);
    asm_println(cg, "%s:", fn_name);
}

static void asm_gen_function(ASMCodegenData_T* cg, ASTObj_T* obj)
{
    char* fn_name = asm_gen_identifier(obj->id);
    asm_gen_function_signature(cg, fn_name);
    if(obj->exported)
        asm_gen_function_signature(cg, obj->exported);

    cg->current_fn = obj;

    // prologue
    asm_println(cg, "  push %%rbp");
    asm_println(cg, "  mov %%rsp, %%rbp");
    asm_println(cg, "  sub $%d, %%rsp", obj->stack_size);
    asm_println(cg, "  mov %%rsp, %d(%%rbp)", obj->alloca_bottom->offset);

    if(cg->embed_file_locations)
        asm_println(cg, "  .loc %d %d", obj->tok->source->file_no + 1, obj->tok->line + 1);

    // save arg registers if function is variadic
    if(is_variadic(obj->data_type))
    {
        i32 gp = 0, fp = 0;
        if(obj->return_ptr)
            gp++;
        for(size_t j = 0; j < obj->args->size; j++)
        {
            ASTObj_T* arg = obj->args->items[j];
            if(is_flonum(unpack(arg->data_type)))
                fp++;
            else
                gp++;
        }

        i32 off = obj->va_area->offset;

        // va_elem
		asm_println(cg, "  movl $%d, %d(%%rbp)", gp * 8, off);          // gp_offset
		asm_println(cg, "  movl $%d, %d(%%rbp)", fp * 8 + 48, off + 4); // fp_offset
		asm_println(cg, "  movq %%rbp, %d(%%rbp)", off + 8);            // overflow_arg_area
		asm_println(cg, "  addq $16, %d(%%rbp)", off + 8);
		asm_println(cg, "  movq %%rbp, %d(%%rbp)", off + 16);           // reg_save_area
		asm_println(cg, "  addq $%d, %d(%%rbp)", off + 24, off + 16);
            
		// __reg_save_area__
	    asm_println(cg, "  movq %%rdi, %d(%%rbp)", off + 24);
	    asm_println(cg, "  movq %%rsi, %d(%%rbp)", off + 32);
	    asm_println(cg, "  movq %%rdx, %d(%%rbp)", off + 40);
	    asm_println(cg, "  movq %%rcx, %d(%%rbp)", off + 48);
	    asm_println(cg, "  movq %%r8, %d(%%rbp)", off + 56);
	    asm_println(cg, "  movq %%r9, %d(%%rbp)", off + 64);
	    asm_println(cg, "  movsd %%xmm0, %d(%%rbp)", off + 72);
	    asm_println(cg, "  movsd %%xmm1, %d(%%rbp)", off + 80);
	    asm_println(cg, "  movsd %%xmm2, %d(%%rbp)", off + 88);
	    asm_println(cg, "  movsd %%xmm3, %d(%%rbp)", off + 96);
	    asm_println(cg, "  movsd %%xmm4, %d(%%rbp)", off + 104);
	    asm_println(cg, "  movsd %%xmm5, %d(%%rbp)", off + 112);
	    asm_println(cg, "  movsd %%xmm6, %d(%%rbp)", off + 120);
	    asm_println(cg, "  movsd %%xmm7, %d(%%rbp)", off + 128);
    }

    // save passed-by-register arguments to the stack
    i32 gp = 0, fp = 0;

    if(obj->return_ptr)
        asm_store_gp(cg, gp++, obj->return_ptr->offset, obj->return_ptr->data_type->size);

    for(size_t j = 0; j < obj->args->size; j++)
    {
        ASTObj_T* arg = obj->args->items[j];
        if(arg->offset > 0)
            continue;
                        
        ASTType_T* ty = unpack(arg->data_type);

        switch(ty->kind)
        {
            case TY_STRUCT:
                assert(ty->size <= 16);
                if(asm_has_flonum(ty, 0, 8, 0))
                    asm_store_fp(cg, fp++, arg->offset, MIN(8, ty->size));
                else
                    asm_store_gp(cg, gp++, arg->offset, MIN(8, ty->size));
                                
                if(ty->size > 8)
                {
                    if(asm_has_flonum(ty, 8, 16, 0))
                        asm_store_fp(cg, fp++, arg->offset + 8, ty->size - 8);
                    else
                        asm_store_gp(cg, gp++, arg->offset + 8, ty->size - 8);
                }
                break;
            case TY_F32:
            case TY_F64:
                asm_store_fp(cg, fp++, arg->offset, ty->size);
                break;
            default:
                asm_store_gp(cg, gp++, arg->offset, ty->size);
        }
    }

    // emit code
    cg->current_fn_name = fn_name;
    asm_gen_stmt(cg, obj->body);
    if(cg->depth != 0) {
        cg->depth = 0;
        throw_error(ERR_CODEGEN_WARN, obj->tok, "cg->depth is not 0");
    }
    cg->current_fn_name = NULL;

    // epilogue
    asm_println(cg, ".L.return.%s:", fn_name);
    asm_gen_defer(cg, obj->deferred);
    asm_println(cg, "  mov %%rbp, %%rsp");
    asm_println(cg, "  pop %%rbp");
    asm_println(cg, "  ret");
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
                if(obj->is_extern || !should_emit(obj)) 
                    continue;
                asm_gen_function(cg, obj);
                break;
            
            case OBJ_LAMBDA:
                asm_gen_lambda(cg, obj);
                break;

            default:
                continue;
        }
    }
}

static i32 get_type_id(ASTType_T *ty) {
    switch (unpack(ty)->kind) {
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

static void asm_pushf(ASMCodegenData_T* cg) 
{
    asm_println(cg, "  sub $8, %%rsp");
    asm_println(cg, "  movsd %%xmm0, (%%rsp)");
    cg->depth++;
}

static void asm_popf(ASMCodegenData_T* cg, i32 reg) 
{
    asm_println(cg, "  movsd (%%rsp), %%xmm%d", reg);
    asm_println(cg, "  add $8, %%rsp");
    cg->depth--;
}

static void asm_gen_index(ASMCodegenData_T* cg, ASTNode_T* index, bool gen_address)
{
    asm_gen_expr(cg, index->left);
    asm_push(cg);

    switch(unpack(index->left->data_type)->kind)
    {
        case TY_PTR:
        case TY_FN:
            asm_gen_expr(cg, index->expr);
            asm_pop(cg, "%rdi");
            asm_println(cg, "  imul $%d, %%rax", index->data_type->size);
            break;

        case TY_C_ARRAY:
            asm_gen_expr(cg, index->expr);
            asm_pop(cg, "%rdi");
            if(index->from_back)
            {
                asm_println(cg, "  imul $%d, %%rax", -index->data_type->size);
                asm_println(cg, "  add $%d, %%rax", index->left->data_type->size + index->data_type->size);
            }
            else
                asm_println(cg, "  imul $%d, %%rax", index->data_type->size);
            break;

        case TY_VLA:
        case TY_ARRAY:
            if(index->from_back)
            {                
                asm_load(cg, (ASTType_T*) primitives[TY_U64]);
                asm_push(cg);
                asm_gen_expr(cg, index->expr);
                asm_pop(cg, "%rcx"); // length
                asm_pop(cg, "%rdi"); // pointer

                asm_println(cg, "  imul $%d, %%rax", -index->data_type->size);
                asm_println(cg, "  imul $%d, %%rcx", index->data_type->size);
                asm_println(cg, "  add %%rcx, %%rax");
            }
            else
            {
                asm_gen_expr(cg, index->expr);
                asm_pop(cg, "%rdi");
                asm_println(cg, "  imul $%d, %%rax", index->data_type->size);
            }
            asm_println(cg, "  add $8, %%rax");
            break;

        default:
            throw_error(ERR_CODEGEN, index->tok, "wrong index type");
    }

    asm_println(cg, "  add %%rdi, %%rax");

    if(!gen_address)
        asm_load(cg, index->data_type);
}

static void asm_gen_addr(ASMCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_CLOSURE:
            asm_gen_addr(cg, node->exprs->items[node->exprs->size - 1]);
            return;

        case ND_ID: 
            if(!node->data_type)
                node->data_type = node->referenced_obj->data_type;

            if(unpack(node->data_type)->kind == TY_VLA)
            {
                asm_println(cg, "  mov %d(%%rbp), %%rax", node->referenced_obj->offset);
                return;
            }

            switch(node->referenced_obj->kind)
            {
                case OBJ_FN_ARG:
                case OBJ_LOCAL:
                    asm_println(cg, "  lea %d(%%rbp), %%rax", node->referenced_obj->offset);
                    return;

                case OBJ_GLOBAL:
                case OBJ_ENUM_MEMBER:
                    asm_println(cg, "  %s %s(%%rip), %%rax", node->call ? "movq" : "lea", node->referenced_obj->is_extern_c ? node->id->callee : asm_gen_identifier(node->id));
                    return;
                
                case OBJ_FUNCTION:
                    if(node->call)
                    {
                        ASTObj_T* obj = node->call->referenced_obj;
                        if(obj->is_extern_c)
                            asm_println(cg, "  mov %s" CSPC_ASM_EXTERN_FN_POSTFIX "(%%rip), %%rax", either(obj->exported, node->id->callee));
                        else if(obj->is_extern)
                            asm_println(cg, "  mov %s" CSPC_ASM_EXTERN_FN_POSTFIX "(%%rip), %%rax", either(obj->exported, asm_gen_identifier(node->id)));
                        else if(obj->kind != OBJ_FUNCTION)
                        {
                            asm_println(cg, "  lea %d(%%rbp), %%rax", node->referenced_obj->offset);
                            asm_println(cg, "  mov (%%rax), %%rax");
                        }
                        else
                            asm_println(cg, "  lea %s(%%rip), %%rax", asm_gen_identifier(node->id));
                    }
                    else if(node->referenced_obj->is_extern_c)
                        asm_println(cg, "  mov %s" CSPC_ASM_EXTERN_FN_POSTFIX "(%%rip), %%rax", either(node->referenced_obj->exported, node->id->callee));
                    else if(node->referenced_obj->is_extern)
                        asm_println(cg, "  mov %s" CSPC_ASM_EXTERN_FN_POSTFIX "(%%rip), %%rax", either(node->referenced_obj->exported, asm_gen_identifier(node->id)));
                    else
                        asm_println(cg, "  lea %s(%%rip), %%rax", asm_gen_identifier(node->id));
                    return;
                
                default:
                    throw_error(ERR_CODEGEN, node->tok, "`%s` reference object of unexpected kind", node->id->callee);
            }
            return;
        
        case ND_CALL:
            if(node->referenced_obj)
                asm_gen_expr(cg, node);
            return;

        case ND_DEREF:
            asm_gen_expr(cg, node->right);
            return;
        case ND_MEMBER:
            asm_gen_addr(cg, node->left);
            asm_println(cg, "  add $%ld, %%rax", node->body->offset);
            if(unpack(node->data_type)->kind == TY_FN)
                asm_println(cg, "  mov (%%rax), %%rax");
            return;
        case ND_INDEX:
            asm_gen_index(cg, node, true);
            return;
        case ND_HOLE:
            asm_println(cg, "  mov %s, %%rax", pipe_reg);
            return;
        case ND_TERNARY:
        case ND_ELSE_EXPR:
            if(unpack(node->data_type)->kind == TY_STRUCT)
            {
                asm_gen_expr(cg, node);
                return;
            }
            // fall through
        case ND_LAMBDA:
        case ND_ARRAY:
        case ND_STRUCT:
            asm_gen_expr(cg, node);
            return;
        case ND_CAST:
            asm_gen_addr(cg, node->left);
            return;
        default:
            throw_error(ERR_CODEGEN, node->tok, "cannot generate address from node of kind %d", node->kind);
            break;
    }
}

static bool asm_has_flonum(ASTType_T* ty, i32 lo, i32 hi, i32 offset)
{
    ty = unpack(ty);
    if(ty->kind == TY_STRUCT)
    {
        for(size_t i = 0; i < ty->members->size; i++)
        {
            ASTNode_T* member = ty->members->items[i];
            if(!asm_has_flonum(member->data_type, lo, hi, offset + member->offset /*offset*/))
                return false;
        }
        return true;
    }
    else if(ty->kind == TY_C_ARRAY || ty->kind == TY_ARRAY)
    {
        for(size_t i = 0; i < (size_t)(ty->size / ty->base->size); i++)
            if(!asm_has_flonum(ty->base, lo, hi, offset + ty->base->size * i))
                return false;
        return true;
    }

    return offset < lo || hi <= offset || ty->kind == TY_F32 || ty->kind == TY_F64; 
}

static bool asm_has_flonum_1(ASTType_T* ty)
{
    return asm_has_flonum(ty, 0, 8, 0);
}

static bool asm_has_flonum_2(ASTType_T* ty)
{
    return asm_has_flonum(ty, 0, 16, 0);
}

static void asm_push_struct(ASMCodegenData_T* cg, ASTType_T* ty)
{
    i32 sz = align_to(unpack(ty)->size, 8);
    asm_println(cg, "  sub $%d, %%rsp", sz);
    cg->depth += sz / 8;
    
    for(i32 i = 0; i < unpack(ty)->size; i++)
    {
        asm_println(cg, "  mov %d(%%rax), %sb", i, call_reg);
        asm_println(cg, "  mov %sb, %d(%%rsp)", call_reg, i);
    }
}

static void asm_copy_struct_reg(ASMCodegenData_T* cg)
{
    ASTType_T* ty = unpack(cg->current_fn->return_type);
    i32 gp = 0, fp = 0;

    asm_println(cg, "  mov %%rax, %%rdi");

    if(asm_has_flonum(ty, 0, 8, 0))
    {
        assert(ty->size == 4 || 8 <= ty->size);
        if(ty->size == 4)
            asm_println(cg, "  movss (%%rdi), %%xmm0");
        else
            asm_println(cg, "  movsd (%%rdi), %%xmm0");
        fp++;
    }
    else
    {
        asm_println(cg, "  xor %%rax, %%rax");
        for(i32 i = MIN(8, ty->size) - 1; i >= 0; i--)
        {
            asm_println(cg, "  shl $8, %%rax");
            asm_println(cg, "  mov %d(%%rdi), %%al", i);
        }
        gp++;
    }

    if(ty->size > 8)
    {
        if(asm_has_flonum(ty, 8, 16, 0) && ty->kind != TY_STRUCT)
        {
            assert(ty->size == 12 || ty->size == 16);
            if(ty->size == 4)
                asm_println(cg, "  movss 8(%%rdi), %%xmm%d", fp);
            else
                asm_println(cg, "  movsd 8(%%rdi), %%xmm%d", fp);
        }
        else
        {
            char* reg1 = (gp == 0) ? "%al" : "%dl";
            char* reg2 = (gp == 0) ? "%rax" : "%rdx";
            asm_println(cg, " xor %s, %s", reg2, reg2);
            for(i32 i = MIN(16, ty->size) - 1; i >= 8; i--)
            {
                asm_println(cg, "  shl $8, %s", reg2);
                asm_println(cg, "  mov %d(%%rdi), %s", i, reg1);
            }
        }
    }
}

static void asm_copy_struct_mem(ASMCodegenData_T* cg, ASTNode_T* val)
{
    ASTType_T* ty = cg->current_fn->return_type;
    assert(ty != NULL);
    assert(cg->current_fn->return_ptr != NULL);
    asm_println(cg, "  mov %d(%%rbp), %%rdi", cg->current_fn->return_ptr->offset);

    for(i32 i = 0; i < ty->size; i++)
    {
        asm_println(cg, "  mov %d(%%rax), %%dl", i);
        asm_println(cg, "  mov %%dl, %d(%%rdi)", i);
    }
}

static void asm_copy_ret_buffer(ASMCodegenData_T* cg, ASTObj_T* var)
{
    ASTType_T* ty = unpack(var->data_type);
    i32 gp = 0, fp = 0;

    if(asm_has_flonum_1(ty))
    {
        assert(ty->size == 4 || 8 <= ty->size);
        if(ty->size == 4)
            asm_println(cg, "  movss %%xmm0, %d(%%rbp)", var->offset);
        else
            asm_println(cg, "  movsd %%xmm0, %d(%%rbp)", var->offset);
        fp++;
    }
    else
    {
        for(i32 i = 0; i < MIN(8, ty->size); i++)
        {
            asm_println(cg, "  mov %%al, %d(%%rbp)", var->offset + i);
            asm_println(cg, "  shr $8, %%rax");
        }
        gp++;
    }

    if(ty->size > 8)
    {
        if(asm_has_flonum_2(ty))
        {
            assert(ty->size == 12 || ty->size == 16);
            if(ty->size == 12)
                asm_println(cg, "  movss %%xmm%d, %d(%%rbp)", fp, var->offset + 8);
            else
                asm_println(cg, "  movsd %%xmm%d, %d(%%rbp)", fp, var->offset + 8);
        }
        else
        {
            char* reg1 = gp == 0 ? "%al" : "%dl";
            char* reg2 = gp == 0 ? "%rax" : "%rdx";
            for(i32 i = 8; i < MIN(16, ty->size); i++) 
            {
                asm_println(cg, "  mov %s, %d(%%rbp)", reg1, var->offset + i);
                asm_println(cg, "  shr $8, %s", reg2);
            }
        }
    }
}

static void asm_cmp_zero(ASMCodegenData_T* cg, ASTType_T* ty)
{
    ty = unpack(ty);
    switch(ty->kind)
    {
        case TY_F32:
            asm_println(cg, "  xorps %%xmm1, %%xmm1");
            asm_println(cg, "  ucomiss %%xmm1, %%xmm0");
            return;
        case TY_F64:
            asm_println(cg, "  xorpd %%xmm1, %%xmm1");
            asm_println(cg, "  ucomisd %%xmm1, %%xmm0");
            return;
        case TY_F80:
            asm_println(cg, "  fldz");
            asm_println(cg, "  fucomip");
            asm_println(cg, "  fstp %%st(0)");
            return;
        default:
            break;
    }

    if(is_integer(ty) && ty->size <= 4)
        asm_println(cg, "  cmp $0, %%eax");
    else
        asm_println(cg, "  cmp $0, %%rax");
}

static void asm_cast(ASMCodegenData_T* cg, ASTType_T* from, ASTType_T* to) 
{
    if(to->kind == TY_VOID)
        return;
    
    if(to->kind == TY_BOOL)
    {
        asm_cmp_zero(cg, from);
        asm_println(cg, "  setne %%al");
        asm_println(cg, "  movzx %%al, %%eax");
        return;
    }

    i32 t1 = get_type_id(from);
    i32 t2 = get_type_id(to);

    if(cast_table[t1][t2])
        asm_println(cg, "  %s", cast_table[t1][t2]);
}

// Store %rax to an address that the stack top is pointing to.
static void asm_store(ASMCodegenData_T* cg, ASTType_T *ty) {
    asm_pop(cg, "%rdi");
    ty = unpack(ty);

    switch (ty->kind) {
        case TY_STRUCT:
        case TY_ARRAY:
        case TY_C_ARRAY:
            for(i32 i = 0; i < ty->size; i++) {
                asm_println(cg, "  mov %d(%%rax), %%r8b", i);
                asm_println(cg, "  mov %%r8b, %d(%%rdi)", i);
            }
            return;
        case TY_F32:
            asm_println(cg, "  movss %%xmm0, (%%rdi)");
            return;
        case TY_F64:
            asm_println(cg, "  movsd %%xmm0, (%%rdi)");
            return;
        case TY_F80:
            asm_println(cg, "  fstpt (%%rdi)");
            return;
        default:    
            break;
    }

    if(ty->size == 1)
        asm_println(cg, "  mov %%al, (%%rdi)");
    else if(ty->size == 2)
        asm_println(cg, "  mov %%ax, (%%rdi)");
    else if(ty->size == 4)
        asm_println(cg, "  mov %%eax, (%%rdi)");
    else
        asm_println(cg, "  mov %%rax, (%%rdi)");
}

// Load a value from where %rax is pointing to.
static void asm_load(ASMCodegenData_T* cg, ASTType_T *ty) {
    ty = unpack(ty);
    switch (ty->kind) {
        case TY_C_ARRAY:
        case TY_STRUCT:
        case TY_ARRAY:
        case TY_VLA:
        case TY_FN:
            return;
        case TY_F32:
            asm_println(cg, "  movss (%%rax), %%xmm0");
            return;
        case TY_F64:
            asm_println(cg, "  movsd (%%rax), %%xmm0");
            return;
        case TY_F80:
            asm_println(cg, "  fldt (%%rax)");
            return;
        default:
            break;
    }

    char *insn = is_unsigned(ty) ? "movz" : "movs";

    // extend short values to an entire register
    if (ty->size == 1)
        asm_println(cg, "  %sbl (%%rax), %%eax", insn);
    else if (ty->size == 2)
        asm_println(cg, "  %swl (%%rax), %%eax", insn);
    else if (ty->size == 4)
        asm_println(cg, "  movsxd (%%rax), %%rax");
    else
        asm_println(cg, "  mov (%%rax), %%rax");
}

static ASTNode_T* make_index_expr(ASTNode_T* left, size_t index)
{
    ASTNode_T* index_expr = init_ast_node(ND_LONG, left->tok);
    index_expr->data_type = (ASTType_T*) primitives[TY_U64];
    index_expr->long_val = index;

    ASTNode_T* expr = init_ast_node(ND_INDEX, left->tok);
    expr->data_type = unpack(left->data_type)->base;
    expr->left = left;
    expr->expr = index_expr;

    return expr;
}

static ASTNode_T* make_member_expr(ASTNode_T* left, ASTNode_T* struct_member)
{
    ASTNode_T* right = init_ast_node(ND_ID, left->tok);
    right->data_type = struct_member->data_type;

    ASTNode_T* expr = init_ast_node(ND_MEMBER, left->tok);
    expr->data_type = struct_member->data_type;
    expr->left = left;
    expr->right = right;
    expr->body = struct_member;

    return expr;
}

static void unpack_array(List_T* unpacked_args, ASTNode_T* arg, ASTType_T* type)
{
    size_t num_indices = type->num_indices;
    for(size_t j = 0; j < num_indices; j++)
    {
        size_t index = arg->unpack_mode == UMODE_FTOB ? j : num_indices - j - 1;
        list_push(unpacked_args, make_index_expr(arg, index));
    }
}

static void unpack_struct(List_T* unpacked_args, ASTNode_T* arg, ASTType_T* type)
{
    size_t num_members = type->members->size;
    for(size_t j = 0; j < num_members; j++)
    {
        ASTNode_T* member = type->members->items[arg->unpack_mode == UMODE_FTOB ? j : num_members - j - 1];
        list_push(unpacked_args, make_member_expr(arg, member));
    }
}

static List_T* unpack_call_args(ASTNode_T* node)
{
    if((node->called_obj && node->called_obj->data_type && !unpack(node->called_obj->data_type)->is_variadic))
        return node->args;
    
    bool needs_unpacking = false;
    for(size_t i = 0; i < node->args->size; i++)
    {
        ASTNode_T* arg = node->args->items[i];
        if(arg->unpack_mode) {
            needs_unpacking = true;
            break;
        }
    }

    if(!needs_unpacking)
        return node->args;

    List_T* unpacked_args = init_list();

    for(size_t i = 0; i < node->args->size; i++)
    {
        ASTNode_T* arg = node->args->items[i];
        if(!arg->unpack_mode)
        {
            list_push(unpacked_args, arg);
            continue;
        }
        
        ASTType_T* arg_type = unpack(arg->data_type);
        switch (arg_type->kind) {
            case TY_ARRAY:
            case TY_C_ARRAY:
                unpack_array(unpacked_args, arg, arg_type);
                break;
            case TY_STRUCT:
                unpack_struct(unpacked_args, arg, arg_type);
                break;
            default:
                unreachable();
        }
    }

    return unpacked_args;
}

static void asm_push_args2(ASMCodegenData_T* cg, List_T* args, bool first_pass)
{
    for(i64 i = args->size - 1; i >= 0; i--)
    {
        ASTNode_T* arg = args->items[i];
        if((first_pass && !arg->pass_by_stack) || (!first_pass && arg->pass_by_stack))
            continue;
        
        asm_gen_expr(cg, arg);

        switch(unpack(arg->data_type)->kind)
        {
            case TY_STRUCT:
                asm_push_struct(cg, arg->data_type);
                break;
            case TY_F32:
            case TY_F64:
                asm_pushf(cg);
                break;
            case TY_F80:
                asm_println(cg, "  sub $16, %%rsp");
                asm_println(cg, "  fstpt (%%rsp)");
                cg->depth += 2;
                break;
            default:
                asm_push(cg);
        }
    }
}

static i32 asm_push_args(ASMCodegenData_T* cg, ASTNode_T* node)
{
    i32 stack = 0, gp = 0, fp = 0;

    // If the return type is a large struct/union, the caller passes
	// a pointer to a buffer as if it were the first argument.
	if (node->return_buffer && node->data_type->size > 16)
		gp++;

    for(size_t i = 0; i < node->args->size; i++)
    {
        ASTNode_T* arg = node->args->items[i];
        ASTType_T* ty;
        if(arg->referenced_obj && arg->referenced_obj->kind == OBJ_GLOBAL)
            ty = unpack(arg->referenced_obj->data_type);
        else 
            ty = unpack(arg->data_type);
        switch(ty->kind)
        {
            case TY_STRUCT:
                if(ty->size > 16)
                {
                    arg->pass_by_stack = true;
                    stack += align_to(ty->size, 8) / 8;
                }
                else
                {
                    bool fp1 = asm_has_flonum_1(ty);
                    bool fp2 = asm_has_flonum_2(ty);

                    if(fp + fp1 + fp2 < ASM_FP_MAX && gp + !fp1 + !fp2 < ASM_GP_MAX)
                    {
                        fp += fp1 + fp2;
                        gp += !fp1 + !fp2;
                    }
                    else
                    {
                        arg->pass_by_stack = true;
                        stack += align_to(ty->size, 8) / 8;
                    }
                } break;
            case TY_F32:
            case TY_F64:
                if(fp++ >= ASM_FP_MAX)
                {
                    arg->pass_by_stack = true;
                    stack++;
                } break;
            case TY_F80:
                arg->pass_by_stack = true;
                stack += 2;
                break;
            default:
                if(gp++ >= ASM_GP_MAX) {
                    arg->pass_by_stack = true;
                    stack++;
                }
        }
    }

    if((cg->depth + stack) % 2 == 1)
    {
        asm_println(cg, "  sub $8, %%rsp");
        cg->depth++;
        stack++;
    }

    asm_push_args2(cg, node->args, true);
    asm_push_args2(cg, node->args, false);

	// If the return type is a large struct/union, the caller passes
	// a pointer to a buffer as if it were the first argument.
    if(node->return_buffer && node->data_type->size > 16)
    {
        asm_println(cg, "  lea %d(%%rbp), %%rax", node->return_buffer->offset);
        asm_push(cg);
    }
    return stack;
}

static i32 asm_pop_args(ASMCodegenData_T* cg, ASTNode_T* node)
{
    i32 gp = 0, fp = 0;

    if(node->data_type->kind != TY_VOID && node->data_type->size > 16)
        asm_pop(cg, argreg64[gp++]);
    for(u64 i = 0; i < node->args->size; i++)
    {
        ASTNode_T* arg = node->args->items[i];
        ASTType_T* ty = unpack(arg->data_type);
        switch(ty->kind)
        {
            case TY_STRUCT:
                if(ty->size > 16)
                    continue;
                
                bool fp1 = asm_has_flonum_1(ty);
                bool fp2 = asm_has_flonum_2(ty);
                if(fp + fp1 + fp2 < ASM_FP_MAX && gp + !fp1 + !fp2 < ASM_GP_MAX)
                {
                    if(fp1)
                        asm_popf(cg, fp++);
                    else
                        asm_pop(cg, argreg64[gp++]);
                    
                    if(ty->size > 8) 
                    {
                        if(fp2)
                            asm_popf(cg, fp++);
                        else
                            asm_pop(cg, argreg64[gp++]);    
                    }
                }
                break;
            case TY_F32:
            case TY_F64:
                if(fp < ASM_FP_MAX)
                    asm_popf(cg, fp++);
                break;
            case TY_F80:
                break;
            default:
                if(gp < ASM_GP_MAX)
                    asm_pop(cg, argreg64[gp++]);
        }
    }

    return fp;
}

static void asm_gen_inc(ASMCodegenData_T* cg, ASTNode_T* node)
{
    // convert x++ to (x = x + 1) - 1
    //         x-- to (x = x + -1) - -1
    ASTNode_T addend = {
        .kind = ND_INT,
        .int_val = node->kind == ND_INC ? 1 : -1,
        .data_type = (ASTType_T*) primitives[TY_I32]
    };

    ASTNode_T converted = {
        .kind = ND_SUB,
        .data_type = node->data_type,
        .right = &addend,
        .left = &(ASTNode_T) {
            .kind = ND_ASSIGN,
            .data_type = node->data_type,
            .left = node->left,
            .right = &(ASTNode_T) {
                .kind = ND_ADD,
                .left = node->left,
                .right = &addend,
                .data_type = node->left->data_type
            } 
        }
    };

    asm_gen_expr(cg, &converted);
}

static void asm_gen_id_ptr(ASMCodegenData_T* cg, ASTNode_T* id)
{
    switch(id->referenced_obj->kind)
    {
        case OBJ_FN_ARG:
        case OBJ_LOCAL:
            asm_print(cg, "%d(%%rbp)", id->referenced_obj->offset);
            break;
        case OBJ_GLOBAL:
        case OBJ_ENUM_MEMBER:
            asm_print(cg, "%s(%%rip)", asm_gen_identifier(id->id));
            break;
        case OBJ_FUNCTION:
            if(id->referenced_obj->is_extern_c)
                asm_print(cg, "%s" CSPC_ASM_EXTERN_FN_POSTFIX "(%%rip)", either(id->referenced_obj->exported, id->id->callee));
            else if(id->referenced_obj->is_extern)
                asm_print(cg, "%s" CSPC_ASM_EXTERN_FN_POSTFIX "(%%rip)", either(id->referenced_obj->exported, asm_gen_identifier(id->id)));
            else
                asm_print(cg, "%s(%%rip)", asm_gen_identifier(id->id));
            break;
        default:
            unreachable();
    }
}

static void asm_gen_struct_lit(ASMCodegenData_T* cg, ASTNode_T* node)
{
    for(size_t i = 0; i < node->args->size; i++)
    {
        ASTNode_T* arg = node->args->items[i];
        asm_push(cg);
        asm_gen_expr(cg, arg);
        asm_store(cg, arg->data_type);
        asm_println(cg, "  mov %%rdi, %%rax");
        asm_println(cg, "  add $%d, %%rax", arg->data_type->size);
    }
}

static void asm_gen_inline_strlen(ASMCodegenData_T* cg)
{
    asm_println(cg, "  mov %%rax, %%rdi");
    asm_println(cg, "  mov $-1, %%rcx");
    asm_println(cg, "  xor %%eax, %%eax");
    asm_println(cg, "  cld");
    asm_println(cg, "  repne scasb");
    asm_println(cg, "  xor $-1, %%rcx");
    asm_println(cg, "  dec %%rcx");
    asm_println(cg, "  mov %%rcx, %%rax");
}

static void asm_gen_expr(ASMCodegenData_T* cg, ASTNode_T* node)
{
    if(node->tok && cg->embed_file_locations)
        asm_println(cg, "  .loc %d %d", node->tok->source->file_no + 1, node->tok->line);
    switch(node->kind)
    {
        case ND_NOOP:
            return;
        case ND_CLOSURE:
            for(size_t i = 0; i < node->exprs->size; i++)
                asm_gen_expr(cg, node->exprs->items[i]);
            return;
        case ND_FLOAT:
        {
            union { f32 f32; u32 u32; } u = { node->float_val };
            asm_println(cg, "  mov $%u, %%eax  # float %f", u.u32, node->float_val);
            asm_println(cg, "  movq %%rax, %%xmm0");
        } return;
        case ND_DOUBLE:
        {
            union { f64 f64; u64 u64; } u = { node->double_val };
            asm_println(cg, "  mov $%lu, %%rax  # float %f", u.u64, node->double_val);
            asm_println(cg, "  movq %%rax, %%xmm0");
        } return;
        case ND_INT:
            if(node->int_val)
                asm_println(cg, "  mov $%d, %%rax", node->int_val);
            else
                asm_println(cg, "  xor %%rax, %%rax");
            return;
        case ND_BOOL:   
            if(node->bool_val)
                asm_println(cg, "  mov $%d, %%rax", node->bool_val);
            else
                asm_println(cg, "  xor %%rax, %%rax");
            return;
        case ND_LONG:
            if(node->long_val)
                asm_println(cg, "  mov $%ld, %%rax", node->long_val);
            else
                asm_println(cg, "  xor %%rax, %%rax");
            return;
        case ND_ULONG:
            if(node->ulong_val)
                asm_println(cg, "  mov $%lu, %%rax", node->ulong_val);
            else
                asm_println(cg, "  xor %%rax, %%rax");
            return;
        case ND_CHAR:
            if(node->int_val)
                asm_println(cg, "  mov $%d, %%rax", node->int_val);
            else
                asm_println(cg, "  xor %%rax, %%rax");
            return;
        case ND_NIL:
            asm_println(cg, "  xor %%rax, %%rax");
            return;
        case ND_STR:
            asm_println(cg, "  lea .L.string.%lu, %%rax", cg->string_literals->size);
            list_push(cg->string_literals, node->str_val);
            return;
        
        case ND_SIZEOF:
            if(node->the_type->size)
                asm_println(cg, "  mov $%d, %%rax", node->the_type->size);
            else
                asm_println(cg, "  xor %%rax, %%rax");
            return;
        
        case ND_ALIGNOF:
            if(node->the_type->align)
                asm_println(cg, "  mov $%d, %%rax", node->the_type->align);
            else
                asm_println(cg, "  xor %%rax, %%rax");
            return;
        
        case ND_LEN:
            {
                ASTType_T* ty = unpack(node->expr->data_type);

                switch(ty->kind)
                {
                    case TY_C_ARRAY:
                        asm_println(cg, "  mov $%lu, %%rax", ty->num_indices);
                        break;
                    
                    case TY_ARRAY:
                    case TY_VLA:
                        asm_gen_addr(cg, node->expr);
                        asm_println(cg, "  mov (%%rax), %%rax");
                        break;
                    
                    case TY_PTR:
                        if(unpack(ty->base)->kind == TY_CHAR)
                        {
                            asm_gen_expr(cg, node->expr);
                            asm_gen_inline_strlen(cg);
                            break;
                        }
                        // fall through

                    default:
                        throw_error(ERR_CODEGEN, node->tok, "`len` operator not implemented for this data type");
                }
            } return;
        
        case ND_PIPE:
            asm_gen_expr(cg, node->left);
            asm_println(cg, "  mov %%rax, %s", pipe_reg);
            asm_gen_expr(cg, node->right);
            return;
        
        case ND_HOLE:
            asm_println(cg, "  mov %s, %%rax", pipe_reg);
            return;

        case ND_NEG:
            asm_gen_expr(cg, node->right);
            switch(node->data_type->kind)
            {
                case TY_F32:
                    asm_println(cg, "  mov $1, %%rax");
                    asm_println(cg, "  shl $31, %%rax");
                    asm_println(cg, "  movq %%rax, %%xmm1");
                    asm_println(cg, "  xorps %%xmm1, %%xmm0");
                    return;
                case TY_F64:
                    asm_println(cg, "  mov $1, %%rax");
                    asm_println(cg, "  shl $63, %%rax");
                    asm_println(cg, "  movq %%rax, %%xmm1");
                    asm_println(cg, "  xorpd %%xmm1, %%xmm0");
                    return;
                case TY_F80:
                    asm_println(cg, "  fchs");
                    return;
                default:
                    asm_println(cg, "  neg %%rax");
                    return;
            }
            return;
        
        case ND_INDEX:
            asm_gen_index(cg, node, false);
            return;
        
        case ND_INC:
        case ND_DEC:
            asm_gen_inc(cg, node);
            return;

        case ND_ADD:
            if(unpack(node->left->data_type)->base && unpack(node->right->data_type)->base)
                throw_error(ERR_SYNTAX_ERROR, node->tok, "cannot add two pointer types together");
            if(ptr_type(node->left->data_type) && unpack(node->left->data_type)->base->size > 1)
            {
                // if we add a number to a pointer, multiply the second argument with the base type size
                // a + b -> a + b * sizeof *a
                ASTNode_T new_right = {
                    .kind = ND_MUL,
                    .data_type = node->right->data_type,
                    .left = node->right,
                    .right = &(ASTNode_T) {
                        .kind = ND_LONG,
                        .data_type = (ASTType_T*) primitives[TY_U64],
                        .long_val = unpack(node->left->data_type)->base->size
                    }
                };

                node->right = &new_right;
            }
            break;
        
        case ND_SUB:
            if(ptr_type(node->left->data_type) && is_integer(node->right->data_type))
            {
                // if we subtract a number from a pointer, multiply the second argument with the base type size
                // a - b -> a - b * sizeof *a

                node->bool_val = true;

                ASTNode_T new_right = {
                    .kind = ND_MUL,
                    .data_type = node->right->data_type,
                    .left = node->right,
                    .right = &(ASTNode_T) {
                        .kind = ND_LONG,
                        .data_type = (ASTType_T*) primitives[TY_U64],
                        .long_val = unpack(node->left->data_type)->size
                    }
                };

                node->right = &new_right;
            }
            else if(ptr_type(node->left->data_type) && ptr_type(node->left->data_type) && !node->bool_val)
            {
                // if both subtraction arguments are pointers, return the number of elements in between
                // a - b -> (a - b) / sizeof *a

                node->bool_val = true;

                ASTNode_T converted = {
                    .kind = ND_DIV,
                    .data_type = (ASTType_T*) primitives[TY_I64],
                    .left = node,
                    .right = &(ASTNode_T) {
                        .kind = ND_LONG,
                        .data_type = (ASTType_T*) primitives[TY_I64],
                        .int_val = unpack(node->left->data_type)->base->size
                    }
                };

                asm_gen_expr(cg, &converted);
                return;
            }
            break;
        
        case ND_ID:
            asm_gen_addr(cg, node);
            asm_load(cg, node->data_type);

            if(unpack(node->data_type)->kind == TY_FN && 
                node->referenced_obj && 
                (node->referenced_obj->kind == OBJ_LOCAL || node->referenced_obj->kind == OBJ_FN_ARG)
            )
                    asm_println(cg, "  mov (%%rax), %%rax");
            return;

        case ND_MEMBER:
            asm_gen_addr(cg, node);
            asm_load(cg, node->data_type);

            if(unpack(node->data_type)->kind == TY_FN)
                asm_println(cg, "  mov (%%rax), %%rax");

            return;
        
        case ND_ARRAY:
        {
            size_t current = 0;
            for(size_t i = 0; i < node->args->size; i++)
            {
                ASTNode_T* arg = node->args->items[i];
                if(arg->unpack_mode) {
                    ASTType_T* arg_type = unpack(arg->data_type);
                    size_t num_indices = arg_type->num_indices;
                    for(size_t j = 0; j < num_indices; j++) {
                        size_t index = arg->unpack_mode == UMODE_FTOB ? j : num_indices - j - 1;
                        asm_println(cg, "  lea %ld(%%rbp), %%rax", node->buffer->offset + 8 + (current++ * node->data_type->base->size));
                        asm_push(cg);

                        asm_gen_expr(cg, &(ASTNode_T){
                            .kind = ND_INDEX,
                            .left = arg,
                            .expr = &(ASTNode_T) {
                                .kind = ND_LONG,
                                .long_val = index,
                                .data_type = (ASTType_T*) primitives[TY_U64]
                            },
                            .data_type = arg_type->base
                        });

                        asm_store(cg, node->data_type->base);
                    }
                }
                else {
                    asm_println(cg, "  lea %ld(%%rbp), %%rax", node->buffer->offset + 8 + (current++ * node->data_type->base->size));
                    asm_push(cg);
                    asm_gen_expr(cg, arg);
                    asm_store(cg, node->data_type->base);
                }
            }
            
            // add the size of the array at the beginning
            asm_println(cg, "  movq $%lu, %d(%%rbp)", node->data_type->num_indices, node->buffer->offset);
            asm_println(cg, "  lea %d(%%rbp), %%rax", node->buffer->offset);
            return;
        }
        
        case ND_STRUCT:
            asm_println(cg, "  lea %d(%%rbp), %%rax", node->buffer->offset);
            asm_push(cg);
            asm_gen_struct_lit(cg, node);
            asm_pop(cg, "%rax");
            return;

        case ND_REF:
            asm_gen_addr(cg, node->right);
            return;
        
        case ND_DEREF:
            asm_gen_expr(cg, node->right);
            asm_load(cg, node->data_type);
            return;
        
        case ND_ASSIGN:
            switch(node->right->kind)
            {
            case ND_ARRAY:
                asm_gen_addr(cg, node->left);
                asm_push(cg);
                asm_println(cg, "  add $8, %%rax");
                for(size_t i = 0; i < node->right->args->size; i++)
                {
                    ASTNode_T* arg = node->right->args->items[i];
                    if(arg->unpack_mode)
                    {
                        size_t num_indices = unpack(arg->data_type)->num_indices;
                        for(size_t j = 0; j < num_indices; j++)
                        {
                            size_t index = arg->unpack_mode == UMODE_FTOB ? j : num_indices - j - 1;
                            asm_push(cg);
                            asm_gen_expr(cg, &(ASTNode_T){
                                .kind = ND_INDEX,
                                .left = arg,
                                .expr = &(ASTNode_T){
                                    .kind = ND_LONG,
                                    .long_val = index,
                                    .data_type = (ASTType_T*) primitives[TY_U64]
                                },
                                .data_type = unpack(arg->data_type)->base
                            });
                            asm_store(cg, node->right->data_type->base);
                            asm_println(cg, "  mov %%rdi, %%rax");
                            asm_println(cg, "  add $%d, %%rax", node->right->data_type->base->size);
                        }
                    }
                    else {
                        asm_push(cg);
                        asm_gen_expr(cg, arg);
                        asm_store(cg, node->right->data_type->base);
                        asm_println(cg, "  mov %%rdi, %%rax");
                        asm_println(cg, "  add $%d, %%rax", node->right->data_type->base->size);
                    }
                }
                
                asm_pop(cg, "%rax");
                return;
            case ND_STRUCT:
                asm_gen_addr(cg, node->left);
                asm_push(cg);
                asm_gen_struct_lit(cg, node->right);
                asm_pop(cg, "%rax");
                return;
            default:
                asm_gen_addr(cg, node->left);
                asm_push(cg);
                asm_gen_expr(cg, node->right);

                asm_store(cg, node->left->data_type);
                return;
            }

        case ND_LAMBDA:
            {
                asm_println(cg, "  mov %%rbp, .lambda.stackptr.%ld", node->long_val);
                asm_println(cg, "  lea lambda.%ld(%%rip), %%rax", node->long_val);

                ASTObj_T* impl = init_ast_obj(OBJ_LAMBDA, node->tok);
                impl->body = node;
                list_push(cg->ast->objs, impl);
            } return;

        case ND_CAST:
            asm_gen_expr(cg, node->left);
            asm_cast(cg, node->left->data_type, node->data_type);
            return;
        
        case ND_NOT:
            asm_gen_expr(cg, node->right);
            asm_cmp_zero(cg, node->right->data_type);
            asm_println(cg, "  sete %%al");
            asm_println(cg, "  movzx %%al, %%rax");
            return;
        
        case ND_BIT_NEG:
            asm_gen_expr(cg, node->right);
            asm_println(cg, "  not %%rax");
            return;
        
        case ND_AND:
        {
            u64 pc = cg->cur_count;
            u64 c = cg->cur_count = cg->max_count++;
            asm_gen_expr(cg, node->left);
            asm_cmp_zero(cg, node->left->data_type);
            asm_println(cg, "  je .L.false.%ld", c);
            asm_gen_expr(cg, node->right);
            asm_cmp_zero(cg, node->right->data_type);
            asm_println(cg, "  je .L.false.%ld", c);
            asm_println(cg, "  mov $1, %%rax");
            asm_println(cg, "  jmp .L.end.%ld", c);
            asm_println(cg, ".L.false.%ld:", c);
            asm_println(cg, "  xor %%rax, %%rax");
            asm_println(cg, ".L.end.%ld:", c);
            cg->cur_count = pc;
        } return;

        case ND_OR:
        {
            u64 pc = cg->cur_count;
            u64 c = cg->cur_count = cg->max_count++;
            asm_gen_expr(cg, node->left);
            asm_cmp_zero(cg, node->left->data_type);
            asm_println(cg, "  jne .L.true.%ld", c);
            asm_gen_expr(cg, node->right);
            asm_cmp_zero(cg, node->right->data_type);
            asm_println(cg, "  jne .L.true.%ld", c);
            asm_println(cg, "  xor %%rax, %%rax");
            asm_println(cg, "  jmp .L.end.%ld", c);
            asm_println(cg, ".L.true.%ld:", c);
            asm_println(cg, "  mov $1, %%rax");
            asm_println(cg, ".L.end.%ld:", c);
            cg->cur_count = pc;
        } return;

        case ND_TERNARY:
        {
            u64 pc = cg->cur_count;
            u64 c = cg->cur_count = cg->max_count++;
            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  je .L.else.%ld", c);
            asm_gen_expr(cg, node->if_branch);
            asm_println(cg, "  jmp .L.end.%ld", c);
            asm_println(cg, ".L.else.%ld:", c);
            asm_gen_expr(cg, node->else_branch);
            asm_println(cg, ".L.end.%ld:", c);
            cg->cur_count = pc;
        } return;
    
        case ND_ELSE_EXPR:
        {
            u64 pc = cg->cur_count;
            u64 c = cg->cur_count = cg->max_count++;
            asm_gen_expr(cg, node->left);
            asm_push(cg);
            asm_push(cg);
            asm_cmp_zero(cg, node->left->data_type);
            asm_println(cg, "  je .L.else.%ld", c);
            asm_pop(cg, "%rax");
            asm_println(cg, "  jmp .L.end.%ld", c);
            asm_println(cg, ".L.else.%ld:", c);
            asm_pop(cg, "%rax");
            asm_gen_expr(cg, node->right);
            asm_println(cg, ".L.end.%ld:", c);
            cg->cur_count = pc;
        } return;

        case ND_CALL:
        {
            List_T* packed_args = node->args;
            node->args = unpack_call_args(node);
            i32 stack_args = asm_push_args(cg, node);
            asm_gen_addr(cg, node->expr);

            if(!node->called_obj || node->called_obj->kind == OBJ_LOCAL || node->called_obj->kind == OBJ_FN_ARG)
                asm_println(cg, "  mov (%%rax), %%rax");

            i32 fp = asm_pop_args(cg, node);

            asm_println(cg, "  mov %%rax, %s", call_reg);
            asm_println(cg, "  mov $%d, %%rax", fp);
            asm_println(cg, "  call *%s", call_reg);
            asm_println(cg, "  add $%d, %%rsp", stack_args * 8);

            cg->depth -= stack_args;

            // It looks like the most significant 48 or 56 bits in RAX may
		    // contain garbage if a function return type is short or bool/char,
		    // respectively. We clear the upper bits here.
            switch(node->data_type->kind)
            {
                case TY_BOOL:
                    asm_println(cg, "  movzx %%al, %%eax");
                    return;
                case TY_CHAR:
                case TY_I8:
                    asm_println(cg, "  movsbl %%al, %%eax");
                    return;
                case TY_U8:
                    asm_println(cg, "  movzbl %%al, %%eax");
                    return;
                case TY_I16:
                    asm_println(cg, "  movswl %%ax, %%eax");
                    return;
                case TY_U16:
                    asm_println(cg, "  movzwl %%ax, %%eax");
                    return;
                default:
                    break;
            }

            // If the return type is a small struct, a value is returned
		    // using up to two registers.
            if(node->return_buffer && node->data_type->size <= 16) {
                asm_copy_ret_buffer(cg, node->return_buffer);
                asm_println(cg, "  lea %d(%%rbp), %%rax", node->return_buffer->offset);
            }

            if(node->args != packed_args) {
                free_list(node->args);
                node->args = packed_args;
            }
        } return;
        
        default:
            break;
    }

    switch(unpack(node->left->data_type)->kind)
    {
        case TY_F32:
        case TY_F64:
        {
            if(node->kind == ND_GT || node->kind == ND_GE)
            {
                asm_gen_expr(cg, node->left);
                asm_pushf(cg);
                asm_gen_expr(cg, node->right);
                asm_popf(cg, 1);
            }
            else {
                asm_gen_expr(cg, node->right);
                asm_pushf(cg);
                asm_gen_expr(cg, node->left);
                asm_popf(cg, 1);
            }

            char* sz = node->left->data_type->kind == TY_F32 ? "ss" : "sd";

            switch(node->kind)
            {
                case ND_ADD:
                    asm_println(cg, "  add%s %%xmm1, %%xmm0", sz);
                    return;
                case ND_SUB:
                    asm_println(cg, "  sub%s %%xmm1, %%xmm0", sz);
                    return;
                case ND_MUL:
                    asm_println(cg, "  mul%s %%xmm1, %%xmm0", sz);
                    return;
                case ND_DIV:
                    asm_println(cg, "  div%s %%xmm1, %%xmm0", sz);
                    return;
                case ND_EQ:
                    asm_println(cg, "  ucomi%s %%xmm0, %%xmm1", sz);
                    asm_println(cg, "  sete %%al");
                    asm_println(cg, "  setnp %%dl");
                    asm_println(cg, "  and %%dl, %%al");
                    asm_println(cg, "  and $1, %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                case ND_NE:
                    asm_println(cg, "  ucomi%s %%xmm0, %%xmm1", sz);
                    asm_println(cg, "  setne %%al");
                    asm_println(cg, "  setnp %%dl");
                    asm_println(cg, "  and %%dl, %%al");
                    asm_println(cg, "  and $1, %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                case ND_LT:
                case ND_GT:
                    asm_println(cg, "  ucomi%s %%xmm0, %%xmm1", sz);
                    asm_println(cg, "  seta %%al");
                    asm_println(cg, "  and $1, %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                case ND_LE:
                case ND_GE:
                    asm_println(cg, "  ucomi%s %%xmm0, %%xmm1", sz);
                    asm_println(cg, "  setae %%al");
                    asm_println(cg, "  and $1, %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                default:
                    break;
            }
            LOG_ERROR_F("unhandled expression (%d)\n", node->kind);
            return;
        }
        case TY_F80:
            if(node->kind == ND_GT || node->kind == ND_GE)
            {
                asm_gen_expr(cg, node->right);
                asm_gen_expr(cg, node->left);
            }
            else
            {
                asm_gen_expr(cg, node->left);
                asm_gen_expr(cg, node->right);
            }

            switch (node->kind) 
            {
                case ND_ADD:
                    asm_println(cg, "  faddp");
                    return;
                case ND_SUB:
                    asm_println(cg, "  fsubrp");
                    return;
                case ND_MUL:
                    asm_println(cg, "  fmulp");
                    return;
                case ND_DIV:
                    asm_println(cg, "  fdivrp");
                    return;
                case ND_EQ:
                    asm_println(cg, "  fcomip");
                    asm_println(cg, "  fstp %%st(0)");
                    asm_println(cg, "  sete %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                case ND_NE:
                    asm_println(cg, "  fcomip");
                    asm_println(cg, "  fstp %%st(0)");
                    asm_println(cg, "  setne %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                case ND_LT:
                case ND_GT:
                    asm_println(cg, "  fcomip");
                    asm_println(cg, "  fstp %%st(0)");
                    asm_println(cg, "  seta %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                case ND_LE:
                case ND_GE:
                    asm_println(cg, "  fcomip");
                    asm_println(cg, "  fstp %%st(0)");
                    asm_println(cg, "  setae %%al");
                    asm_println(cg, "  movzb %%al, %%rax");
                    return;
                default:
                    break;
            }

            LOG_ERROR_F("unhandled expression (%d)\n", node->kind);
            return;

        default:
            break;
    }

    asm_gen_expr(cg, node->kind == ND_GE || node->kind == ND_GT ? node->left : node->right);
    asm_push(cg);
    asm_gen_expr(cg, node->kind == ND_GE || node->kind == ND_GT ? node->right : node->left);
    asm_pop(cg, "%rdi");

    char* ax, * di, * dx;

    if(node->left->data_type->kind == TY_I64 || node->left->data_type->kind == TY_U64 || node->left->data_type->base)
    {
        ax = "%rax";
        di = "%rdi";
        dx = "%rdx";
    }
    else
    {
        ax = "%eax";
        di = "%edi";
        dx = "%edx";
    }

    switch(node->kind)
    {
        case ND_ADD:
            asm_println(cg, "  add %s, %s", di, ax);
            return;
        case ND_SUB:
            asm_println(cg, "  sub %s, %s", di, ax);
            return;
        case ND_MUL:
            asm_println(cg, "  imul %s, %s", di, ax);
            return;
        case ND_DIV:
        case ND_MOD:
            if(unsigned_type(node->data_type))
            {
                asm_println(cg, "  xor %s, %s", dx, dx);
                asm_println(cg, "  div %s", di);
            }
            else
            {
                if(node->left->data_type->size == 8)
                    asm_println(cg, "  cqo");
                else
                    asm_println(cg, "  cdq");
                asm_println(cg, "  idiv %s", di);
            }

            if(node->kind == ND_MOD)
                asm_println(cg, "  mov %%rdx, %%rax");
            return;
        case ND_BIT_AND:
            asm_println(cg, "  and %s, %s", di, ax);
            return;
        case ND_BIT_OR:
            asm_println(cg, "  or %s, %s", di, ax);
            return;
        case ND_XOR:
            asm_println(cg, "  xor %s, %s", di, ax);
            return;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
        case ND_GT:
        case ND_GE:
            asm_println(cg, "  cmp %s, %s", di, ax);

            if(node->kind == ND_EQ)
                asm_println(cg, "  sete %%al");
            else if(node->kind == ND_NE)
                asm_println(cg, "  setne %%al");
            else if(node->kind == ND_LT || node->kind == ND_GT)
                if(unsigned_type(node->left->data_type))
                    asm_println(cg, "  setb %%al");
                else
                    asm_println(cg, "  setl %%al");
            else if(node->kind == ND_LE || node->kind == ND_GE)
                if(unsigned_type(node->left->data_type))
                    asm_println(cg, "  setbe %%al");
                else
                    asm_println(cg, "  setle %%al");
            else {}
            
            asm_println(cg, "  movzb %%al, %%rax");
            return;
        case ND_LSHIFT:
            asm_println(cg, "  mov %%rdi, %%rcx");
            asm_println(cg, "  shl %%cl, %s", ax);
            return;
        case ND_RSHIFT:
            asm_println(cg, "  mov %%rdi, %%rcx");
            if(unsigned_type(node->left->data_type))
                asm_println(cg, "  shr %%cl, %s", ax);
            else
                asm_println(cg, "  sar %%cl, %s", ax);
            return;
        default: 
            LOG_ERROR_F("unhandled expression (%d)", node->kind);
    }
}

static void asm_init_zero(ASMCodegenData_T* cg, ASTObj_T* var)
{
    asm_println(cg, "  mov $%d, %%rcx", var->data_type->size);
    asm_println(cg, "  lea %d(%%rbp), %%rdi", var->offset);
    asm_println(cg, "  xor %%al, %%al");
    asm_println(cg, "  rep stosb");
}

static void asm_gen_stmt(ASMCodegenData_T* cg, ASTNode_T* node)
{
    if(node->tok && cg->embed_file_locations)
        asm_println(cg, "  .loc %d %d", node->tok->source->file_no + 1, node->tok->line);

    switch(node->kind)
    {
        case ND_NOOP:
            return;
        
        case ND_ASM:
        asm_print(cg, "  ");
        for(size_t i = 0; i < node->args->size; i++)
        {
            ASTNode_T* arg = node->args->items[i];
            switch(arg->kind)
            {
                case ND_STR:
                    asm_print(cg, "%s",arg->str_val);
                    break;
                case ND_INT:
                    asm_print(cg, "$%d", arg->int_val);
                    break;
                case ND_LONG:
                    asm_print(cg, "$%ld", arg->long_val);
                    break;
                case ND_ULONG:
                    asm_print(cg, "$%lu", arg->ulong_val);
                    break;
                case ND_ID:
                    asm_gen_id_ptr(cg, arg);
                    break;
                default:
                    unreachable();
            }
        }
        asm_print(cg, "\n");
        return;

        case ND_IF:
        {
            u64 pc = cg->cur_count;
            u64 c = cg->cur_count = cg->max_count++;
            
            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  je  .L.else.%lu", c);
            asm_gen_stmt(cg, node->if_branch);
            asm_println(cg, "  jmp .L.end.%lu", c);
            asm_println(cg, ".L.else.%lu:", c);
            if(node->else_branch)
                asm_gen_stmt(cg, node->else_branch);
            asm_println(cg, ".L.end.%lu:", c);

            cg->cur_count = pc;
        } return;

        case ND_WITH:
        {
            u64 pc = cg->cur_count;
            u64 c = cg->cur_count = cg->max_count++;
            ASTNode_T* id = node->condition->left;

            asm_gen_expr(cg, node->condition);
            asm_gen_expr(cg, id);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  je .L.else.%lu", c);
            asm_gen_stmt(cg, node->if_branch);

            // generate the exit function
            asm_gen_expr(cg, &(ASTNode_T) {
                .kind = ND_CALL,
                .data_type = (ASTType_T*) primitives[TY_VOID],
                .referenced_obj = node->exit_fn,
                .expr = &(ASTNode_T) {
                    .kind = ND_ID,
                    .referenced_obj = node->exit_fn,
                    .id = node->exit_fn->id
                },
                .args = &(List_T) {
                    .size = 1,
                    .items = (void**) &id
                }
            });
            
            asm_println(cg, "  jmp .L.end.%lu", c);
            asm_println(cg, ".L.else.%lu:", c);
            if(node->else_branch)
                asm_gen_stmt(cg, node->else_branch);
            asm_println(cg, ".L.end.%lu:", c);

            cg->cur_count = pc;
        } return;

        case ND_FOR:
        {
            u64 pc = cg->cur_count;
            u64 pbrk = cg->cur_brk_id;
            u64 pcnt = cg->cur_cnt_id;
            u64 c = cg->cur_brk_id 
                  = cg->cur_cnt_id
                  = cg->cur_count 
                  = cg->max_count++;
            
            if(node->init_stmt)
                asm_gen_stmt(cg, node->init_stmt);
            asm_println(cg, ".L.begin.%lu:", c);
            if(node->condition) {
                asm_gen_expr(cg, node->condition);
                asm_cmp_zero(cg, node->condition->data_type);
                asm_println(cg, "  je .L.break.%lu", c);
            }
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.continue.%lu:", c);
            if(node->expr)
                asm_gen_expr(cg, node->expr);
            asm_println(cg, "  jmp .L.begin.%lu", c);
            asm_println(cg, ".L.break.%lu:", c);

            cg->cur_count = pc;
            cg->cur_brk_id = pbrk;
            cg->cur_cnt_id = pcnt;
        } return;
    
        case ND_FOR_RANGE:
        {
            u64 pc = cg->cur_count;
            u64 pbrk = cg->cur_brk_id;
            u64 pcnt = cg->cur_cnt_id;
            u64 c = cg->cur_brk_id 
                  = cg->cur_count 
                  = cg->cur_cnt_id
                  = cg->max_count++;

            asm_gen_expr(cg, node->left);
            asm_push(cg);
            asm_gen_expr(cg, node->right);
            asm_println(cg, "  mov %%rax, %%rdx");
            asm_pop(cg, "%rdi");
            
            asm_println(cg, ".L.begin.%lu:", c);
            asm_println(cg, "  cmp %%rdx, %%rdi");
            asm_println(cg, "  jge .L.break.%lu", c);

            asm_println(cg, "  push %%rdi");
            asm_println(cg, "  push %%rdx");
            asm_gen_stmt(cg, node->body);
            asm_println(cg, "  pop %%rdx");
            asm_println(cg, "  pop %%rdi");
            
            asm_println(cg, ".L.continue.%lu:", c);
            asm_println(cg, "  inc %%rdi");
            asm_println(cg, "  jmp .L.begin.%lu", c);
            asm_println(cg, ".L.break.%lu:", c);

            cg->cur_count = pc;
            cg->cur_brk_id = pbrk;
            cg->cur_cnt_id = pcnt;        
        } return;

        case ND_WHILE:
        {
            u64 pc = cg->cur_count;
            u64 pbrk = cg->cur_brk_id;
            u64 pcnt = cg->cur_cnt_id; 
            u64 c = cg->cur_brk_id 
                  = cg->cur_count 
                  = cg->cur_cnt_id
                  = cg->max_count++;

            asm_println(cg, ".L.begin.%lu:", c);
            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  je .L.break.%lu", c);
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.continue.%lu:", c);
            asm_println(cg, "  jmp .L.begin.%lu", c);
            asm_println(cg, ".L.break.%lu:", c);

            cg->cur_count = pc;
            cg->cur_brk_id = pbrk;
            cg->cur_cnt_id = pcnt;
        } return;

        case ND_DO_UNLESS:
        {
            u64 pc = cg->cur_count;
            u64 c = cg->cur_count = cg->max_count++;

            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  jne .L.skip_unless.%lu", c);
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.skip_unless.%lu:", c);

            cg->cur_count = pc;
        } return;

        case ND_DO_WHILE:
        {
            u64 pc = cg->cur_count;
            u64 pbrk = cg->cur_brk_id;
            u64 pcnt = cg->cur_cnt_id; 
            u64 c = cg->cur_brk_id 
                  = cg->cur_count 
                  = cg->cur_cnt_id
                  = cg->max_count++;  

            asm_println(cg, ".L.begin.%lu:", c);
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.continue.%lu:", c);
            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  jne .L.begin.%lu", c);
            asm_println(cg, ".L.break.%lu:", c);

            cg->cur_count = pc;
            cg->cur_brk_id = pbrk;
            cg->cur_cnt_id = pcnt; 
        } return;   

        case ND_LOOP:
        {
            u64 pc = cg->cur_count;
            u64 pbrk = cg->cur_brk_id;
            u64 pcnt = cg->cur_cnt_id; 
            u64 c = cg->cur_brk_id 
                  = cg->cur_count 
                  = cg->cur_cnt_id
                  = cg->max_count++;
            
            asm_println(cg, ".L.begin.%lu:", c);
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.continue.%lu:", c);
            asm_println(cg, "  jmp .L.begin.%lu", c);
            asm_println(cg, ".L.break.%lu:", c);

            cg->cur_count = pc;
            cg->cur_brk_id = pbrk;
            cg->cur_cnt_id = pcnt;
        } return;
    
        case ND_MATCH:
        {
            asm_gen_expr(cg, node->condition);
            u64 pc = cg->cur_count;
            u64 pbrk = cg->cur_brk_id;
            u64 c = cg->cur_count = cg->cur_brk_id = cg->max_count++;
            char* ax = (node->condition->data_type->size == 8) ? "%rax" : "%eax";
            char* di = (node->condition->data_type->size == 8) ? "%rdi" : "%edi";
            
            for(size_t i = 0; i < node->cases->size; i++)
            {
                ASTNode_T* _case = node->cases->items[i];
                _case->long_val = i;
                asm_push(cg);
                asm_push(cg);
                asm_gen_expr(cg, _case->condition);
                asm_pop(cg, "%rdi");
                asm_println(cg, "  cmp %s, %s", ax, di);
                asm_pop(cg, "%rax");

                asm_println(cg, "  %s .L.case.%ld.%lu", jmp_mode[_case->mode], i, c);
            }
            
            if(node->default_case) 
            {
                node->default_case->long_val = node->cases->size;
                asm_println(cg, "  jmp .L.case.%lu.%lu", node->cases->size, c);
            }
            
            for(size_t i = 0; i < node->cases->size; i++)
                asm_gen_stmt(cg, node->cases->items[i]);
            if(node->default_case)
                asm_gen_stmt(cg, node->default_case);
            
            asm_println(cg, ".L.break.%lu:", c);

            cg->cur_count = pc;
            cg->cur_brk_id = pbrk;
        } return;
        
        case ND_CASE:
            asm_println(cg, ".L.case.%ld.%lu:", node->long_val, cg->cur_count);
            asm_gen_stmt(cg, node->body);
            asm_println(cg, "  jmp .L.break.%lu", cg->cur_count);
            return;
        
        case ND_BLOCK:
            for(size_t i = 0; i < node->locals->size; i++) {
                ASTObj_T* local = node->locals->items[i];
                asm_init_zero(cg, local);

                // load all of the known array sizes into the first 8 bytes of the local variable
                if(unpack(local->data_type)->kind == TY_ARRAY)
                    asm_println(cg, "  movq $%lu, %d(%%rbp)", local->data_type->num_indices, local->offset);
            }

            for(size_t i = 0; i < node->stmts->size; i++)
                asm_gen_stmt(cg, node->stmts->items[i]);
            return;
        
        case ND_RETURN:
            if(node->return_val)
            {
                asm_gen_expr(cg, node->return_val);
                ASTType_T* ty = unpack(node->return_val->data_type);

                if(ty->kind == TY_STRUCT)
                {
                    if(ty->size <= 16)
                        asm_copy_struct_reg(cg);
                    else
                        asm_copy_struct_mem(cg, node->return_val);
                }
            }

            asm_println(cg, "  jmp .L.return.%s", cg->current_fn_name);
            return;
        
        case ND_EXPR_STMT:
            asm_gen_expr(cg, node->expr);
            return;
        
        case ND_CONTINUE:
            asm_println(cg, "  jmp .L.continue.%lu", cg->cur_cnt_id);
            return;

        case ND_BREAK:
            asm_println(cg, "  jmp .L.break.%lu", cg->cur_brk_id);
            return;
        
        case ND_MATCH_TYPE:
            if(node->body)
                asm_gen_stmt(cg, node->body);
            return;
        
        case ND_DEFER:
            if(!cg->current_fn->deferred) {
                cg->current_fn->deferred = init_list(); 
                mem_add_list(cg->current_fn->deferred);
            }
            list_push(cg->current_fn->deferred, node->body);
            return;
        case ND_USING:
            if(node->body)
                asm_gen_stmt(cg, node->body);
            return;

        default:
            break;
    }

    throw_error(ERR_CODEGEN, node->tok, "unexpected statement");
}

static void asm_store_fp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz)
{
    switch(sz)
    {
        case 4:
            asm_println(cg, "  movss %%xmm%d, %d(%%rbp)", r, offset);
            return;
        case 8:
            asm_println(cg, "  movsd %%xmm%d, %d(%%rbp)", r, offset);
            return;
    }
    unreachable();
}

static void asm_store_gp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz)
{
    switch(sz)
    {
        case 1:
            asm_println(cg, "  mov %s, %d(%%rbp)", argreg8[r], offset);
            return;
        case 2:
            asm_println(cg, "  mov %s, %d(%%rbp)", argreg16[r], offset);
            return;
        case 4:
            asm_println(cg, "  mov %s, %d(%%rbp)", argreg32[r], offset);
            return;
        case 8:
            asm_println(cg, "  mov %s, %d(%%rbp)", argreg64[r], offset);
            return;
        default:
            for(i32 i = 0; i < sz; i++)
            {
                asm_println(cg, "  mov %s, %d(%%rbp)", argreg8[r], offset + i);
                asm_println(cg, "  shr $8, %s", argreg64[r]);
            }
            return;
    }
}

static void asm_gen_lambda(ASMCodegenData_T* cg, ASTObj_T* lambda_obj)
{
    ASTNode_T* lambda = lambda_obj->body;
    char* prev_fn_name = cg->current_fn_name;
    char lambda_name[BUFSIZ] = {};
    sprintf(lambda_name, "lambda.%ld", lambda->long_val);
    cg->current_fn_name = &(lambda_name[0]);

    asm_println(cg, "  .globl %s", lambda_name);
    asm_println(cg, "  .text");
    asm_println(cg, "  .type %s, @function", lambda_name);
    asm_println(cg, "%s:", lambda_name);
    asm_println(cg, "  push %%rbp");
    asm_println(cg, "  mov .lambda.stackptr.%ld, %%rbp", lambda->long_val);

    // save passed-by-register arguments to the stack
    i32 gp = 0, fp = 0;

    if(lambda->return_ptr)
        asm_store_gp(cg, gp++, lambda->return_ptr->offset, lambda->return_ptr->data_type->size);

    for(size_t j = 0; j < lambda->args->size; j++)
    {
        ASTObj_T* arg = lambda->args->items[j];
        if(arg->offset > 0)
            continue;
                        
        ASTType_T* ty = unpack(arg->data_type);

        switch(ty->kind)
        {
            case TY_STRUCT:
                assert(ty->size <= 16);
                if(asm_has_flonum(ty, 0, 8, 0))
                    asm_store_fp(cg, fp++, arg->offset, MIN(8, ty->size));
                else
                    asm_store_gp(cg, gp++, arg->offset, MIN(8, ty->size));
                                
                if(ty->size > 8)
                {
                    if(asm_has_flonum(ty, 8, 16, 0))
                        asm_store_fp(cg, fp++, arg->offset + 8, ty->size - 8);
                    else
                        asm_store_gp(cg, gp++, arg->offset + 8, ty->size - 8);
                }
                break;
            case TY_F32:
            case TY_F64:
                asm_store_fp(cg, fp++, arg->offset, ty->size);
                break;
            default:
                asm_store_gp(cg, gp++, arg->offset, ty->size);
        }
    }

    ASTObj_T* prev = cg->current_fn;
    cg->current_fn = lambda_obj; 
    asm_gen_stmt(cg, lambda->body);
    cg->current_fn = prev;

    asm_println(cg, ".L.return.%s:", lambda_name);
    asm_gen_defer(cg, lambda_obj->deferred);
    asm_println(cg, "  pop %%rbp");
    asm_println(cg, "  ret");

    cg->current_fn_name = prev_fn_name;
}

static void asm_gen_string_literals(ASMCodegenData_T* cg) 
{
    for(size_t i = 0; i < cg->string_literals->size; i++)
    {
        const char* str = cg->string_literals->items[i];
        size_t size = (strlen(str) + 1) * CHAR_S;

        asm_println(cg, "  .data");    
        asm_println(cg, "  .align 1");
        asm_println(cg, "  .size .L.string.%d, %d", i, size);
        asm_println(cg, ".L.string.%d:", i);
        
        for(size_t i = 0; i < size; i++)
            asm_println(cg, "  .byte %u", str[i] == '\\' ? (i++, escape_sequence(str[i], &str[i], &i)) : str[i]);
    }
}
