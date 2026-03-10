#include "parser.h"
#include "token.h"
#include "../vec.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define inc_ppd (*ppd->current)++
#define dec_ppd (*ppd->current)--
#define current_ppd ppd->tokens[*ppd->current]
#define ppd_printable(token) ppd->filepath, token.start.line, token.start.column

typedef struct {
    const char* filepath;
    const char* code;
    const Token* tokens;
    size_t* current;
} ProgramParseData;

bool consume_token(ProgramParseData* ppd, TokenType tt) {
    if (current_ppd.type == tt) {
        inc_ppd;
        return true;
    }
    print_err("Expected `%tr` found `%t` at %y\n", tt, current_ppd, ppd_printable(current_ppd));
    return false;
}

char* take_ident(ProgramParseData* ppd) {
    if (current_ppd.type != TOKEN_IDENTIFIER) {
        print_err("Expected identifier found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
        exit(1);
    }
    size_t len = current_ppd.value.string.length;
    char* name = malloc(sizeof(char) * len+1);
    strncpy(name, current_ppd.value.string.text, len);
    name[len] = '\0';
    inc_ppd;
    return name;
}

bool parse_expr(ProgramParseData* ppd, Expr* expr) {
    if (current_ppd.type != TOKEN_INTEGER) {
        print_err("Expected integer found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
        return false;
    }
    expr->type = EXPR_LITERAL_INTEGER;
    expr->value.integer = current_ppd.value.integer;
    inc_ppd;
    return true;
}

bool parse_stms_let(ProgramParseData* ppd, Stmt* stmt) {
    // let ident = expr;
    inc_ppd;
    stmt->type = STMT_DECLARE;
    stmt->value.decl.name = take_ident(ppd);
    if (!consume_token(ppd, TOKEN_EQUALS)) return false;
    Expr expr;
    parse_expr(ppd, &expr);
    stmt->value.decl.expr = expr;
    if (!consume_token(ppd, TOKEN_SEMICOLON)) return false;
    return true;
}

bool parse_stmts(ProgramParseData* ppd, Stmt** stmts) {
    // 1. let ident = expr; // declaration
    // 2. ident = expr; // assign
    // 3. ident() // call
    // 4. if expr { stmt; stmt; }
    while (current_ppd.type != TOKEN_RBRACE) {
        switch (current_ppd.type) {
            case TOKEN_EOF: return false;
            case TOKEN_KW_LET:
                Stmt stmt;
                if (!parse_stms_let(ppd, &stmt)) return false;
                vec_push(*stmts, stmt);
                break;
            default:
                print_err("Expected statement found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
                return false;
        }
    }
    return true;
}

bool parse_item_fn(ProgramParseData* ppd, Program* program) {
    // fn ident() { stmt; stmt; }
    Item fun = {
        .type = ITEM_FUNC
    };
    inc_ppd;
    fun.value.fn.name = take_ident(ppd);
    if (!consume_token(ppd, TOKEN_LPAREN)) return false;
    // TODO: function args
    if (!consume_token(ppd, TOKEN_RPAREN)) return false;
    if (!consume_token(ppd, TOKEN_LBRACE)) return false;
    fun.value.fn.stmts = NULL;
    if (!parse_stmts(ppd, &fun.value.fn.stmts)) return false;
    if (!consume_token(ppd, TOKEN_RBRACE)) return false;
    vec_push(program->items, fun);
    return true;
}

Option parse(const char* filepath, const char* code, const Token* tokens) {
    Option op = {.t = OPTION_None};
    Program* program = calloc(1, sizeof(Program));
    size_t current = 0;

    ProgramParseData ppd = {
        .filepath = filepath,
        .code = code,
        .tokens = tokens,
        .current = &current,
    };

    while (tokens[current].type != TOKEN_EOF) {
        switch (tokens[current].type) {
            case TOKEN_KW_FN:
                if (!parse_item_fn(&ppd, program)) return op;
                break;
            default:
                print_err("Expected item `fn` found `%t` at %y\n", tokens[current], filepath, tokens[current].start.line, tokens[current].start.column);
                return op;
        }
    }

    op.t = OPTION_Some;
    op.data = program;
    return op;
}

void print_expr(Expr expr) {
    if (expr.type == EXPR_LITERAL_INTEGER) {
        printf("\t%zu\n", expr.value.integer);
    }
}

void print_stmt(Stmt stmt) {
    if (stmt.type == STMT_DECLARE) {
        printf("\tDECLARATION\n\tVariable Name: %s\n\tExpr:\n", stmt.value.decl.name);
        print_expr(stmt.value.decl.expr);
    }
}

void print_item(Item item) {
    if (item.type == ITEM_FUNC) {
        printf("FUNCTION\n Name: %s\n Stmts:\n", item.value.fn.name);
        for (size_t i = 0; i < vec_len(item.value.fn.stmts); i++) {
            printf("%zu.", i+1);
            print_stmt(item.value.fn.stmts[i]);
        }
    }
}

void print_program(Program* prog) {
    for (size_t i = 0; i < vec_len(prog->items); i++) {
        print_item(prog->items[i]);
    }
}