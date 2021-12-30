#include "asm_codegen.h"
#include "../../io/log.h"
#include "../../io/io.h"
#include "../../ast/ast_iterator.h"
#include "../../platform/platform_bindings.h"
#include "../codegen_utils.h"
#include "../../error/error.h"
#include "../../ast/mem/ast_mem.h"

#include <stdio.h>
#include <assert.h>

#define GP_MAX 6
#define FP_MAX 8

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
static void asm_assign_lvar_offsets(ASMCodegenData_T* cg, List_T* objs);
static bool asm_has_flonum(ASTType_T* ty, i32 lo, i32 hi, i32 offset);
static void asm_store_fp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz);
static void asm_store_gp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz);
static void asm_gen_stmt(ASMCodegenData_T* cg, ASTNode_T* node);

void init_asm_cg(ASMCodegenData_T* cg, ASTProg_T* ast)
{
    cg->ast = ast;
    cg->print = false;
    cg->silent = false;

    cg->code_buffer = open_memstream(&cg->buf, &cg->buf_len);
    cg->current_fn = NULL;
    cg->current_fn_name = NULL;
    cg->depth = 0;
}

__attribute((format(printf, 2, 3)))
static void asm_println(ASMCodegenData_T* cg, char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(cg->code_buffer, fmt, va);
    va_end(va);
    fprintf(cg->code_buffer, "\n");
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
    asm_assign_lvar_offsets(cg, cg->ast->objs);
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
        const char* args[] = {
            "as",
            "-c",
            asm_source_file,
            "-o",
            obj_file,
            NULL
        };
        i32 exit_code = subprocess(args[0], (char* const*) args, false);

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

        const char* args[] = {
            "ld",
            "-o",
            target,
            "-m",
            "elf_x86_64",
            "/usr/lib64/crt1.o",
            "/usr/lib64/crti.o",
            "/usr/lib/gcc/x86_64-pc-linux-gnu/11.1.0/crtbegin.o",
            "-L/usr/lib/gcc/x86_64-pc-linux-gnu/11.1.0",
            "-L/usr/lib/x86_64-linux-gnu",
            "-L/usr/lib64",
            "-L/lib64",
            "-L/usr/lib/x86_64-linux-gnu",
            "-L/usr/lib/x86_64-pc-linux-gnu",
            "-L/usr/lib/x86_64-redhat-linux",
            "-L/usr/lib",
            "-L/lib",
            "-dynamic-linker",
            "/lib64/ld-linux-x86-64.so.2",
            obj_file,
            "-lc",
            "-lgcc",
            "--as-needed",
            "-lgcc_s",
            "--no-as-needed",
            "/usr/lib/gcc/x86_64-pc-linux-gnu/11.1.0/crtend.o",
            "/usr/lib64/crtn.o",
            NULL
        };

        i32 exit_code = subprocess(args[0], (char* const*) args, false);
        if(exit_code != 0)
        {
            LOG_ERROR_F("error linking code. (exit code %d)\n", exit_code);
            exit(1);
        }
    }
}

static char* asm_gen_identifier(ASTIdentifier_T* id)
{
    char* str = gen_identifier(id);
    ast_mem_add_ptr(str);
    return str;
}

