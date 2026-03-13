#include <stdio.h>
#include <stdlib.h>
// #include "objgen/objgen.h"
#include "vec.h"
#define DROP_IMPLEMENTATION
#include "dropper.h"
#include "lang/parser.h"
#include "lang/util.h"
#include "lang/token.h"
#include "lang/util.h"

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
    drop_new(d);
    // create_obj();
    char* file = "./main.cimpl";
    char* code = 0;
    read_file(file, &code);
    drop_free(d, &code);
    Option op_tokens = tokenize(file, code);
    if (op_tokens.t == OPTION_None) {
        return 1;
    }
    Token* tokens = op_tokens.data;
    drop_vec(d, &tokens);
    pretty_print_tokens(tokens);
    Option op_program = parse(file, code, tokens, &d);
    if (op_program.t == OPTION_None) {
        return 1;
    }
    Program* program = op_program.data;
    drop_free(d, &program);
    print_program(program);
    return 0;
}