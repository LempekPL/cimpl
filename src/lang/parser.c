#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "../vec.h"
#include "token.h"
#include "util.h"

#define inc_ppd (*ppd->current)++
#define dec_ppd (*ppd->current)--
#define current_ppd ppd->tokens[*ppd->current]
#define next_ppd ppd->tokens[*ppd->current + 1]
#define ppd_printable(token) ppd->filepath, token.start.line, token.start.column
#define ppd_free(x) do { drop_free(*ppd->drops, (x)); } while (0)
#define ppd_vec_free(x) do { drop_vec(*ppd->drops, (x)); } while (0)
#define ppd_malloc(name, type, size) type* name = malloc(sizeof(type) * (size)); drop_box_push(*(ppd->drops), (name))

typedef struct {
    const char* filepath;
    const char* code;
    const Token* tokens;
    size_t* current;
    Drop** drops;
} ProgramParseData;

bool consume_token(ProgramParseData* ppd, TokenType tt) {
    if (current_ppd.type == tt) {
        inc_ppd;
        return true;
    }
    print_err("Expected `%tr` found `%t` at %y\n", tt, current_ppd, ppd_printable(current_ppd));
    return false;
}

Option take_ident(ProgramParseData* ppd) {
    if (current_ppd.type != TOKEN_IDENTIFIER) {
        print_err("Expected identifier found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
        return OptionNone;
    }
    size_t len = current_ppd.value.string.length;
    ppd_malloc(name, char, len + 1);
    memcpy(name, current_ppd.value.string.text, len);
    name[len] = '\0';
    inc_ppd;
    return OptionSome(name);
}

int get_infix_bp(TokenType type) {
    switch (type) {
        case TOKEN_AND:
        case TOKEN_OR:
            return 10;
        case TOKEN_EQUALSEQUALS:
        case TOKEN_GREATER:
        case TOKEN_GREATEREQUALS:
        case TOKEN_LESSER:
        case TOKEN_LESSEREQUALS:
            return 20;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 30;
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
            return 40;
        default:
            return 0;
    }
}

Expr* parse_expr_bp(ProgramParseData* ppd, int min_bp) {
    ppd_malloc(left, Expr, 1);
    // Expr* left = malloc(sizeof(Expr));
    if (current_ppd.type == TOKEN_INTEGER) {
        left->type = EXPR_LITERAL_INTEGER;
        left->value.integer = current_ppd.value.integer;
        inc_ppd;
    } else if (current_ppd.type == TOKEN_IDENTIFIER && next_ppd.type == TOKEN_LPAREN) {
        left->type = EXPR_CALL;
        OptionUnwrap(left->value.ident, take_ident(ppd));
        if (!consume_token(ppd, TOKEN_LPAREN)) return false;
        // TODO: function args
        if (!consume_token(ppd, TOKEN_RPAREN)) return false;
    }  else if (current_ppd.type == TOKEN_IDENTIFIER) {
        left->type = EXPR_IDENTIFIER;
        OptionUnwrap(left->value.ident, take_ident(ppd));
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
        // Expr* new_left = malloc(sizeof(Expr));
        ppd_malloc(new_left, Expr, 1);
        new_left->type = EXPR_BINOP;
        new_left->value.binop.left = left;
        new_left->value.binop.op = op;
        new_left->value.binop.right = right;
        left = new_left;
    }

    return left;
}

bool parse_expr(ProgramParseData* ppd, Expr** expr) {
    *expr = parse_expr_bp(ppd, 0);
    return true;
}

bool parse_stmt_decl(ProgramParseData* ppd, Stmt* stmt) {
    // let ident = expr;
    inc_ppd;
    stmt->type = STMT_DECLARE;
    OptionUnwrap(stmt->value.decl.name, take_ident(ppd));
    if (current_ppd.type != TOKEN_SEMICOLON) {
        if (!consume_token(ppd, TOKEN_EQUALS)) return false;
        ppd_malloc(expr, Expr, 1);
        parse_expr(ppd, &expr);
        stmt->value.decl.expr = expr;
    }
    if (!consume_token(ppd, TOKEN_SEMICOLON)) return false;
    return true;
}

bool parse_stmt_call(ProgramParseData* ppd, Stmt* stmt) {
    // ident(expr1, expr2);
    stmt->type = STMT_CALL;
    OptionUnwrap(stmt->value.call.name, take_ident(ppd));
    if (!consume_token(ppd, TOKEN_LPAREN)) return false;
    // TODO: function args
    if (!consume_token(ppd, TOKEN_RPAREN)) return false;
    if (!consume_token(ppd, TOKEN_SEMICOLON)) return false;
    return true;
}

bool parse_stmt_ass(ProgramParseData* ppd, Stmt* stmt) {
    // ident +-*/= expr;
    stmt->type = STMT_ASSIGN;
    OptionUnwrap(stmt->value.ass.name , take_ident(ppd));
    switch (current_ppd.type) {
        case TOKEN_EQUALS:
        case TOKEN_PLUSEQUALS:
        case TOKEN_MINUSEQUALS:
        case TOKEN_STAREQUALS:
        case TOKEN_SLASHEQUALS:
        case TOKEN_PERCENTEQUALS:
            stmt->value.ass.type = current_ppd;
            break;
        default:
            print_err("Expected `=` found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
    }
    inc_ppd;
    ppd_malloc(expr, Expr, 1);
    parse_expr(ppd, &expr);
    stmt->value.ass.expr = expr;
    if (!consume_token(ppd, TOKEN_SEMICOLON)) return false;
    return true;
}

bool parse_stmts(ProgramParseData* ppd, Stmt** stmts);

bool parse_stmt_if(ProgramParseData* ppd, Stmt* stmt) {
    // if expr { stmt; stmt; }
    stmt->type = STMT_IF;
    inc_ppd;
    ppd_malloc(expr, Expr, 1);
    parse_expr(ppd, &expr);
    stmt->value.ifs.expr = expr;
    if (!consume_token(ppd, TOKEN_LBRACE)) return false;
    stmt->value.ifs.stmts = NULL;
    parse_stmts(ppd, &stmt->value.ifs.stmts);
    drop_box_push(*(ppd->drops), stmt->value.ifs.stmts);
    if (!consume_token(ppd, TOKEN_RBRACE)) return false;
    return true;
}

bool parse_stmts(ProgramParseData* ppd, Stmt** stmts) {
    // 1. let ident = expr; // declaration
    // 2. ident = expr; // assign
    // 3. ident() // call
    // 4. if expr { stmt; stmt; }
    while (current_ppd.type != TOKEN_RBRACE) {
        Stmt stmt = {0};
        switch (current_ppd.type) {
            case TOKEN_EOF: return false;
            case TOKEN_KW_LET: 
                if (!parse_stmt_decl(ppd, &stmt)) return false;
                break;
            case TOKEN_IDENTIFIER:
                if (next_ppd.type == TOKEN_LPAREN) {
                    if (!parse_stmt_call(ppd, &stmt)) return false;
                } else {
                    if (!parse_stmt_ass(ppd, &stmt)) return false;
                }
                break;
            case TOKEN_KW_IF:
                if (!parse_stmt_if(ppd, &stmt)) return false;
                break;
            default:
                print_err("Expected statement found `%t` at %y\n", current_ppd, ppd_printable(current_ppd));
                return false;
        }
        vec_push(*stmts, stmt);
    }
    return true;
}

bool parse_item_fn(ProgramParseData* ppd, Program* program) {
    // fn ident() { stmt; stmt; }
    Item fun = {
        .type = ITEM_FUNC
    };
    inc_ppd;
    OptionUnwrap(fun.value.fn.name, take_ident(ppd));
    if (!consume_token(ppd, TOKEN_LPAREN)) return false;
    // TODO: function args
    if (!consume_token(ppd, TOKEN_RPAREN)) return false;
    if (!consume_token(ppd, TOKEN_LBRACE)) return false;
    vec_new(fun.value.fn.stmts, 2);
    drop_vec(*ppd->drops, &fun.value.fn.stmts);
    if (!parse_stmts(ppd, &fun.value.fn.stmts)) return false;
    if (!consume_token(ppd, TOKEN_RBRACE)) return false;
    vec_push(program->items, fun);
    return true;
}

void free_program(void** program) {
    Program** prog = (Program**)program;
    if (prog == NULL || *prog == NULL) return;
    vec_free((*prog)->items);
    free(*prog);
    free(prog);
}

Option parse(const char* filepath, const char* code, const Token* tokens, Drop** drops) {
    Program* program = calloc(1, sizeof(Program));
    Program** box_value(programPtr, program);
    drop_push(*drops, free_program, programPtr);
    size_t current = 0;

    ProgramParseData ppd = {
        .filepath = filepath,
        .code = code,
        .tokens = tokens,
        .current = &current,
        .drops = drops
    };

    while (tokens[current].type != TOKEN_EOF) {
        switch (tokens[current].type) {
            case TOKEN_KW_FN:
                if (!parse_item_fn(&ppd, program)) return OptionNone;
                break;
            default:
                print_err("Expected item `fn` found `%t` at %y\n", tokens[current], filepath, tokens[current].start.line, tokens[current].start.column);
                return OptionNone;
        }
    }

    return OptionSome(program);
}

void print_indent(size_t level) {
    for (size_t i = 0; i < level; i++) {
        printf("  ");
    }
}

void print_expr(Expr* expr, size_t indent) {
    if (expr == NULL) return;

    print_indent(indent);

    switch (expr->type) {
        case EXPR_LITERAL_INTEGER:
            printf("Integer: %zu\n", expr->value.integer);
            break;
            
        case EXPR_IDENTIFIER:
            printf("Identifier: %s\n", expr->value.ident);
            break;
            
        case EXPR_CALL:
            printf("Call: %s()\n", expr->value.call.name);
            break;
            
        case EXPR_BINOP: {
            printf("BinaryOp: ");
            char buf[64];
            sprint_token(buf, expr->value.binop.op);
            printf("%s\n", buf);

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

void print_stmt(Stmt stmt, size_t indent) {
    switch (stmt.type) {
        case STMT_DECLARE:
            printf("\tDECLARATION\n\tVariable Name: %s\n\tExpr:\n", stmt.value.decl.name);
            print_expr(stmt.value.decl.expr, indent);
            printf("\n");
            break;
        case STMT_ASSIGN:
            printf("\tASSIGN\n\tVariable Name: %s\n\tType: `", stmt.value.ass.name);
            print_token(stmt.value.ass.type);
            printf("`\n");
            print_expr(stmt.value.ass.expr, indent);
            printf("\n");
            break;
        case STMT_CALL:
            printf("\tCALL\n\tFunction Name: %s\n", stmt.value.call.name);
            printf("\n");
            break;
        case STMT_IF:
            printf("\tIF\n\tExpr:\n");
            print_expr(stmt.value.ifs.expr, indent);
            printf("\tStmts:\n");
            for (size_t i = 0; i < vec_len(stmt.value.ifs.stmts); i++) {
                print_indent(indent+2);
                printf("%zu.", i+1);
                print_stmt(stmt.value.ifs.stmts[i], indent+2);
            }
            break;
    }
}

void print_item(Item item) {
    if (item.type == ITEM_FUNC) {
        printf("FUNCTION\n Name: %s\n Stmts:\n", item.value.fn.name);
        for (size_t i = 0; i < vec_len(item.value.fn.stmts); i++) {
            printf("%zu.", i+1);
            print_stmt(item.value.fn.stmts[i], 2);
        }
    }
}

void print_program(Program* prog) {
    for (size_t i = 0; i < vec_len(prog->items); i++) {
        print_item(prog->items[i]);
    }
}