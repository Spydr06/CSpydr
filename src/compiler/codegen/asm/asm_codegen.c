#include "asm_codegen.h"
#include "../../io/log.h"
#include "../../io/io.h"
#include "../../ast/ast_iterator.h"
#include "../../platform/platform_bindings.h"
#include "../codegen_utils.h"
#include "../../error/error.h"
#include "../../mem/mem.h"
#include "../../ast/types.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#define GP_MAX 6
#define FP_MAX 8

enum { I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, F80, LAST };

// a counter variable for generating unique labels
static u64 asm_c = 0;

const char asm_start_text[] = 
    "  .globl _start\n"
    "  .text\n"
    "_start:\n"
    "  xorl %ebp, %ebp\n"
    "  popq %rdi\n"
    "  movq %rsp, %rsi\n"
    "  andq $~15, %rsp\n"
    "  call main\n"
    "  movq %rax, %rdi\n"
    "  movq $60, %rax\n"
    "  syscall";

static char *argreg8[]  = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *argreg16[] = {"%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
static char *argreg32[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
static char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

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

#define CONVERT_INDEX_NODE(node)                                  \
    (ASTNode_T) {                                                 \
        .kind = ND_DEREF,                                         \
        .data_type = node->data_type,                             \
        .right = &(ASTNode_T) {                                   \
            .kind = ND_CAST,                                      \
            .data_type = (ASTType_T*) primitives[TY_I64],         \
            .left = &(ASTNode_T) {                                \
                .kind = ND_ADD,                                   \
                .data_type = (ASTType_T*) primitives[TY_I64],     \
                .left = node->left,                               \
                .right = &(ASTNode_T) {                           \
                    .kind = ND_CAST,                              \
                    .data_type = (ASTType_T*) primitives[TY_I64], \
                    .left = node->expr                            \
                }                                                 \
            }                                                     \
        }                                                         \
    }

static void generate_files(ASMCodegenData_T* cg);
static void asm_gen_file_descriptors(ASMCodegenData_T* cg);
static void asm_gen_data(ASMCodegenData_T* cg, List_T* objs);
static void asm_gen_text(ASMCodegenData_T* cg, List_T* objs);
static void asm_assign_lvar_offsets(ASMCodegenData_T* cg, List_T* objs);
static bool asm_has_flonum(ASTType_T* ty, i32 lo, i32 hi, i32 offset);
static bool asm_has_flonum_1(ASTType_T* ty);
static bool asm_has_flonum_2(ASTType_T* ty);
static void asm_store_fp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz);
static void asm_store_gp(ASMCodegenData_T* cg, i32 r, i32 offset, i32 sz);
static void asm_gen_stmt(ASMCodegenData_T* cg, ASTNode_T* node);
static void asm_gen_expr(ASMCodegenData_T* cg, ASTNode_T* node);

void init_asm_cg(ASMCodegenData_T* cg, ASTProg_T* ast)
{
    cg->ast = ast;
    cg->print = false;
    cg->silent = false;
    cg->embed_file_locations = false;

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

static void get_platform(char* dest)
{
#if defined (__linux__) || defined(__linux)
#if defined(__x86_64) || defined(__x86_64__)
    strcpy(dest, "x86_64");
#else
    #warn "unsupported assembler platform"
#endif
    strcat(dest, "-linux");
#if defined(__GNUC__) || defined(__gnu_linux__)
    strcat(dest, "-gnu");
#else
    #warn "unsupported assembler platoform"
#endif
#else
    #warn "unsupported assembler platform"
#endif
}

void asm_gen_code(ASMCodegenData_T* cg, const char* target)
{
    char platform[1024] = { '\0' };
    get_platform(platform);
    if(!cg->silent)
    {
        LOG_OK_F(COLOR_BOLD_BLUE "  Generating" COLOR_BOLD_WHITE " assembly" COLOR_RESET " for " COLOR_BOLD_WHITE "%s" COLOR_RESET "\n", platform);
    }

    // generate the assembly code
    if(cg->embed_file_locations)
        asm_gen_file_descriptors(cg);
    asm_assign_lvar_offsets(cg, cg->ast->objs);
    asm_gen_data(cg, cg->ast->objs);
    
    asm_println(cg, "%s", asm_start_text);
    
    asm_gen_text(cg, cg->ast->lambda_literals);
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
            LOG_OK_F(COLOR_BOLD_BLUE "  Linking    " COLOR_RESET "%s\n", target);

        List_T* args = init_list(sizeof(char*));
        list_push(args, "ld");
        list_push(args, "-o");
        list_push(args, (void*) target);
        list_push(args, "-m");
        list_push(args, "elf_x86_64");
        list_push(args, "-L/usr/lib64");
        list_push(args, "-L/lib64");
        list_push(args, "-L/usr/lib");
        list_push(args, "-L/lib");

        for(size_t i = 0; i < global.linker_flags->size; i++)
            list_push(args, global.linker_flags->items[i]);

        list_push(args, "-dynamic-linker");
        list_push(args, "/lib64/ld-linux-x86-64.so.2");
        list_push(args, obj_file);
        list_push(args, NULL);


        i32 exit_code = subprocess((char*) args->items[0], (char* const*) args->items, false);
        if(exit_code != 0)
        {
            LOG_ERROR_F("error linking code. (exit code %d)\n", exit_code);
            exit(1);
        }

        free_list(args);
    }
}

static char* asm_gen_identifier(ASTIdentifier_T* id)
{
    char* str = gen_identifier(id);
    mem_add_ptr(str);
    return str;
}

static void asm_gen_file_descriptors(ASMCodegenData_T* cg)
{
    for(size_t i = 0; i < cg->ast->imports->size; i++)
    {
        SrcFile_T* file = cg->ast->imports->items[i];
        asm_println(cg, "  .file %d \"%s\"", file->file_no + 1, file->short_path);
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

            // assign function argument offsets
            for(size_t j = 0; j < obj->args->size; j++)
            {
                ASTObj_T* var = obj->args->items[j];
                if(var->offset)
                    continue;
                
                // AMD64 System V ABI has a special alignment rule for an array of
			    // length at least 16 bytes. We need to align such array to at least
			    // 16-byte boundaries. See p.14 of
			    // https://github.com/hjl-tools/x86-psABI/wiki/x86-64-psABI-draft.pdf.
			    
                i32 align = var->data_type->kind == TY_ARR && var->data_type->size >= 16 ? MAX(16, var->align) : var->align;
                bottom += var->data_type->size;
                bottom = align_to(bottom, align);
                var->offset = -bottom;
            }

            // assign local offsets
            if(!obj->is_extern)
                for(size_t j = 0; j < obj->objs->size; j++)
                {
                    ASTObj_T* var = obj->objs->items[j];
                    i32 align = var->data_type->kind == TY_ARR && var->data_type->size >= 16 ? MAX(16, var->align) : var->align;
                    bottom += var->data_type->size;
                    bottom = align_to(bottom, align);
                    var->offset = -bottom;
                }
            obj->stack_size = align_to(bottom, 16);
        } break;
        }
    }
}

