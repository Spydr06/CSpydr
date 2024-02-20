#include "mangle.h"
#include "ast/ast.h"
#include "context.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

size_t num_special_chars(const char* callee)
{
    size_t num = 0;
    for(const char* c = callee; *c; c++)
        if(!isalnum(*c) && *c != '_')
            num++;
    return num;
}

#define SPECIAL_CHAR_MANGLED_SIZE 3

char* mangle_identifier(Context_T* context, ASTIdentifier_T* ident)
{
/*    size_t mangled_len = 0;
    for(ASTIdentifier_T* id = ident; id != NULL; id = id->outer)
        mangled_len += SPECIAL_CHAR_MANGLED_SIZE * 2 + strlen(id->callee) + num_special_chars(id->callee) * SPECIAL_CHAR_MANGLED_SIZE;

    char* mangled = malloc((mangled_len + 1) * sizeof(char));
    size_t i = 0;
    for(ASTIdentifier_T* id = ident; id != NULL; id = id->outer)
    {
        for(const char* c = id->callee; *c; c++)
            if(isalnum(*c) || *c == '_')
                mangled[i++] = *c;
            else
            {
                snprintf(mangled + i, SPECIAL_CHAR_MANGLED_SIZE, "$%02x", (int) *c);
                i += SPECIAL_CHAR_MANGLED_SIZE;
            }
        
        if(id->outer)
        {
            snprintf(mangled + i, SPECIAL_CHAR_MANGLED_SIZE * 2, "$%02x$%02x", (int) ':', (int) ':');
            i += SPECIAL_CHAR_MANGLED_SIZE * 2;
        }
    }
    mangled[i] = '\0';
    
    CONTEXT_ALLOC_REGISTER(context, (void*) mangled);

    return mangled;*/
    return ident->callee;
}
