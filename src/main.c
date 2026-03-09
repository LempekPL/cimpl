// #include "objgen/objgen.h"
#include "lang/token.h"
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
    Token* tokens = tokenize(file, code);
    pretty_print_tokens(tokens);
    return 0;
}