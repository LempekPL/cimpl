#ifndef PARSER_H
#define PARSER_H
#include "token.h"

typedef enum {
    STMT_FUNC,
    STMT_CONST
} StmtType;

typedef struct {
    StmtType type;
    union {

    } value;
} Stmt;

typedef enum {
    ITEM_FUNC,
    ITEM_CONST
} ItemType;

typedef struct {
    char* name;
    Stmt* stmts;
} ItemFunc;

typedef struct {

} ItemConst;

typedef struct {
    ItemType type;
    union {
        ItemFunc fn;
        ItemConst cst;
    } value;
} Item;

typedef struct {
    Item* items;
} Program;

Program parse(const char* filepath, const char* code, Token* tokens);

#endif // PARSER_H