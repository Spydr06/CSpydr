#include "../backend.h"

#include "codegen/codegen.h"
#include "error/error.h"
#include "ir/ir.h"
#include "list.h"
#include "platform/linux/linux_platform.h"
#include "io/log.h"

#include <stdint.h>

#define _(x) x
#define P(...) codegen_printf(c, __VA_ARGS__)
#define P_ln(...) codegen_println(c, __VA_ARGS__)

struct BACKEND_DATA_STRUCT {
    List_T* generated_types;
};

static size_t C99_get_type_id(CodegenData_T* c, IRType_T* type)
{
    // TODO: optimize this
    for(size_t i = 0; i < c->ir->types->size; i++)
        if(c->ir->types->items[i] == type)
            return i;
    unreachable();
    return SIZE_MAX;
}

static void C99_generate_type(CodegenData_T* c, IRType_T* type)
{
#define PRIMITIVE(en, ct) case IR_TYPE_##en: P(#ct); break;

    switch(type->kind)
    {
        PRIMITIVE(I8, int8_t);
        PRIMITIVE(I16, int16_t);
        PRIMITIVE(I32, int32_t);
        PRIMITIVE(I64, int64_t);
        PRIMITIVE(U8, uint8_t);
        PRIMITIVE(U16, uint16_t);
        PRIMITIVE(U32, uint32_t);
        PRIMITIVE(U64, uint64_t);
        PRIMITIVE(F32, float);
        PRIMITIVE(F64, double);
        PRIMITIVE(F80, long double);
        PRIMITIVE(VOID, void);

        case IR_TYPE_STRUCT:
        case IR_TYPE_VLA:
        case IR_TYPE_ARRAY:
            P("struct _%zu", C99_get_type_id(c, type));
            break;
        case IR_TYPE_UNION:
            P("union _%zu", C99_get_type_id(c, type));
            break;
        case IR_TYPE_PTR:
            C99_generate_type(c, type->base);
            P("*");
            break;
        case IR_TYPE_C_ARRAY:
            P("_array_%zu", C99_get_type_id(c, type));
            break;
        case IR_TYPE_FN:
            P("_fn_%zu", C99_get_type_id(c, type));
            break;
        default:
            unreachable();

    }

#undef PRIMITIVE
}

static void C99_define_type(CodegenData_T* c, IRType_T* type, size_t i)
{
    if(list_contains(c->b_data->generated_types, type))
        return;

    switch(type->kind)
    {
    case IR_TYPE_STRUCT:
    case IR_TYPE_UNION:
        for(size_t i = 0; i < type->fields->size; i++)
        {
            IRTypeField_T* field = type->fields->items[i];
            C99_define_type(c, field->type, C99_get_type_id(c, field->type));
        }

        P_ln("%s _%zu {", type->kind == IR_TYPE_STRUCT ? "struct" : "union", i);
        for(size_t i = 0; i < type->fields->size; i++)
        {
            IRTypeField_T* field = type->fields->items[i];
            P("  ");
            C99_generate_type(c, field->type);
            P_ln(" %s;", field->id);
        }
        P_ln("};");
        break;
    case IR_TYPE_ARRAY:
    case IR_TYPE_VLA:
        C99_define_type(c, type->base, C99_get_type_id(c, type->base));
        P_ln("struct _%zu {", i);
        P_ln("  uint64_t len;");
        C99_generate_type(c, type->base);
        P_ln(" data[%zu]", type->kind == IR_TYPE_VLA ? 0 : type->indices);
        P_ln("};");
        break;
    case IR_TYPE_C_ARRAY:
        C99_define_type(c, type->base, C99_get_type_id(c, type->base));
        P("typedef ");
        C99_generate_type(c, type->base);
        P_ln(" _array_%zu[%zu];", i, type->indices);
        break;
    case IR_TYPE_FN:
        P("typedef ");
        C99_generate_type(c, type->base);
        P(" (* _fn_%zu)(", i);
        if(type->fn.arguments->size)
        {
            for(size_t i = 0; i < type->fn.arguments->size; i++)
            {
                C99_generate_type(c, type->fn.arguments->items[i]);
                if(type->fn.arguments->size - i > 1)
                    P(", ");
            }
            if(type->fn.variadic)
                P(", ...");
        }
        else if(type->fn.variadic)
            P("...");
        else
            P("void");
        P_ln(");");
        break;
    default:
        return;
    }

    list_push(c->b_data->generated_types, type);
}

static void C99_generate_literal(CodegenData_T* c, IRLiteral_T* lit)
{
    switch(lit->kind) {
        case IR_LITERAL_VOID:
            break;
        case IR_LITERAL_I8:
            P("((int8_t) %hhd)", lit->i8_lit);
            break;
        case IR_LITERAL_U8:
            P("((uint8_t) %hhu)", lit->u8_lit);
            break;
        case IR_LITERAL_I32:
            P("((int32_t) %d)", lit->i32_lit);
            break;
        case IR_LITERAL_I64:
            P("((int64_t) %ld)", lit->i64_lit);
            break;
        case IR_LITERAL_U64:
            P("((uint64_t) %lu)", lit->u64_lit);
            break;
        case IR_LITERAL_F32:
            P("%f", (double) lit->f32_lit);
            break;
        case IR_LITERAL_F64:
            P("%f", lit->f64_lit);
            break;
        case IR_LITERAL_REG:
            P("(&_%d)", lit->reg.id);
            break;
        case IR_LITERAL_DEREF:
            if(lit->deref.offset) 
            {
                P("(*(");
                C99_generate_type(c, lit->type);
                P("*) (((void*) &_%d) + %d))", lit->deref.reg.id, lit->deref.offset);
            }
            else
                P("_%d", lit->deref.reg.id);
            break;
        default:
            unreachable();
    }
}