static void asm_gen_file_descriptors(ASMCodegenData_T* cg)
{
    for(size_t i = 0; i < cg->ast->imports->size; i++)
    {
        SrcFile_T* file = cg->ast->imports->items[i];
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
            asm_gen_text(cg, obj->objs);
            break;
        
        case OBJ_FUNCTION:
        {
            i32 top = 16, bottom = 0;
            i32 gp = 0, fp = 0;

            // Assign offsets to pass-by-stack parameters
            for(size_t j = 0; j < obj->args->size; j++)
            {
                ASTObj_T* var = obj->args->items[j];
                ASTType_T* ty = var->data_type;
                switch(ty->kind)
                {
                case TY_STRUCT:
                    if(ty->size <= 16)
                    {
                        bool fp1 = asm_has_flonum(ty, 0, 8, 0);
                        bool fp2 = asm_has_flonum(ty, 8, 16, 8);
                        if(fp + fp1 + fp2 < FP_MAX && gp + !fp1 + !fp2 < GP_MAX)
                        {
                            fp = fp + fp1 + fp2;
                            gp = gp + !fp1 + fp2;
                            continue;
                        }
                    } break;
                
                case TY_F32:
                case TY_F64:
                    if(fp++ < FP_MAX)
                        continue;
                    break;
                case TY_F80:
                    break;
                default:
                    if(gp++ < GP_MAX)
                        continue;
                }

                top = align_to(top, 8);
                var->offset = top;
                top += var->data_type->size;
            }

            // alloca size
            {
                bottom += obj->alloca_bottom->data_type->size;
                bottom = align_to(bottom, obj->alloca_bottom->align);
                obj->alloca_bottom->offset = -bottom;
            }

            for(size_t j = 0; j < obj->args->size; j++)
            {
                ASTObj_T* var = obj->args->items[j];
                //if(var->offset)
                //    continue;
                
                // AMD64 System V ABI has a special alignment rule for an array of
			    // length at least 16 bytes. We need to align such array to at least
			    // 16-byte boundaries. See p.14 of
			    // https://github.com/hjl-tools/x86-psABI/wiki/x86-64-psABI-draft.pdf.
			    
                int align = var->data_type->kind == TY_ARR && var->data_type->size >= 16 ? MAX(16, var->align) : var->align;
                bottom += var->data_type->size;
                bottom = align_to(bottom, align);
                var->offset = -bottom;
            }

            obj->stack_size = align_to(bottom, 16);
        } break;
        }
    }
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
            
            case OBJ_GLOBAL:
                if(obj->is_extern)
                    continue;
                {
                    char* id = asm_gen_identifier(obj->id);
                    asm_println(cg, "  .globl %s", id);

                    int align = obj->data_type->kind == TY_ARR && obj->data_type->size >= 16 ? MAX(10, obj->align) : obj->align;

                    bool is_tls = false; //fixme: evaluate correctly
                    if(obj->value)
                    {
                        if(is_tls)
                            asm_println(cg, "  .section .tdata,\"awT\",@progbits");
                        else
                            asm_println(cg, "  .data");

                        asm_println(cg, "  .type %s, @object", id);
                        asm_println(cg, "  .size %s, %d", id, obj->data_type->size);
                        asm_println(cg, "  .align %d", align);
                        asm_println(cg, "%s:", id);

                        // todo: evaluate relocation

                        continue;
                    }

                    if(is_tls)
                        asm_println(cg, "  .section .tbss,\"awT\",@nobits");
                    else
                        asm_println(cg, "  .bss");
                    
                    asm_println(cg, "  .align %d", align);
                    asm_println(cg, "%s:", id);
                    asm_println(cg, "  .zero %d", obj->data_type->size);
                } break;

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

                    char* fn_name = asm_gen_identifier(obj->id);
                    asm_println(cg, "  .globl %s", fn_name);
                    asm_println(cg, "  .text");
                    asm_println(cg, "  .type %s, @function", fn_name);
                    asm_println(cg, "%s:", fn_name);

                    cg->current_fn = obj;

                    // prologue
                    asm_println(cg, "  push %%rbp");
                    asm_println(cg, "  mov %%rsp, %%rbp");
                    asm_println(cg, "  sub $%d, %%rsp", obj->stack_size);
                    asm_println(cg, "  mov %%rsp, %d(%%rbp)", obj->alloca_bottom->offset);

                    // save arg registers if function is variadic
                    if(obj->args->size >= 1 && ((ASTObj_T*)obj->args->items[obj->args->size - 1])->data_type->kind == TY_VA_LIST)
                    {
                        i32 gp = 0, fp = 0;
                        for(size_t j = 0; j < obj->args->size; j++)
                        {
                            ASTObj_T* arg = obj->args->items[j];
                            if(is_flonum(arg->data_type))
                                fp++;
                            else
                                gp++;
                            
                            i32 off = ((ASTObj_T*)obj->args->items[obj->args->size - 1])->offset;

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
                    }

                    // save passed-by-register arguments to the stack
                    i32 gp = 0, fp = 0;
                    for(size_t j = 0; j < obj->args->size; j++)
                    {
                        ASTObj_T* arg = obj->args->items[j];
                        if(arg->offset > 0)
                            continue;
                        
                        ASTType_T* ty = arg->data_type;

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
                    assert(cg->depth == 0);
                    cg->current_fn_name = NULL;

                    // epilogue
                    asm_println(cg, ".L.return.%s:", fn_name);
                    asm_println(cg, "  mov %%rbp, %%rsp");
                    asm_println(cg, "  pop %%rbp");
                    asm_println(cg, "  ret");
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
    asm_println(cg, "  push %%rax");
    cg->depth++;
}

static void asm_pop(ASMCodegenData_T* cg, char *arg) 
{
    asm_println(cg, "  pop %s", arg);
    cg->depth--;
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


static char *asm_reg_dx(i32 sz) 
{
    switch (sz) 
    {
        case 1: return "%dl";
        case 2: return "%dx";
        case 4: return "%edx";
        case 8: return "%rdx";
    }
    unreachable();
    return 0;
}

static char *asm_reg_ax(i32 sz) 
{
    switch (sz) 
    {
        case 1: return "%al";
        case 2: return "%ax";
        case 4: return "%eax";
        case 8: return "%rax";
    }
    unreachable();
    return 0;
}

static void gen_addr(ASMCodegenData_T* cg, union {ASTNode_T* node; ASTObj_T* obj;} u)
{

}

static bool asm_has_flonum(ASTType_T* ty, i32 lo, i32 hi, i32 offset)
{
    if(ty->kind == TY_STRUCT)
    {
        for(size_t i = 0; i < ty->members->size; i++)
            // todo: apply correct offset
            if(!asm_has_flonum(((ASTObj_T*) ty->members->items[i])->data_type, lo, hi, offset));
                return false;
        return true;
    }
    else if(ty->kind == TY_ARR)
    {
        // todo: iterate over every index and check
    }

    return offset < lo || hi <= offset || ty->kind == TY_F32 || ty->kind == TY_F64 || ty->kind == TY_F80; 
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
    i32 sz = align_to(ty->size, 8);
    asm_println(cg, "  sub $%d, %%rsp", sz);
    cg->depth += sz / 8;
    
    for(i32 i = 0; i < ty->size; i++)
    {
        asm_println(cg, "  mov %d(%%rax), %%r10b", i);
        asm_println(cg, "  mov %%r10b, %d(%%rsp)", i);
    }
}

static void asm_copy_struct_reg(ASMCodegenData_T* cg)
{
    ASTType_T* ty = cg->current_fn->return_type;
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
        asm_println(cg, "  mov $0, %%rax");
        for(i32 i = MIN(8, ty->size) - 1; i >= 0; i--)
        {
            asm_println(cg, "  shl $8, %%rax");
            asm_println(cg, "  mov %d(%%rdi), %%al", i);
        }
        gp++;
    }

    if(ty->size > 8)
    {
        if(asm_has_flonum(ty, 8, 16, 0))
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
            asm_println(cg, "  mov $0, %s", reg2);
            for(i32 i = MIN(16, ty->size) - 1; i >= 8; i--)
            {
                asm_println(cg, "  shl $8, %s", reg2);
                asm_println(cg, "  mov %d(%%rdi), %s", i, reg1);
            }
        }
    }
}

