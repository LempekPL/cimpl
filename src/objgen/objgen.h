#ifndef OBJGEN_H
#define OBJGEN_H
#include <elf.h>
#include <stdint.h>

typedef struct {
    Elf64_Ehdr e_header;
    Elf64_Phdr* p_headers;
    Elf64_Shdr* s_headers;
    uint8_t* code;
    uint8_t* data;
    uint8_t* string_table;
} ElfProgram;

ElfProgram* create_program();
void write_executable(char* filename, ElfProgram* program);

#endif // OBJGEN_H