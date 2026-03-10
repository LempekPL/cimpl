#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "util.h"

typedef enum {
    // token characters
    TOKEN_PLUS,      
    TOKEN_PLUSEQUALS,
    TOKEN_MINUS,     
    TOKEN_MINUSEQUALS,
    TOKEN_STAR,      
    TOKEN_STAREQUALS,
    TOKEN_SLASH,     
    TOKEN_SLASHEQUALS,
    TOKEN_PERCENT,     
    TOKEN_PERCENTEQUALS,
    TOKEN_EQUALS,    
    TOKEN_EQUALSEQUALS,
    TOKEN_GREATER,
    TOKEN_GREATEREQUALS,
    TOKEN_LESSER,
    TOKEN_LESSEREQUALS,
    TOKEN_BITAND,
    TOKEN_AND,
    TOKEN_BITOR,
    TOKEN_OR,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,    
    TOKEN_RPAREN,    
    TOKEN_LBRACK,    
    TOKEN_RBRACK,    
    TOKEN_LBRACE,    
    TOKEN_RBRACE,    
    
    // keywords
    TOKEN_KW_FN,
    TOKEN_KW_LET,
    TOKEN_KW_IF,
    
    // special with '.value'
    TOKEN_IDENTIFIER, 
    TOKEN_INTEGER,
    TOKEN_DECIMAL,
    TOKEN_STRING,

    TOKEN_EOF
} TokenType;

typedef struct {
    size_t line;
    size_t column;
} Pos;

typedef struct {
    const char* text;
    size_t length;
} StringView;

typedef struct {
    TokenType type;

    union {
        size_t integer;
        double decimal;
        StringView string;
    } value;

    Pos start;
    Pos end;
} Token;

Option tokenize(const char*, const char*);
void print_token_type(const TokenType tt);
void fprint_token_type(FILE* stream, const TokenType tt);
void sprint_token_type(char* buffer, const TokenType tt);
void print_token(const Token token);
void fprint_token(FILE* stream, const Token token);
void sprint_token(char* buffer, const Token token);
void pretty_print_tokens(const Token* tokens);

#endif // TOKEN_H