static void asm_copy_struct_mem(ASMCodegenData_T* cg)
{

}

static void asm_copy_ret_buffer(ASMCodegenData_T* cg, ASTObj_T* var)
{

}

static void asm_cmp_zero(ASMCodegenData_T* cg, ASTType_T* ty)
{
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

    switch (ty->kind) {
        case TY_STRUCT:
            for (i32 i = 0; i < ty->size; i++) {
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
    switch (ty->kind) {
        case TY_ARR:
        case TY_STRUCT:
        case TY_FN:
        case TY_VA_LIST:
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

static void asm_push_args2(ASMCodegenData_T* cg, ASTObj_T* arg, bool first_pass)
{

}

static i32 asm_push_args(ASMCodegenData_T* cg, ASTObj_T* fn)
{
    return 0;
}

static void asm_builtin_alloca(ASMCodegenData_T* cg)
{
    // Align size to 16 bytes.
	asm_println(cg, "  add $15, %%rdi");
	asm_println(cg, "  and $0xfffffff0, %%edi");

	// Shift the temporary area by %rdi.
	asm_println(cg, "  mov %d(%%rbp), %%rcx", cg->current_fn->alloca_bottom->offset);
	asm_println(cg, "  sub %%rsp, %%rcx");
	asm_println(cg, "  mov %%rsp, %%rax");
	asm_println(cg, "  sub %%rdi, %%rsp");
	asm_println(cg, "  mov %%rsp, %%rdx");
	asm_println(cg, "1:");
	asm_println(cg, "  cmp $0, %%rcx");
	asm_println(cg, "  je 2f");
	asm_println(cg, "  mov (%%rax), %%r8b");
	asm_println(cg, "  mov %%r8b, (%%rdx)");
	asm_println(cg, "  inc %%rdx");
	asm_println(cg, "  inc %%rax");
	asm_println(cg, "  dec %%rcx");
	asm_println(cg, "  jmp 1b");
	asm_println(cg, "2:");

	// Move alloca_bottom pointer.
	asm_println(cg, "  mov %d(%%rbp), %%rax", cg->current_fn->alloca_bottom->offset);
	asm_println(cg, "  sub %%rdi, %%rax");
	asm_println(cg, "  mov %%rax, %d(%%rbp)", cg->current_fn->alloca_bottom->offset);
}

static void asm_gen_expr(ASMCodegenData_T* cg, ASTNode_T* node)
{
    if(node->tok)
        asm_println(cg, "  .loc %d %d", node->tok->source->file_no + 1, node->tok->line);
    
    switch(node->kind)
    {
        case ND_NIL:
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
                asm_println(cg, "  mov $%lu, %%rax  # float %Lf", u.u64, node->double_val);
                asm_println(cg, "  movq %%rax, %%xmm0");
            } return;
        case ND_INT:
            asm_println(cg, "  mov $%d, %%rax", node->int_val);
            return;
        case ND_LONG:
            asm_println(cg, "  mov $%ld, %%rax", node->long_val);
            return;
        case ND_LLONG:
            asm_println(cg, "  mov $%lld, %%rax", node->llong_val);
            return;
    }
}

static void asm_gen_stmt(ASMCodegenData_T* cg, ASTNode_T* node)
{
    asm_println(cg, "  .loc %d %d", node->tok->source->file_no + 1, node->tok->line + 1);

    switch(node->kind)
    {
        case ND_IF:
        {
            i32 c = asm_count();
            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  je  .L.else.%d", c);
            asm_gen_stmt(cg, node->if_branch);
            asm_println(cg, "  jmp .L.end.%d", c);
            asm_println(cg, ".L.else.%d:", c);
            if(node->else_branch)
                asm_gen_stmt(cg, node->else_branch);
            asm_println(cg, ".L.end.%d:", c);
        } return;

        case ND_FOR:
            return;
        
        case ND_MATCH:
            return;
        
        case ND_CASE:
            return;
        
        case ND_WHILE:
            return;
        
        case ND_BLOCK:
            for(size_t i = 0; i < node->stmts->size; i++)
                asm_gen_stmt(cg, node->stmts->items[i]);
        
        case ND_RETURN:
            if(node->return_val)
            {
                asm_gen_expr(cg, node->return_val);
                ASTType_T* ty = node->return_val->data_type;

                if(ty->kind == TY_STRUCT)
                {
                    if(ty->size <= 16)
                        asm_copy_struct_reg(cg);
                    else
                        asm_copy_struct_mem(cg);
                }
            }

            asm_println(cg, "  jmp .L.return.%s", cg->current_fn_name);
            return;
        
        case ND_EXPR_STMT:
            asm_gen_expr(cg, node->expr);
            return;

        default:
            break;
    }

    unreachable();
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