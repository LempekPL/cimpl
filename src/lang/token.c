#include "token.h"
#include "../vec.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define LIST_OF_KW \
    RUN_KW("fn", TOKEN_KW_FN) \
    RUN_KW("let", TOKEN_KW_LET) \
    RUN_KW("if", TOKEN_KW_IF)

// slash is done manually because of comments
#define CASE_LIST_OF_TOKENS \
    do_wide_token('+', TOKEN_PLUS, '=', TOKEN_PLUSEQUALS) \
    do_wide_token('-', TOKEN_MINUS, '=', TOKEN_MINUSEQUALS) \
    do_wide_token('*', TOKEN_STAR, '=', TOKEN_STAREQUALS) \
    do_wide_token('%', TOKEN_PERCENT, '=', TOKEN_PERCENTEQUALS) \
    do_wide_token('=', TOKEN_EQUALS, '=', TOKEN_EQUALSEQUALS) \
    do_wide_token('>', TOKEN_GREATER, '=', TOKEN_GREATEREQUALS) \
    do_wide_token('<', TOKEN_LESSER, '=', TOKEN_LESSEREQUALS) \
    do_wide_token('&', TOKEN_BITAND, '&', TOKEN_AND) \
    do_wide_token('|', TOKEN_BITOR, '|', TOKEN_OR) \
    do_token('.', TOKEN_DOT) \
    do_token(',', TOKEN_COMMA) \
    do_token(':', TOKEN_COLON) \
    do_token(';', TOKEN_SEMICOLON) \
    do_token('(', TOKEN_LPAREN) \
    do_token(')', TOKEN_RPAREN) \
    do_token('{', TOKEN_LBRACE) \
    do_token('}', TOKEN_RBRACE) \
    do_token('[', TOKEN_LBRACK) \
    do_token(']', TOKEN_RBRACK)
    
#define RUN_KW(str, token_type) if (length == (sizeof(str) - 1) && strncmp(text, str, length) == 0) { return token_type; }

TokenType check_keyword(const char* text, size_t length) {
    LIST_OF_KW
    return TOKEN_IDENTIFIER;
}

bool read_string(Token** tokens, const char* code, size_t* current, size_t* column, size_t* line) {
    size_t start_col = *column;
    size_t start_line = *line;
    (*current)++;
    (*column)++;
    const char* start_ptr = &code[*current];
    size_t length = 0;

    while (code[*current] != '"') {
        if (code[*current] == '\0') return false;
        if (code[*current] == '\n') {
            (*line)++;
            *column = 1;
        } else (*column)++;
        (*current)++;
        length++;
    }
    (*current)++;
    (*column)++;

    Token t = {0};
    t.type = TOKEN_STRING; 
    t.value.string.text = start_ptr; 
    t.value.string.length = length;
    t.start.line = start_line;
    t.start.column = start_col;
    t.end.line = *line;
    t.end.column = *column;
    
    vec_push(*tokens, t);

    return true;
}

void read_ident(Token** tokens, const char* code, size_t* current, size_t* column, size_t line) {
    size_t start_col = *column;
    const char* start_ptr = &code[*current];
    size_t length = 0;

    while (isalnum(code[*current])) {
        (*current)++;
        (*column)++;
        length++;
    }

    Token t = {0};
    t.type = check_keyword(start_ptr, length); 
    t.value.string.text = start_ptr; 
    t.value.string.length = length;
    t.start.line = line;
    t.start.column = start_col;
    t.end.line = line;
    t.end.column = *column;
    
    vec_push(*tokens, t);
}

typedef enum {
    B32,
    Dec,
    Hex,
    Oct,
    Bin
} NumberType;

void read_number(Token** tokens, const char* code, size_t* current, size_t* column, size_t line) {
    size_t start_col = *column;
    char* number = NULL;
    NumberType nt = Dec;

    if (code[*current] == '0') {
        (*current)++;
        switch (code[*current]) {
            case 'b':
                nt = Bin;
                break;
            case 'd':
                nt = Dec;
                break;
            case 'o':
                nt = Oct;
                break;
            case 'x':
                nt = Hex;
                break;
            case 'z':
                nt = B32;
                break;
        }
        if (!isdigit(code[*current])) (*current)++;
    }

    while (isdigit(code[*current])) {
        vec_push(number, code[*current]);
        (*current)++;
        (*column)++;
    }
    vec_push(number, '\0');

    Token t = {0};
    t.type = TOKEN_INTEGER; 

    switch (nt) {
        case Bin:
            t.value.integer = strtol(number, NULL, 2);
            break;
        case Oct:
            t.value.integer = strtol(number, NULL, 8);
            break;
        case Hex:
            t.value.integer = strtol(number, NULL, 16);
            break;
        case B32:
            t.value.integer = strtol(number, NULL, 32);
            break;
        default:
            t.value.integer = strtol(number, NULL, 10);
            break;    
    }

    t.start.line = line;
    t.start.column = start_col;
    t.end.line = line;
    t.end.column = *column;
    
    vec_free(number);
    vec_push(*tokens, t);
}

