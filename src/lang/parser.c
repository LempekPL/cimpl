#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

Option parse(const char* filepath, const char* code, Token* tokens) {
    Option op = {.t = OPTION_None};
    Program* program = malloc(sizeof(Program));
    size_t current = 0;

    while (tokens[current].type != TOKEN_EOF) {
        switch (tokens[current].type) {
            default:
                print_err("Expected token `fn` found `%t` at %q\n", tokens[current], filepath, tokens[current].start.line, tokens[current].start.column);
                exit(1);
        
        }
    }

    op.t = OPTION_Some;
    op.data = program;
    return op;
}