static void C99_generate_lvalue(CodegenData_T* c, IRLValue_T* lvalue)
{
    switch(lvalue->kind) {
        default:
            break;
    }
}

static void C99_generate_local_decl(CodegenData_T* c, IRStmt_T* decl)
{
    P("  ");
    C99_generate_type(c, decl->decl.value.type);
    P(" _%d", decl->decl.reg.id);
    C99_generate_lvalue(c, &decl->decl.value);
    P_ln(";");
}

static void C99_generate_stmt(CodegenData_T* c, IRStmt_T* stmt)
{
    switch(stmt->kind)
    {
        case IR_STMT_RETURN:
            P("  return ");
            C99_generate_literal(c, &stmt->_return.lit);
            P_ln(";");
            break;
        case IR_STMT_DECL:
            C99_generate_local_decl(c, stmt);
            break;
        case IR_STMT_STORE_DEREF:
            if(stmt->store_deref.offset) 
            {
                P("  (*(");
                C99_generate_type(c, stmt->store_deref.value.type);
                P("*) (((void*) &_%d) + %d))", stmt->store_deref.location.id, stmt->store_deref.offset);
            }
            else
                P("  _%d", stmt->store_deref.location.id);
            P(" = ");
            C99_generate_literal(c, &stmt->store_deref.value);
            P_ln(";");
            break;
        case IR_STMT_LABEL:
            P("_L%u:", stmt->label.id);
            break;
        default:
    }
}

static void C99_generate_global_decl(CodegenData_T* c, IRGlobal_T* global)
{
    P("extern ");
    C99_generate_type(c, global->type);
    P_ln(" %s;", global->mangled_id);
}

static void C99_generate_global_definition(CodegenData_T* c, IRGlobal_T* global)
{
    if(global->is_extern)
        return;

    C99_generate_type(c, global->type);
    P(" %s = ", global->mangled_id);
    C99_generate_literal(c, &global->value);
    P_ln(";");
}

static void C99_generate_parameter(CodegenData_T* c, IRParameter_T* param)
{
    C99_generate_type(c, param->type);
    P(" _%d", param->reg.id);
}

static void C99_generate_function_signature(CodegenData_T* c, IRFunction_T* func)
{
    if(func->is_extern)
        P("extern ");

    C99_generate_type(c, func->return_type); 
    P(" %s(", func->mangled_id);

    if(func->params->size == 0)
        P("void");

    for(size_t i = 0; i < func->params->size; i++)
    {
        C99_generate_parameter(c, func->params->items[i]);
        if(i + 1 < func->params->size)
            P(", ");
    }

    if(func->variadic)
        P(", ...");

    P(")");
}

static void C99_generate_function_decl(CodegenData_T* c, IRFunction_T* func)
{
    C99_generate_function_signature(c, func);
    P_ln(";");
}

static void C99_generate_function_definition(CodegenData_T* c, IRFunction_T* func)
{
    if(func->is_extern)
        return;

    C99_generate_function_signature(c, func);
    P_ln(" {");

    for(size_t i = 0; i < func->stmts->size; i++)
        C99_generate_stmt(c, func->stmts->items[i]);

    P_ln("}");
}

static void C99_generate_ir(CodegenData_T* c, IR_T* ir)
{
    for(size_t i = 0; i < ir->globals->size; i++)
        C99_generate_global_decl(c, ir->globals->items[i]);

    for(size_t i = 0; i < ir->functions->size; i++)
        C99_generate_function_decl(c, ir->functions->items[i]);
    
    P_ln("");

    for(size_t i = 0; i < ir->globals->size; i++)
        C99_generate_global_definition(c, ir->globals->items[i]);

    for(size_t i = 0; i < ir->functions->size; i++)
        C99_generate_function_definition(c, ir->functions->items[i]);

    P_ln("");
}

static void C99_begin_file(CodegenData_T* c)
{
    c->b_data = calloc(1, sizeof(struct BACKEND_DATA_STRUCT));
    c->b_data->generated_types = init_list();

    P_ln(_(
"// generated by cspc\n\
#include <stdint.h>\n\
\n"));

    for(size_t i = 0; i < c->ir->types->size; i++)
        C99_define_type(c, c->ir->types->items[i], i); 
}

static void C99_finish_file(CodegenData_T* c)
{
    P_ln(_(
"int main(int argc, char** argv)\n\
{\n\
    return __csp_main((int32_t) argc, (int8_t**) argv);\n\
}"));

    free_list(c->b_data->generated_types);
    free(c->b_data);
}

static void C99_compile(CodegenData_T* c, const char* input_file, const char* output_file)
{
    List_T* args = init_list(); 

    list_push(args, c->context->cc);
    list_push(args, "-c");
    list_push(args, (void*) input_file);
    list_push(args, "-std=c99");
    list_push(args, "-o");
    list_push(args, (void*) output_file);

    if(c->generate_debug_info)
        list_push(args, "-g");

    for(size_t i = 0; i < c->context->compiler_flags->size; i++)
        list_push(args, c->context->compiler_flags->items[i]);

    if(c->context->flags.optimize)
        list_push(args, "-O2"); // -O3 is known to cause issues

    list_push(args, NULL);

    i32 exit_code = subprocess(args->items[0], (char* const*) args->items, false);
    free_list(args);

    if(exit_code != 0)
    {
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " C compiler terminated with non-zero exit code %d.\n" COLOR_RESET, exit_code);
        throw(c->context->main_error_exception);
    }
}

BACKEND_CALLBACKS_IMPL_EXT(C99);