#define make_token(token_type) \
    do { \
        Token t = {0}; \
        t.type = (token_type); \
        t.start.column = column; \
        t.start.line = line; \
        t.end.column = column; \
        t.end.line = line; \
        vec_push(tokens, t); \
    } while(0) 

#define do_token(token, token_type) \
    case (token): \
        make_token(token_type); \
        break;

#define make_token_wide(token_type) \
    do { \
        Token t = {0}; \
        t.type = (token_type); \
        t.start.column = column; \
        t.start.line = line; \
        t.end.column = ++column; \
        t.end.line = line; \
        vec_push(tokens, t); \
    } while(0) 

#define do_wide_token(token1, token_type1, token2, token_type2) \
    case (token1): \
        if (code[current+1] == (token2)) { \
            make_token_wide(token_type2); \
            current++; \
        } else \
            make_token(token_type1); \
        break;

#define wide_case_token(token1, token_type1, token2, token_type2) do_wide_token(token1, token_type1, token2, token_type2)

void token_cleanup(Token** tokens) {
    vec_free(*tokens);
}

Option tokenize(const char* filepath, const char* code) {
    if (code == NULL) return OptionNone;
    __attribute__((cleanup(token_cleanup))) 
    Token* tokens = NULL;
    size_t line = 1;
    size_t column = 1;
    size_t current = 0;

    while (code[current] != '\0') {
        char c_char = code[current];

        if (isspace(c_char)) {
            if (c_char == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            current++;
            continue;
        }

        if (isalpha(c_char)) {
            read_ident(&tokens, code, &current, &column, line);
            continue; 
        }

        if (isdigit(c_char)) {
            read_number(&tokens, code, &current, &column, line);
            continue; 
        }
        
        if (c_char == '"') {
            size_t s_col = column;
            size_t s_line = line;
            if (!read_string(&tokens, code, &current, &column, &line)) {
                print_err("Missing terminating \" character at %y\n", filepath, s_line, s_col);
                return OptionNone;
            }
            continue;
        }

        switch (c_char) {
            case '/': 
                if (code[current+1] == '=') {
                    make_token_wide(TOKEN_SLASHEQUALS);
                    current++;
                } else if (code[current+1] == '/') {
                    while (code[current] != '\n' && code[current] != '\0') current++;
                    current--;
                    line++;
                    column = 1;
                } else if (code[current+1] == '*') {
                    while (code[current] != '\0') {
                        column++;
                        if (code[current] == '\n') {
                            line++;
                            column = 1;
                        }
                        if (code[current] == '*' && code[current+1] == '/') {
                            current++;
                            break;
                        }
                        current++;
                    }
                } else
                    make_token(TOKEN_SLASH);
                break;
            CASE_LIST_OF_TOKENS
            default:
                print_err("Unknown character `%c` at %y\n", c_char, filepath, line, column);
                return OptionNone;
        }
        current++;
        column++;
    }

    make_token(TOKEN_EOF);
    Token* take = tokens;
    tokens = NULL;
    return OptionSome(take);
}

#undef do_token
#define do_token(token, token_type) \
    case (token_type): \
        fprintf(stream, "%c", token); \
        break;

#undef do_wide_token
#define do_wide_token(token1, token_type1, token2, token_type2) \
    case (token_type1): \
        fprintf(stream, "%c", (token1)); \
        break; \
    case (token_type2): \
        fprintf(stream, "%c%c", (token1), (token2)); \
        break;

#undef RUN_KW
#define RUN_KW(str, token_type) \
    case (token_type): \
        fprintf(stream, "%s", (str)); \
        break;

void fprint_token_type(FILE* stream, const TokenType tt) {
    switch (tt) {
        case TOKEN_IDENTIFIER:
            fprintf(stream, "ident");
            break;
        case TOKEN_STRING:
            fprintf(stream, "string");
            break;
        case TOKEN_INTEGER:
            fprintf(stream, "integer");
            break;
        case TOKEN_DECIMAL:
            fprintf(stream, "decimal");
            break;
        do_wide_token('/', TOKEN_SLASH, '=', TOKEN_SLASHEQUALS)
        LIST_OF_KW
        CASE_LIST_OF_TOKENS
        default:
            fprintf(stream, "unknown");
            break;
    }
}

#undef do_token
#define do_token(token, token_type) \
    case (token_type): \
        sprintf(buffer, "%c", token); \
        break;

#undef do_wide_token
#define do_wide_token(token1, token_type1, token2, token_type2) \
    case (token_type1): \
        sprintf(buffer, "%c", (token1)); \
        break; \
    case (token_type2): \
        sprintf(buffer, "%c%c", (token1), (token2)); \
        break;

#undef RUN_KW
#define RUN_KW(str, token_type) \
    case (token_type): \
        sprintf(buffer, "%s", (str)); \
        break;

void sprint_token_type(char* buffer, const TokenType tt) {
    switch (tt) {
        case TOKEN_IDENTIFIER:
            sprintf(buffer, "ident");
            break;
        case TOKEN_STRING:
            sprintf(buffer, "string");
            break;
        case TOKEN_INTEGER:
            sprintf(buffer, "integer");
            break;
        case TOKEN_DECIMAL:
            sprintf(buffer, "decimal");
            break;
        do_wide_token('/', TOKEN_SLASH, '=', TOKEN_SLASHEQUALS)
        LIST_OF_KW
        CASE_LIST_OF_TOKENS
        default:
            sprintf(buffer, "unknown");
            break;
    }
}

void print_token_type(const TokenType tt) {
    fprint_token_type(stdout, tt);
}

void dbg_token(const Token token) {
    printf("[%zu:%zu] ", token.start.line, token.start.column);
    print_token_type(token.type);
    printf("\n");
}

void fprint_token(FILE* stream, const Token token) {
    if (token.type == TOKEN_IDENTIFIER) {
        fprintf(stream, "%.*s", (int)token.value.string.length, token.value.string.text);
    } else if (token.type == TOKEN_INTEGER) {
        fprintf(stream, "%zu", token.value.integer);
    } else if (token.type == TOKEN_STRING) {
        fprintf(stream, "\"%.*s\"", (int)token.value.string.length, token.value.string.text);
    } else if (token.type == TOKEN_EOF) {} else {
        fprint_token_type(stream, token.type);
    }
}

void sprint_token(char* buffer, const Token token) {
    if (token.type == TOKEN_IDENTIFIER) {
        sprintf(buffer, "%.*s", (int)token.value.string.length, token.value.string.text);
    } else if (token.type == TOKEN_INTEGER) {
        sprintf(buffer, "%zu", token.value.integer);
    } else if (token.type == TOKEN_STRING) {
        sprintf(buffer, "\"%.*s\"", (int)token.value.string.length, token.value.string.text);
    } else if (token.type == TOKEN_EOF) {} else {
        sprint_token_type(buffer, token.type);
    }
}

void print_token(const Token token) {
    fprint_token(stdout, token);
}

void pretty_print_tokens(const Token* tokens) {
    size_t tab_count = 0;
    for (size_t i = 0; i < vec_len(tokens); i++) {
        if (tokens[i].type == TOKEN_EOF) break;
        if (tokens[i].type == TOKEN_RBRACE) tab_count--;
        if (tokens[i].type == TOKEN_LBRACE) tab_count++;
        print_token(tokens[i]);
        if (i+1 > vec_len(tokens)) continue;
        if (
            (tokens[i].type == TOKEN_LBRACE && tokens[i+1].type != TOKEN_RBRACE) ||
            (tokens[i].type == TOKEN_RBRACE) ||
            (tokens[i].type == TOKEN_SEMICOLON)
        ) {
            printf("\n");
            for (size_t j = tokens[i+1].type == TOKEN_RBRACE; j < tab_count; j++) printf("\t");
            continue;
        }
        if (
            (tokens[i].type == TOKEN_IDENTIFIER && tokens[i+1].type == TOKEN_LPAREN) ||
            (tokens[i].type == TOKEN_LPAREN && tokens[i+1].type == TOKEN_RPAREN) ||
            (tokens[i+1].type == TOKEN_SEMICOLON) ||
            (tokens[i+1].type == TOKEN_STRING) ||
            (tokens[i].type == TOKEN_STRING)
        ) continue;
        printf(" ");
    }
    printf("\n");
    // for (size_t i = 0; i < vec_len(tokens); i++) {
    //     dbg_token(tokens[i]);
    // }
}