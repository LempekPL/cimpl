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
    char* name = malloc(len + 1);
    memcpy(name, current_ppd.value.string.text, len);
    name[len] = '\0';
    inc_ppd;
    return name;
}

int get_infix_bp(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 10;
        case TOKEN_STAR:
        case TOKEN_SLASH:
            return 20;
        default:
            return 0;
    }
}

Expr* parse_expr_bp(ProgramParseData* ppd, int min_bp) {
    Expr* left = malloc(sizeof(Expr));
    if (current_ppd.type == TOKEN_INTEGER) {
        left->type = EXPR_LITERAL_INTEGER;
        printf("AAAA: %zu\n", current_ppd.value.integer);
        left->value.integer = current_ppd.value.integer;
        inc_ppd;
    } else if (current_ppd.type == TOKEN_IDENTIFIER) {
        left->type = EXPR_IDENTIFIER;
        left->value.ident = take_ident(ppd);
    } else if (current_ppd.type == TOKEN_LPAREN) {
        inc_ppd;
        left = parse_expr_bp(ppd, 0); 
        if (!consume_token(ppd, TOKEN_RPAREN)) exit(1);
    }
    else {
        print_err("Expected expression found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
        free(left);
        exit(1);
    }

    while (true) {
        Token op = current_ppd;
        int bp = get_infix_bp(op.type);
        if (bp == 0 || bp < min_bp) {
            break;
        }
        inc_ppd;
        Expr* right = parse_expr_bp(ppd, bp + 1);
        Expr* new_left = malloc(sizeof(Expr));
        new_left->type = EXPR_BINOP;
        new_left->value.binop.left = left;
        new_left->value.binop.op = op;
        new_left->value.binop.right = right;

        left = new_left;
    }

    return left;
}

bool parse_expr(ProgramParseData* ppd, Expr* expr) {
    expr = parse_expr_bp(ppd, 0);
    if (expr->type == EXPR_LITERAL_INTEGER) {
        printf("BBB: %zu\n", expr->value.integer);
    }
    return true;

    // if (current_ppd.type != TOKEN_INTEGER) {
    //     print_err("Expected integer found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
    //     return false;
    // }
    // expr->type = EXPR_LITERAL_INTEGER;
    // expr->value.integer = current_ppd.value.integer;
    // inc_ppd;
    // return true;
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
            case TOKEN_KW_LET: {
                Stmt stmt;
                if (!parse_stms_let(ppd, &stmt)) return false;
                vec_push(*stmts, stmt);
                break;
            }
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

void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

void print_expr(Expr* expr, int indent) {
    if (expr == NULL) return;

    print_indent(indent);

    switch (expr->type) {
        case EXPR_LITERAL_INTEGER:
            printf("Integer: %zu\n", expr->value.integer);
            break;
            
        case EXPR_IDENTIFIER:
            printf("Identifier: %s\n", expr->value.ident);
            break;
            
        case EXPR_BINOP: {
            // Wypisujemy sam węzeł operatora
            printf("BinaryOp: ");
            // Używamy makra/funkcji do wypisania samego znaku (np. '+', '*')
            // Zależnie od tego, jak masz to teraz nazwane w swoim kodzie, np:
            char buf[64];
            sprint_token(buf, expr->value.binop.op);
            printf("%s\n", buf);

            // Rekurencyjnie wypisujemy lewą i prawą stronę, zwiększając wcięcie!
            print_indent(indent + 1);
            printf("Left:\n");
            print_expr(expr->value.binop.left, indent + 2);

            print_indent(indent + 1);
            printf("Right:\n");
            print_expr(expr->value.binop.right, indent + 2);
            break;
        }
    }
}

void print_stmt(Stmt stmt) {
    if (stmt.type == STMT_DECLARE) {
        printf("\tDECLARATION\n\tVariable Name: %s\n\tExpr:\n", stmt.value.decl.name);
        print_expr(&stmt.value.decl.expr, 2);
        printf("\n");
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