static void asm_gen_relocation(ASMCodegenData_T* cg, ASTObj_T* var, ASTNode_T* val)
{
    if(var && !val)
        val = var->value;

    switch(val->kind)
    {
        case ND_NIL:
            asm_println(cg, "  .zero %d", (var ? var->data_type : val->data_type)->size);
            return;
        case ND_CLOSURE:
            asm_gen_relocation(cg, var, val->expr);
            return;
        case ND_STR:
            asm_println(cg,"  .ascii \"%s\\0\"", val->str_val);
            return;
        case ND_CHAR:
            asm_println(cg, "  .byte %d", val->str_val[0]);
            return;
        case ND_BOOL:
            asm_println(cg, "  .byte %d", val->bool_val);
            return;
        case ND_INT:
            {
                u8* bytes = (u8*)&val->int_val;
                for(i32 i = 0; i < I32_S; i++)
                    asm_println(cg, "  .byte %d", bytes[i]);
            } return;
        case ND_LONG:
            {
                u8* bytes = (u8*)&val->long_val;
                for(i32 i = 0; i < I64_S; i++)
                    asm_println(cg, "  .byte %d", bytes[i]);
            } return;
        case ND_LLONG:
            {
                u8* bytes = (u8*)&val->llong_val;
                for(i32 i = 0; i < I64_S; i++)
                    asm_println(cg, "  .byte %d", bytes[i]);
            } return;
        case ND_FLOAT:
            {
                union { f32 f32; u8 bytes[sizeof(f32)]; } u = { .f32 = val->float_val};
                for(i32 i = 0; i < F32_S; i++)
                    asm_println(cg, "  .byte %d", u.bytes[i]);
            } return;
        case ND_DOUBLE:
            {
                union { f64 f64; u8 bytes[sizeof(f64)]; } u = { .f64 = val->float_val};
                for(i32 i = 0; i < F64_S; i++)
                    asm_println(cg, "  .byte %d", u.bytes[i]);
            } return;
        case ND_ARRAY:
            {
                for(size_t i = 0; i < val->args->size; i++)
                    asm_gen_relocation(cg, NULL, val->args->items[i]);
            } return;
        case ND_STRUCT:
            {
                LOG_ERROR("not implemented\n");
            } return;
        default:
            throw_error(ERR_CODEGEN, val->tok, "cannot generate relocation for `%s`", val->tok->value);
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
                        char* id = asm_gen_identifier(member->id);
                        asm_println(cg, "  .globl %s", id);
                        asm_println(cg, "  .data");
                        asm_println(cg, "  .type %s, @object", id);
                        asm_println(cg, "  .size %s, 4", id);
                        asm_println(cg, "  .align 4");
                        asm_println(cg, "%s:", id);

                        asm_gen_relocation(cg, member, NULL); 
                    }
            } break;

            case OBJ_GLOBAL:
                if(obj->is_extern)
                    continue;
                {
                    char* id = asm_gen_identifier(obj->id);
                    asm_println(cg, "  .globl %s", id);

                    i32 align = obj->data_type->kind == TY_ARR && obj->data_type->size >= 16 ? MAX(16, obj->align) : obj->align;

                    if(obj->value)
                    {
                        asm_println(cg, "  .data");
                        asm_println(cg, "  .type %s, @object", id);
                        asm_println(cg, "  .size %s, %d", id, obj->data_type->size);
                        asm_println(cg, "  .align %d", align);
                        asm_println(cg, "%s:", id);

                        asm_gen_relocation(cg, obj, NULL);

                        continue;
                    }
                    
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

static u64 asm_count(void) 
{
    return asm_c++;
}

static u64 asm_current_count(void)
{
    return asm_c;
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

static void asm_gen_addr(ASMCodegenData_T* cg, ASTNode_T* node)
{
    switch(node->kind)
    {
        case ND_CLOSURE:
            asm_gen_addr(cg, node->expr);
            return;

        case ND_ID: 
            if(unpack(node->data_type)->is_vla)
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
                    asm_println(cg, "  lea %s(%%rip), %%rax", asm_gen_identifier(node->id));
                    return;
                
                case OBJ_FUNCTION:
                    if(node->referenced_obj->is_extern)
                        asm_println(cg, "  mov %s@GOTPCREL(%%rip), %%rax", asm_gen_identifier(node->id));
                    else
                        asm_println(cg, "  mov %s(%%rip), %%rax", asm_gen_identifier(node->id));
                    return;
                
                default:
                    throw_error(ERR_CODEGEN, node->tok, "`%s` reference object of unexpected kind", node->id->callee);
            }
            return;
        
        case ND_CALL:
            if(node->called_obj->is_extern)
                asm_println(cg, "  mov %s@GOTPCREL(%%rip), %%rax", asm_gen_identifier(node->expr->id));
            else
                asm_println(cg, "  lea %s(%%rip), %%rax", asm_gen_identifier(node->expr->id));
            return;

        case ND_DEREF:
            asm_gen_expr(cg, node->right);
            return;
        case ND_MEMBER:
            asm_gen_addr(cg, node->left);
            asm_println(cg, "  add $%d, %%rax", node->body->int_val);
            return;
        case ND_INDEX:
            {
                // x[y] gets converted to *(x + y * sizeof *x)
                ASTNode_T converted = CONVERT_INDEX_NODE(node);
                asm_gen_addr(cg, &converted);
            }
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
            if(!asm_has_flonum(member->data_type, lo, hi, offset + member->int_val /*offset*/))
                return false;
        }
        return true;
    }
    else if(ty->kind == TY_ARR)
    {
        for(size_t i = 0; i < ty->size / ty->base->size; i++)
            if(!asm_has_flonum(ty->base, lo, hi, offset + ty->base->size * i))
                return false;
        return true;
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
    i32 sz = align_to(unpack(ty)->size, 8);
    asm_println(cg, "  sub $%d, %%rsp", sz);
    cg->depth += sz / 8;
    
    for(i32 i = 0; i < unpack(ty)->size; i++)
    {
        asm_println(cg, "  mov %d(%%rax), %%r10b", i);
        asm_println(cg, "  mov %%r10b, %d(%%rsp)", i);
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
    ASTType_T* ty = cg->current_fn->return_type;
    ASTObj_T* var = cg->current_fn->args->items[0];

    asm_println(cg, "  mov %d(%%rbp), %%rdi", var->offset);

    for(u32 i = 0; i < ty->size; i++)
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
        for(u32 i = 0; i < MIN(8, ty->size); i++)
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
            for(u32 i = 0; i < MIN(16, ty->size); i++) 
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
    ty = unpack(ty);
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

static void asm_push_args2(ASMCodegenData_T* cg, List_T* args, bool first_pass)
{
    for(i64 i = args->size - 1; i >= 0; i--)
    {
        ASTNode_T* arg = args->items[i];
        if((first_pass && !arg->pass_by_stack) || (!first_pass && arg->pass_by_stack))
            return;
        
        asm_gen_expr(cg, arg);

        switch(arg->data_type->kind)
        {
            case TY_STRUCT:
                asm_push_struct(cg, arg->data_type);
                break;
            case TY_F32:
                //asm_cast(cg, (ASTType_T*) primitives[TY_F32], (ASTType_T*) primitives[TY_F64]);
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
        ASTType_T* ty = unpack(arg->data_type);

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

                    if(fp + fp1 + fp2 < FP_MAX && gp + !fp + !fp2 < GP_MAX)
                    {
                        fp = fp + fp1 + fp2;
                        gp = gp + !fp1 + !fp2;
                    }
                    else
                    {
                        arg->pass_by_stack = true;
                        stack += align_to(ty->size, 8) / 8;
                    }
                } break;
            case TY_F32:
            case TY_F64:
                if(fp++ >= FP_MAX)
                {
                    arg->pass_by_stack = true;
                    stack++;
                } break;
            case TY_F80:
                arg->pass_by_stack = true;
                stack += 2;
                break;
            default:
                if(gp++ >= GP_MAX) {
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

static void asm_gen_inc(ASMCodegenData_T* cg, ASTNode_T* node)
{
    // convert x++ to (x = x + 1) - 1
    //         x-- to (x = x + -1) - -1
    ASTNode_T addend = {
        .kind = ND_INT,
        .int_val = node->kind == ND_INC ? 1 : -1
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
            } 
        }
    };

    asm_gen_expr(cg, &converted);
}

static void asm_gen_expr(ASMCodegenData_T* cg, ASTNode_T* node)
{
    if(node->tok && cg->embed_file_locations)
        asm_println(cg, "  .loc %d %d", node->tok->source->file_no + 1, node->tok->line + 1);
    
    switch(node->kind)
    {
        case ND_NOOP:
            return;
        case ND_ASM:
            asm_println(cg, "%s", node->expr->str_val);
            return;
        case ND_CLOSURE:
            asm_gen_expr(cg, node->expr);
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
            asm_println(cg, "  mov $%d, %%rax", node->int_val);
            return;
        case ND_LONG:
            asm_println(cg, "  mov $%ld, %%rax", node->long_val);
            return;
        case ND_LLONG:
            asm_println(cg, "  mov $%lld, %%rax", (long long) node->llong_val);
            return;
        case ND_CHAR:
            asm_println(cg, "  mov $%d, %%rax", node->str_val[0]);
            return;
        case ND_NIL:
            asm_println(cg, "  mov $0, %%rax");
            return;
        
        case ND_SIZEOF:
            asm_println(cg, "  mov $%d, %%rax", node->the_type->size);
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
            {            
                node->left->data_type = unpack(node->left->data_type);
                if(!node->left->data_type || !node->left->data_type->base)
                    throw_error(ERR_TYPE_ERROR, node->tok, "Cannot get index of data type `%d`", node->data_type->kind);
                // x[y] gets converted to *(x + y * sizeof *x)
                ASTNode_T converted = CONVERT_INDEX_NODE(node);
                asm_gen_expr(cg, &converted);
            } return;
        
        case ND_INC:
        case ND_DEC:
            asm_gen_inc(cg, node);
            return;

        case ND_ADD:
            if(unpack(node->left->data_type)->base && unpack(node->right->data_type)->base)
                throw_error(ERR_SYNTAX_ERROR, node->tok, "cannot add two pointer types together");
            if(unpack(node->left->data_type)->base)
            {
                // if we add a number to a pointer, multiply the second argument with the base type size
                // a + b -> a + b * sizeof *a
                ASTNode_T new_right = {
                    .kind = ND_MUL,
                    .data_type = node->right->data_type,
                    .left = node->right,
                    .right = &(ASTNode_T) {
                        .kind = ND_LONG,
                        .data_type = (ASTType_T*) primitives[TY_I64],
                        .long_val = unpack(node->left->data_type)->base->size
                    }
                };

                node->right = &new_right;
            }
            
            break;
        
        case ND_SUB:
            if(unpack(node->left->data_type)->base && is_integer(node->right->data_type))
            {
                // if we subtract a number from a pointer, multiply the second argument with the base type size
                // a - b -> a - b * sizeof *a
                ASTNode_T new_right = {
                    .kind = ND_MUL,
                    .data_type = node->right->data_type,
                    .left = node->right,
                    .right = &(ASTNode_T) {
                        .kind = ND_LONG,
                        .data_type = (ASTType_T*) primitives[TY_I64],
                        .long_val = unpack(node->left->data_type)->size
                    }
                };

                node->right = &new_right;
            }
            else if(unpack(node->left->data_type)->base && unpack(node->left->data_type)->base)
            {
                // if both subtraction arguments are pointers, return the number of elements in between
                // a - b -> (a - b) / sizeof *a

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
            return;

        case ND_MEMBER:
            asm_gen_addr(cg, node);
            asm_load(cg, node->data_type);
            return;
        
        case ND_STRUCT:
            throw_error(ERR_CODEGEN, node->tok, "cannot have struct literal at this place");
            return;
        
        case ND_ARRAY:
            throw_error(ERR_CODEGEN, node->tok, "cannot have array literal at this place");
            return;

        case ND_REF:
            asm_gen_addr(cg, node->right);
            return;
        
        case ND_DEREF:
            asm_gen_expr(cg, node->right);
            asm_load(cg, node->data_type);
            return;
        
        case ND_ASSIGN:
            if(unpack(node->left->data_type)->base)
            {   
                switch(node->right->kind)
                {
                    case ND_STRUCT:
                        printf("not implemented!\n");
                        return;
                    case ND_ARRAY:
                        // convert x = [y, z, w] to x[0] = y, x[1] = z, x[2] = w
                        for(size_t i = 0; i < node->right->args->size; i++)
                        {
                            ASTNode_T converted = {
                                .kind = ND_ASSIGN,
                                .data_type = unpack(node->left->data_type)->base,
                                .left = &(ASTNode_T) {
                                    .kind = ND_INDEX,
                                    .data_type = unpack(node->left->data_type)->base,
                                    .left = node->left,
                                    .expr = &(ASTNode_T) {
                                        .kind = ND_LONG,
                                        .data_type = (ASTType_T*) primitives[TY_I64],
                                        .long_val = i
                                    }
                                },
                                .right = node->right->args->items[i]
                            };  
                            asm_gen_expr(cg, &converted);
                        }
                        return;
                    default:
                        break;
                }
            }
            asm_gen_addr(cg, node->left);
            asm_push(cg);
            asm_gen_expr(cg, node->right);
            asm_store(cg, node->left->data_type);
            return;

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
            u64 c = asm_count();
            asm_gen_expr(cg, node->left);
            asm_cmp_zero(cg, node->left->data_type);
            asm_println(cg, "  je .L.false.%ld", c);
            asm_gen_expr(cg, node->right);
            asm_cmp_zero(cg, node->right->data_type);
            asm_println(cg, "  je .L.false.%ld", c);
            asm_println(cg, "  mov $1, %%rax");
            asm_println(cg, "  jmp .L.end.%ld", c);
            asm_println(cg, ".L.false.%ld:", c);
            asm_println(cg, "  mov $0, %%rax");
            asm_println(cg, ".L.end.%ld:", c);
        } return;

        case ND_OR:
        {
            u64 c = asm_count();
            asm_gen_expr(cg, node->left);
            asm_cmp_zero(cg, node->left->data_type);
            asm_println(cg, "  jne .L.true.%ld", c);
            asm_gen_expr(cg, node->right);
            asm_cmp_zero(cg, node->right->data_type);
            asm_println(cg, "  jne .L.true.%ld", c);
            asm_println(cg, "  mov $1, %%rax");
            asm_println(cg, "  jmp .L.end.%ld", c);
            asm_println(cg, ".L.true.%ld:", c);
            asm_println(cg, "  mov $1, %%rax");
            asm_println(cg, ".L.end.%ld:", c);
        } return;

        case ND_CALL:
        {
            i32 stack_args = asm_push_args(cg, node);
            asm_gen_addr(cg, node);

            i32 gp = 0, fp = 0;

            if(node->data_type->kind != TY_VOID && node->data_type->size > 16)
                asm_pop(cg, argreg64[gp++]);
            
            for(i64 i = 0; i < node->args->size; i++)
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

                        if(fp + fp1 + fp2 < FP_MAX && gp + !fp1 + !fp2 < GP_MAX)
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
                        if(fp < FP_MAX)
                            asm_popf(cg, fp++);
                        break;
                    case TY_F80:
                        break;
                    default:
                        if(gp < GP_MAX)
                            asm_pop(cg, argreg64[gp++]);
                }
            }

            asm_println(cg, "  mov %%rax, %%r10");
            asm_println(cg, "  mov $%d, %%rax", fp);
            asm_println(cg, "  call *%%r10");
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
                    asm_println(cg, "  movswl %%al, %%eax");
                    return;
                case TY_U16:
                    asm_println(cg, "  movzwl %%al, %%eax");
                    return;
            }

            // If the return type is a small struct, a value is returned
		    // using up to two registers.
            if(node->return_buffer && node->data_type->size <= 16) {
                asm_copy_ret_buffer(cg, node->return_buffer);
                asm_println(cg, "  lea %d(%%rbp), %%rax", node->return_buffer->offset);
            }
        } return;
    }

    switch(node->left->data_type->kind)
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
                    asm_println(cg, "  seta %%ak");
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
            }

            LOG_ERROR_F("unhandled expression (%d)\n", node->kind);
            return;
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
                asm_println(cg, "  mov $0, %s", dx);
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
    }

    LOG_ERROR_F("unhandled expression (%d)", node->kind);
}

