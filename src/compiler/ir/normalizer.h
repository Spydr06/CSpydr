#ifndef CSPYDR_IR_NORMALIZER_H
#define CSPYDR_IR_NORMALIZER_H

#include "ast/ast.h"
#include "config.h"
#include "context.h"
#include "ir/ir.h"
#include "list.h"
#include "util.h"

typedef struct LOOP_CONTEXT_STRUCT {
    u32 break_label;
    u32 continue_label;
} LoopContext_T;


typedef struct FUNC_CONTEXT_STRUCT {
    u32 register_id;
} FunctionContext_T;

typedef struct NORMALIZER_STRUCT {
    Context_T* context;
    IR_T* ir;
    List_T* type_pairs;

    uint64_t label_id;
    LoopContext_T loop_context;
    FunctionContext_T func_context;
} Normalizer_T;

int normalization_pass(Context_T* context, ASTProg_T* ast, IR_T* ir);

#endif
    
