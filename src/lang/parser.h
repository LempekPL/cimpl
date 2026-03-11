#ifndef PARSER_H
#define PARSER_H
#include "token.h"

typedef enum {
    EXPR_LITERAL_INTEGER,
    EXPR_IDENTIFIER,
    EXPR_BINOP,
    EXPR_CALL
} ExprType;

typedef struct {
    char* name;
    // TODO: args
} ExprCall;

typedef struct {
    Token op;
    struct expr_t* left;
    struct expr_t* right;
} ExprBinOp;

typedef struct expr_t {
    ExprType type;
    union {
        size_t integer;
        ExprBinOp binop;
        char* ident;
        ExprCall call;
    } value;
} Expr;

typedef enum {
    STMT_DECLARE,
    STMT_ASSIGN
} StmtType;

typedef struct {
    char* name;
    Expr* expr;
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