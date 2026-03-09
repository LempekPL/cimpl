#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

Program parse(const char* filepath, const char* code, Token* tokens) {
    Program program = {0};
    size_t current = 0;
    
    while (tokens[current].type != TOKEN_EOF) {
        switch (tokens[current].type) {
            default:
                // if (filepath != NULL)
                //     fprintf(stderr, "[ERROR] Wrong token `%c` at [%s:%zu:%zu]\n", , filepath, line, column);
                // else
                //     fprintf(stderr, "[ERROR] Wrong token `%c` at [%zu:%zu]\n", c_char, line, column);
                exit(1);
        
        }
    }

    return program;
}