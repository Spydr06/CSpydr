#include "token.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

token_T* initToken(char* value, tokenType_T type)
{
    token_T* token = calloc(1, sizeof(struct TOKEN_STRUCT));
    token->value = value;
    token->type = type;

    return token;
}

const char* tokenTypeToString(tokenType_T type)
{
    switch(type) {
        case TOKEN_ID: return "TOKEN_ID";            
        case TOKEN_LEFT_PAREN: return "TOKEN_LEFT_PAREN";    
        case TOKEN_RIGHT_PAREN: return "TOKEN_RIGHT_PAREN";   
        case TOKEN_LEFT_BRACE: return "TOKEN_LEFT_BRACE";    
        case TOKEN_RIGHT_BRACE: return "TOKEN_RIGHT_BRACE";   
        case TOKEN_LEFT_BRACKET: return "TOKEN_LEFT_BRACKET";  
        case TOKEN_RIGHT_BRACKET: return "TOKEN_RIGHT_BRACKET"; 
        case TOKEN_EQUAL: return "TOKEN_EQUAL";         
        case TOKEN_EQUAL_EQUAL: return "TOKEN_EQUAL_EQUAL";   
        case TOKEN_COLON: return "TOKEN_COLON";         
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";     
        case TOKEN_DOT: return "TOKEN_DOT";          
        case TOKEN_COMMA: return "TOKEN_COMMA"; 
        case TOKEN_GREATER: return "TOKEN_GREATER";       
        case TOKEN_GREATER_EQUALS: return "TOKEN_GREATER_EQUALS";
        case TOKEN_GREATER_GREATER: return "TOKEN_GREATER_GREATER";
        case TOKEN_LESS: return "TOKEN_LESS";          
        case TOKEN_LESS_EQUALS: return "TOKEN_LESS_EQUALS";   
        case TOKEN_LESS_LESS: return "TOKEN_LESS_LESS";     
        case TOKEN_PLUS: return "TOKEN_PLUS";          
        case TOKEN_PLUS_EQUALS: return "TOKEN_PLUS_EQUALS";   
        case TOKEN_MINUS: return "TOKEN_MINUS";         
        case TOKEN_MINUS_EQUALS: return "TOKEN_MINUS_EQUALS";  
        case TOKEN_STAR: return "TOKEN_STAR";          
        case TOKEN_STAR_EQUALS: return "TOKEN_STAR_EQUALS";   
        case TOKEN_PERCENT: return "TOKEN_PERCENT";       
        case TOKEN_PERCENT_EQUALS: return "TOKEN_PERCENT_EQUALS";
        case TOKEN_SLASH: return "TOKEN_SLASH";         
        case TOKEN_SLASH_EQUALS: return "TOKEN_SLASH_EQUALS";  
        case TOKEN_BANG: return "TOKEN_BANG";          
        case TOKEN_BANG_EQUALS: return "TOKEN_BANG_EQUALS";   
        case TOKEN_AND: return "TOKEN_AND";           
        case TOKEN_AND_AND: return "TOKEN_AND_AND";       
        case TOKEN_AND_EQUALS: return "TOKEN_AND_EQUALS";    
        case TOKEN_OR: return "TOKEN_OR";            
        case TOKEN_OR_OR: return "TOKEN_OR_OR";         
        case TOKEN_OR_EQUALS: return "TOKEN_OR_EQUALS";     
        case TOKEN_STR: return "TOKEN_STR";           
        case TOKEN_NUMBER: return "TOKEN_NUMBER";        
        case TOKEN_TRUE: return "TOKEN_TRUE";          
        case TOKEN_FALSE: return "TOKEN_FALSE";         
        case TOKEN_LET: return "TOKEN_LET";           
        case TOKEN_INT: return "TOKEN_INT";           
        case TOKEN_FN: return "TOKEN_FN";            
        case TOKEN_VEC: return "TOKEN_VEC";           
        case TOKEN_STRING: return "TOKEN_STRING";        
        case TOKEN_FLOAT: return "TOKEN_FLOAT";         
        case TOKEN_BOOL: return "TOKEN_BOOL";          
        case TOKEN_STMT: return "TOKEN_STMT";
        case TOKEN_EOF: return "TOKEN_EOF";      
   }

   return "notStringable";
}

char* tokenToString(token_T* token)
{
    const char* typeStr = tokenTypeToString(token->type);
    const char* template = "[type=%s: int type=%d: value='%s']";

    char* str = calloc(strlen(typeStr) + strlen(template) + 8, sizeof(char));
    sprintf(str, template, typeStr, token->type, token->value);

    return str;
}