#include <elf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "objgen.h"
#include "../vec.h"

void append_vec(uint8_t** bytes, const uint8_t* const vec) {
    for (size_t i = 0; i < vec_len(vec); i++) {
        vec_push(*bytes, vec[i]);
    }
}

void append_arr(uint8_t** bytes, uint8_t* vec, size_t vec_len) {
    for (size_t i = 0; i < vec_len; i++) {
        vec_push(*bytes, vec[i]);
    }
}

ElfProgram* create_program() {
    ElfProgram* ep = malloc(sizeof(ElfProgram));
    ep->e_header.e_ident[EI_MAG0] = ELFMAG0; 
    ep->e_header.e_ident[EI_MAG1] = ELFMAG1; 
    ep->e_header.e_ident[EI_MAG2] = ELFMAG2; 
    ep->e_header.e_ident[EI_MAG3] = ELFMAG3; 
    ep->e_header.e_ident[EI_CLASS] = ELFCLASS64;
    ep->e_header.e_ident[EI_DATA] = ELFDATA2LSB;
    ep->e_header.e_ident[EI_VERSION] = EV_CURRENT;
    ep->e_header.e_ident[EI_OSABI] = ELFOSABI_NONE;
    ep->e_header.e_ident[EI_ABIVERSION] = 0;
    ep->e_header.e_type = ET_DYN;
    ep->e_header.e_machine = EM_X86_64;
    ep->e_header.e_version = EV_CURRENT;
    ep->e_header.e_phoff = sizeof(Elf64_Ehdr);
    ep->e_header.e_ehsize = sizeof(Elf64_Ehdr);
    ep->e_header.e_phentsize = sizeof(Elf64_Phdr);
    // segment amount
    ep->e_header.e_phnum = 2;
    ep->e_header.e_shentsize = sizeof(Elf64_Shdr);
    // section amount
    ep->e_header.e_shnum = 4;
    // section index to the section header string table
    ep->e_header.e_shstrndx = 3;

    ep->p_headers = NULL;
    Elf64_Phdr pr_text = {0};
    pr_text.p_type = PT_LOAD;
    pr_text.p_flags = PF_R | PF_X;
    pr_text.p_align = 0x1000;
    vec_push(ep->p_headers, pr_text);

    Elf64_Phdr pr_data = {0};
    pr_data.p_type = PT_LOAD;
    pr_data.p_flags = PF_R | PF_W;
    pr_data.p_align = 0x1000;
    vec_push(ep->p_headers, pr_data);
    
    ep->s_headers = NULL;
    Elf64_Shdr sc_text = {0};
    sc_text.sh_name = 1;
    sc_text.sh_type = SHT_PROGBITS;
    sc_text.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
    sc_text.sh_addralign = 16;
    vec_push(ep->s_headers, sc_text);

    Elf64_Shdr sc_data = {0};
    sc_data.sh_name = 7;
    sc_data.sh_type = SHT_PROGBITS;
    sc_data.sh_flags = SHF_ALLOC | SHF_WRITE;
    sc_data.sh_addralign = 8;
    vec_push(ep->s_headers, sc_data);

    Elf64_Shdr sc_strtab = {0};
    sc_strtab.sh_name = 7;
    sc_strtab.sh_type = SHT_PROGBITS;
    sc_strtab.sh_flags = SHF_ALLOC | SHF_WRITE;
    sc_strtab.sh_addralign = 8;
    vec_push(ep->s_headers, sc_strtab);

    ep->code = NULL;
    ep->data = NULL;
    uint8_t code[] = {
        0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00, // mov rax, 1 // for sys_write
        0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00, // mov rdi, 0 // FD
        0x48, 0x8D, 0x35, 0x00, 0x00, 0x00, 0x00, // lea rsi, [rip + OFFSET]
        0x48, 0xC7, 0xC2, 0x0D, 0x00, 0x00, 0x00, // mov rdx, LENGTH (= 13)
        0x0F, 0x05, // syscall

        0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov rax, 0x3C
        0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00, // mov rdi, 0x0
        0x0F, 0x05 // syscall
    };
    size_t code_size = sizeof(code)/sizeof(*code);
    vec_arr_append(ep->code, code, code_size);

    uint8_t data[] = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21, 0x0A
    };
    size_t data_size = sizeof(data)/sizeof(*data);
    vec_arr_append(ep->data, data, data_size);

    uint8_t shstrtab_data[] = {
        0x00, 
        '.', 't', 'e', 'x', 't', 0x00,
        '.', 'd', 'a', 't', 'a', 0x00,
        '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', 0x00 
    };
    size_t shstrtab_size = sizeof(shstrtab_data) / sizeof(*shstrtab_data);
    vec_arr_append(ep->string_table, shstrtab_data, shstrtab_size);
    return ep;
}

void write_executable(char* filename, ElfProgram* program) {
    FILE* progObj = fopen( filename, "wb");
    uint8_t* bytes = (uint8_t*)&(program->e_header);
    for (size_t i = 0; i < sizeof(Elf64_Ehdr); i++) {
        fputc(bytes[i], progObj);
    }
    for (size_t hi = 0; hi < vec_len(program->p_headers); hi++) {
        bytes = (uint8_t*)&(program->p_headers[hi]);
        for (size_t i = 0; i < sizeof(Elf64_Phdr); i++) {
            fputc(bytes[i], progObj);
        }
    }
    for (size_t hi = 0; hi < vec_len(program->s_headers); hi++) {
        bytes = (uint8_t*)&(program->s_headers[hi]);
        for (size_t i = 0; i < sizeof(Elf64_Shdr); i++) {
            fputc(bytes[i], progObj);
        }
    }
    bytes = program->code;
    for (size_t i = 0; i < vec_len(bytes); i++) {
            fputc(bytes[i], progObj);
    }
    bytes = program->data;
    for (size_t i = 0; i < vec_len(bytes); i++) {
            fputc(bytes[i], progObj);
    }
    bytes = program->string_table;
    for (size_t i = 0; i < vec_len(bytes); i++) {
            fputc(bytes[i], progObj);
    }

    fclose(progObj);
}