static void asm_init_zero(ASMCodegenData_T* cg, ASTObj_T* var)
{
    asm_println(cg, "  mov $%d, %%rcx", var->data_type->size);
    asm_println(cg, "  lea %d(%%rbp), %%rdi", var->offset);
    asm_println(cg, "  mov $0, %%al");
    asm_println(cg, "  rep stosb");
}

static void asm_gen_stmt(ASMCodegenData_T* cg, ASTNode_T* node)
{
    if(node->tok && cg->embed_file_locations)
        asm_println(cg, "  .loc %d %d", node->tok->source->file_no + 1, node->tok->line + 1);

    switch(node->kind)
    {
        case ND_NOOP:
            return;
        case ND_IF:
        {
            u64 c = asm_count();
            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  je  .L.else.%ld", c);
            asm_gen_stmt(cg, node->if_branch);
            asm_println(cg, "  jmp .L.end.%ld", c);
            asm_println(cg, ".L.else.%ld:", c);
            if(node->else_branch)
                asm_gen_stmt(cg, node->else_branch);
            asm_println(cg, ".L.end.%ld:", c);
        } return;

        case ND_FOR:
        {
            u64 c = asm_count();
            if(node->init_stmt)
                asm_gen_stmt(cg, node->init_stmt);
            asm_println(cg, ".L.begin.%ld:", c);
            if(node->condition) {
                asm_gen_expr(cg, node->condition);
                asm_cmp_zero(cg, node->condition->data_type);
                asm_println(cg, "  je .L.break.%ld", c);
            }
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.continue.%ld:", c);
            if(node->expr)
                asm_gen_expr(cg, node->expr);
            asm_println(cg, "  jmp .L.begin.%ld", c);
            asm_println(cg, ".L.break.%ld:", c);
        } return;
    
        case ND_WHILE:
        {
            u64 c = asm_count();
            asm_println(cg, ".L.begin.%ld:", c);
            asm_gen_expr(cg, node->condition);
            asm_cmp_zero(cg, node->condition->data_type);
            asm_println(cg, "  je .L.break.%ld", c);
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.continue.%ld:", c);
            asm_println(cg, "  jmp .L.begin.%ld", c);
            asm_println(cg, ".L.break.%ld:", c);
        } return;

        case ND_LOOP:
        {
            u64 c = asm_count();
            asm_println(cg, ".L.begin.%ld:", c);
            asm_gen_stmt(cg, node->body);
            asm_println(cg, ".L.continue.%ld:", c);
            asm_println(cg, "  jmp .L.begin.%ld", c);
            asm_println(cg, ".L.break.%ld:", c);
        } return;
    
        case ND_MATCH:
            {
                asm_gen_expr(cg, node->condition);
                //asm_println(cg, "  mov %%rax, %%rdi");
                asm_count();

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
                    //asm_println(cg, "  mov %s, %s", ax, di);
                    asm_println(cg, "  cmp %s, %s", ax, di);
                    asm_pop(cg, "%rax");
                    asm_println(cg, "  je .L.case.%ld.%ld", i, asm_current_count());
                }

                if(node->default_case) 
                {
                    node->default_case->long_val = node->cases->size;
                    asm_println(cg, "  jmp .L.case.%ld.%ld", node->cases->size, asm_current_count());
                }
                for(size_t i = 0; i < node->cases->size; i++)
                    asm_gen_stmt(cg, node->cases->items[i]);
                if(node->default_case)
                    asm_gen_stmt(cg, node->default_case);
                
                asm_println(cg, ".L.break.%ld:", asm_current_count());
            } return;
        
        case ND_CASE:
            asm_println(cg, ".L.case.%ld.%ld:", node->long_val, asm_current_count());
            asm_gen_stmt(cg, node->body);
            asm_println(cg, "  jmp .L.break.%ld", asm_current_count());
            return;
        
        case ND_BLOCK:
            for(size_t i = 0; i < node->locals->size; i++)
                asm_init_zero(cg, node->locals->items[i]);

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
                        asm_copy_struct_mem(cg);
                }
            }

            asm_println(cg, "  jmp .L.return.%s", cg->current_fn_name);
            return;
        
        case ND_EXPR_STMT:
            asm_gen_expr(cg, node->expr);
            return;
        
        case ND_CONTINUE:
            asm_println(cg, "  jmp .L.continue.%ld", asm_current_count());
            return;

        case ND_BREAK:
            asm_println(cg, "  jmp .L.break.%ld", asm_current_count());
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
