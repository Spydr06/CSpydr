#include "mangle.h"
#include "ast/ast.h"
#include "context.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SPECIAL_CHAR_MANGLED_SIZE 3
#define STATIC_PREFIX_SIZE (sizeof(MANGLE_STATIC_PREFIX) - sizeof(char))

static size_t num_special_chars(const char* callee)
{
    size_t num = 0;
    for(const char* c = callee; *c; c++)
        if(!isalnum(*c) && *c != '_')
            num++;
    return num;
}

void free_mangled_idents(List_T** mangled_idents)
{
    for(size_t i = 0; i < (*mangled_idents)->size; i++) 
        free((*mangled_idents)->items[i]);
    free_list(*mangled_idents);
    *mangled_idents = NULL;
}

const char* mangle_identifier(Normalizer_T* n, ASTIdentifier_T* ident, ASTObj_T* backing) 
{
    assert(backing != NULL);

    for(size_t i = 0; i < n->mangled_idents->size; i++) 
    {
        MangledIdentifier_T* cached = n->mangled_idents->items[i];
        if(cached->backing == backing)
            return cached->mangled_ident;
    }

    if(backing->exported)
    {
        MangledIdentifier_T* cached = malloc(sizeof(struct MANGLED_IDENTIFIER_STRUCT));
        cached->backing = backing;
        cached->mangled_ident = backing->exported;
        list_push(n->mangled_idents, cached);
        return backing->exported;
    }

    size_t mangled_len = STATIC_PREFIX_SIZE + 1;
    for(ASTIdentifier_T* id = ident; id; id = id->outer)
        mangled_len += SPECIAL_CHAR_MANGLED_SIZE * 2 + strlen(id->callee) + num_special_chars(id->callee) * SPECIAL_CHAR_MANGLED_SIZE ;

    char* mangled = malloc((mangled_len + 1) * sizeof(char));
    CONTEXT_ALLOC_REGISTER(n->context, (void*) mangled);

    memcpy(mangled, MANGLE_STATIC_PREFIX, STATIC_PREFIX_SIZE);
    size_t i = STATIC_PREFIX_SIZE;

    for(ASTIdentifier_T* id = ident; id; id = id->outer)
    {
        for(const char* c = id->callee; *c; c++) 
        {
            if(isalnum(*c) || *c == '_')
                mangled[i++] = *c;
            else {
                snprintf(mangled + i, SPECIAL_CHAR_MANGLED_SIZE + 1, "$%02hhx", *c);
                i += SPECIAL_CHAR_MANGLED_SIZE;
            }
        }

        if(id->outer) {
            snprintf(mangled + i, SPECIAL_CHAR_MANGLED_SIZE * 2 + 1, "$%02hhx$%02hhx", ':', ':');
            i += SPECIAL_CHAR_MANGLED_SIZE * 2;
        }
    }

    mangled[i] = '\0';

    MangledIdentifier_T* cached = malloc(sizeof(struct MANGLED_IDENTIFIER_STRUCT));
    cached->backing = backing;
    cached->mangled_ident = mangled;
    list_push(n->mangled_idents, cached);

    return mangled;
}

