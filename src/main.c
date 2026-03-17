#include <stdio.h>
#include <stdlib.h>
#define VEC_EXTRAS
#include "vec.h"
#define DROP_IMPLEMENTATION
#include "dropper.h"
#include "lang/parser.h"
#include "util.h"
#include "lang/token.h"
#include "objgen/objgen.h"

bool read_file(const char* filepath, char** buffer) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    *buffer = malloc(length + 1);
    if (*buffer) {
        fread(*buffer, 1, length, f);
        (*buffer)[length] = '\0';
        fclose(f);
        return true;
    }

    fclose(f);
    return false;
}

int main() {
    ElfProgram* prog = create_program();
    write_executable("prog", prog);
    // drop_new(d);
    // char* file = "./main.cimpl";
    // char* code = 0;
    // read_file(file, &code);
    // drop_free(d, &code);
    // Option op_tokens = tokenize(file, code);
    // if (op_tokens.t == OPTION_None) {
    //     return 1;
    // }
    // Token* tokens = op_tokens.data;
    // drop_vec(d, &tokens);
    // // pretty_print_tokens(tokens);
    // Option op_program = parse(file, code, tokens, &d);
    // if (op_program.t == OPTION_None) {
    //     return 1;
    // }
    // Program* program = op_program.data;
    // print_program(program);
    return 0;
}