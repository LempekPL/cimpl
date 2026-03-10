// #include "objgen/objgen.h"
#include "lang/parser.h"
#include "lang/util.h"
#include "lang/token.h"
#include "lang/util.h"
#include <stdio.h>
#include <stdlib.h>

bool read_file(const char* filepath, char** buffer) {
    FILE *f = fopen(filepath, "r");
    size_t length;
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        *buffer = malloc(length);
        if (*buffer) {
        fread(*buffer, 1, length, f);
        }
        fclose(f);
        return true;
    }
    return false;
}

int main() {
    // create_obj();
    char* file = "./main.cimpl";
    char* code = 0;
    read_file(file, &code);
    Option op_tokens = tokenize(file, code);
    if (op_tokens.t == OPTION_None) {
        return 1;
    }
    Token* tokens = op_tokens.data;
    pretty_print_tokens(tokens);
    Option op_program = parse(file, code, tokens);
    if (op_program.t == OPTION_None) {
        return 1;
    }
    Program* program = op_program.data;
    print_program(program);
    return 0;
}