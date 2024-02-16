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

typedef struct LOCALS_INFO_STRUCT {
    u32 local_id; 
} LocalsInfo_T;

typedef struct NORMALIZER_STRUCT {
    Context_T* context;
    IR_T* ir;
    List_T* types;

    u64 label_id;
    LocalsInfo_T locals_info;
    LoopContext_T loop_context;
} Normalizer_T;

int normalization_pass(Context_T* context, ASTProg_T* ast, IR_T* ir);

#endif
    
