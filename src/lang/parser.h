#ifndef PARSER_H
#define PARSER_H
#include "token.h"

typedef enum {
    EXPR_LITERAL_INTEGER,
    EXPR_BINOP
} ExprType;

typedef struct {
    ExprType type;
    union {
        size_t integer;
    } value;
} Expr;

typedef enum {
    STMT_DECLARE,
    STMT_ASSIGN
} StmtType;

typedef struct {
    char* name;
    Expr expr;
} StmtDecl;

typedef struct {
    StmtType type;
    union {
        StmtDecl decl;
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

Option parse(const char* filepath, const char* code, const Token* tokens);

void print_program(Program* prog);

#endif // PARSER_H