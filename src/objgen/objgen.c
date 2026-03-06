#include <stdint.h>
#include <stdio.h>
#include "../vec.h"

void objgen() {

}

void append_1(uint8_t** bytes, uint8_t v) {
    vec_push(*bytes, v);
}

void edit_1(uint8_t** bytes, uint8_t v, size_t i) {
    (*bytes)[i] = v;
}

uint8_t get_1(uint8_t** bytes, size_t i) {
    return (*bytes)[i];
}

void append_2(uint8_t** bytes, uint16_t v) {
    vec_push(*bytes, (v & 0x00FF));
    vec_push(*bytes, (v & 0xFF00) >> 8);
}

void edit_2(uint8_t** bytes, uint16_t v, size_t i) {
    (*bytes)[i+0] = (v & 0x00FF);
    (*bytes)[i+1] = (v & 0xFF00) >> 8;
}

uint16_t get_2(uint8_t** bytes, size_t i) {
    return (*bytes)[i] + ((uint16_t)(*bytes)[i+1] << 8);
}

void append_4(uint8_t** bytes, uint32_t v) {
    vec_push(*bytes, (v & 0x000000FF));
    vec_push(*bytes, (v & 0x0000FF00) >> 8);
    vec_push(*bytes, (v & 0x00FF0000) >> 16);
    vec_push(*bytes, (v & 0xFF000000) >> 24);
}

void edit_4(uint8_t** bytes, uint32_t v, size_t i) {
    (*bytes)[i+0] = (v & 0x000000FF);
    (*bytes)[i+1] = (v & 0x0000FF00) >> 8;
    (*bytes)[i+2] = (v & 0x00FF0000) >> 16;
    (*bytes)[i+3] = (v & 0xFF000000) >> 24;
}

uint32_t get_4(uint8_t** bytes, size_t i) {
    return (*bytes)[i] + ((uint32_t)(*bytes)[i+1] << 8)
     + ((uint32_t)(*bytes)[i+2] << 16) + ((uint32_t)(*bytes)[i+3] << 24);
}

void append_8(uint8_t** bytes, uint64_t v) {
    vec_push(*bytes, (v & 0x00000000000000FF));
    vec_push(*bytes, (v & 0x000000000000FF00) >> 8);
    vec_push(*bytes, (v & 0x0000000000FF0000) >> 16);
    vec_push(*bytes, (v & 0x00000000FF000000) >> 24);
    vec_push(*bytes, (v & 0x000000FF00000000) >> 32);
    vec_push(*bytes, (v & 0x0000FF0000000000) >> 40);
    vec_push(*bytes, (v & 0x00FF000000000000) >> 48);
    vec_push(*bytes, (v & 0xFF00000000000000) >> 56);
}

void edit_8(uint8_t** bytes, uint64_t v, size_t i) {
    (*bytes)[i+0] = (v & 0x00000000000000FF);
    (*bytes)[i+1] = (v & 0x000000000000FF00) >> 8;
    (*bytes)[i+2] = (v & 0x0000000000FF0000) >> 16;
    (*bytes)[i+3] = (v & 0x00000000FF000000) >> 24;
    (*bytes)[i+4] = (v & 0x000000FF00000000) >> 32;
    (*bytes)[i+5] = (v & 0x0000FF0000000000) >> 40;
    (*bytes)[i+6] = (v & 0x00FF000000000000) >> 48;
    (*bytes)[i+7] = (v & 0xFF00000000000000) >> 56;
}

uint64_t get_8(uint8_t** bytes, size_t i) {
    return (*bytes)[i] + ((*bytes)[i+1] << 8)
     + ((uint64_t)(*bytes)[i+2] << 16) + ((uint64_t)(*bytes)[i+3] << 24)
     + ((uint64_t)(*bytes)[i+4] << 32) + ((uint64_t)(*bytes)[i+5] << 40)
     + ((uint64_t)(*bytes)[i+6] << 48) + ((uint64_t)(*bytes)[i+7] << 56);
}

void append_vec(uint8_t** bytes, const uint8_t* const vec) {
    for (size_t i = 0; i < vec_len(vec); i++) {
        append_1(bytes, vec[i]);
    }
}

void append_arr(uint8_t** bytes, uint8_t* vec, size_t vec_len) {
    for (size_t i = 0; i < vec_len; i++) {
        append_1(bytes, vec[i]);
    }
}

void padding(uint8_t** bytes, size_t pad_in_bytes) {
    for (size_t i = 0; i < pad_in_bytes; i++) {
        vec_push(*bytes, 0);
    }
}

void create_obj() {
    uint64_t vaddress = 0x400000;

    uint8_t* prog = NULL;
    // 0-3 required magic
    append_4(&prog, 0x464c457F); // E L F
    // 4-5 64bit | little endian
    append_1(&prog, 0x02);
    append_1(&prog, 0x01);
    // 6-8 ELF header version | OS ABI | ABI Version
    append_1(&prog, 1);
    append_1(&prog, 0);
    append_1(&prog, 0);
    // 9-15 padding
    padding(&prog, 7);

    // 16-17 type = Shared
    append_2(&prog, 3);
    // 18-19 Instruction Set = x86-64
    append_2(&prog, 0x3E);
    // 20-23 ELF version
    append_4(&prog, 1);


    size_t program_entry = vec_len(prog);
    // 24-31 program entry offset aka program start
    append_8(&prog, 0);
    // 32-39 program header table offset = 64bits after the ELF header
    append_8(&prog, 0x40);

    size_t shtabl_offset = vec_len(prog);
    // 40-47 section header table offset
    append_8(&prog, 0);
    // 48-51 flags IGNORED
    append_4(&prog, 0);
    // 52-53 ELF header size - always 64bits (in 64bit)
    append_2(&prog, 0x40);

    // 54-55 size of an entry in the program header table
    append_2(&prog, 0x38);
    // 56-57 number of entries in the program header table = amount of program headers
    append_2(&prog, 2);
    
    // 58-59 size of an entry in the section header table
    append_2(&prog, 0x40);
    // 60-61 number of entries in the section header table
    append_2(&prog, 4);
    // 62-63 section index to the section header string table 
    append_2(&prog, 3);
    


    // TEXT (CODE) segment
    // segment type = load
    append_4(&prog, 1);
    // flags - 1 = executable, 2 = writable, 4 = readable
    append_4(&prog, 5);

    // size_t code_bytes_offset = vec_len(prog);
    // The offset in the file that the data for this segment can be found (p_offset)
    append_8(&prog, 0);
    // append_8(&prog, vaddress);
    // size_t code_bytes_vaddr = vec_len(prog); // p_vaddr should equal p_offset, modulo p_align
    // Where you should start to put this segment in virtual memory (p_vaddr)
    append_8(&prog, vaddress);

    // Reserved for segment's physical address (p_paddr) IGNORED
    append_8(&prog, vaddress);

    size_t code_bytes_filesz = vec_len(prog);
    // Size of the segment in the file (p_filesz) 
    append_8(&prog, 0);
    size_t code_bytes_memsz = vec_len(prog);
    // Size of the segment in memory (p_memsz, at least as big as p_filesz) 
    append_8(&prog, 0);
    // The required alignment for this section (usually a power of 2)
    append_8(&prog, 0x1000);


    // DATA segment
    // segment type = load
    append_4(&prog, 1);
    // flags - 1 = executable, 2 = writable, 4 = readable
    append_4(&prog, 6);

    size_t data_bytes_offset = vec_len(prog);
    // The offset in the file that the data for this segment can be found (p_offset)
    append_8(&prog, 0);
    size_t data_bytes_vaddr = vec_len(prog);
    // Where you should start to put this segment in virtual memory (p_vaddr)
    append_8(&prog, 0);


    // Reserved for segment's physical address (p_paddr) IGNORED
    append_8(&prog, 0);

    size_t data_bytes_filesz = vec_len(prog);
    // Size of the segment in the file (p_filesz) 
    append_8(&prog, 0);
    size_t data_bytes_memsz = vec_len(prog);
    // Size of the segment in memory (p_memsz, at least as big as p_filesz) 
    append_8(&prog, 0);
    // The required alignment for this section (usually a power of 2) 
    append_8(&prog, 0x1000);

    size_t code_offset = vec_len(prog);
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
    append_arr(&prog, code, code_size);

    size_t data_offset = vec_len(prog);
    uint8_t data[] = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21, 0x0A
    };
    size_t data_size = sizeof(data)/sizeof(*data);
    append_arr(&prog, data, data_size);

    size_t shstrtab_offset = vec_len(prog);
    uint8_t shstrtab_data[] = {
        0x00, 
        '.', 't', 'e', 'x', 't', 0x00,
        '.', 'd', 'a', 't', 'a', 0x00,
        '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', 0x00 
    };
    size_t shstrtab_size = sizeof(shstrtab_data) / sizeof(*shstrtab_data);
    append_arr(&prog, shstrtab_data, shstrtab_size);

    // ==========================================
    // NOWE: SECTION HEADER TABLE (SHT)
    // ==========================================
    size_t sht_offset = vec_len(prog);

    // 0. Wymagana pusta sekcja (NULL) - 64 bajty zer
    padding(&prog, 64);

    // 1. Sekcja .text
    append_4(&prog, 1); // sh_name (offset 1 w .shstrtab -> ".text")
    append_4(&prog, 1); // sh_type = SHT_PROGBITS (kod/dane programu)
    append_8(&prog, 6); // sh_flags = SHF_ALLOC | SHF_EXECINSTR
    append_8(&prog, vaddress + code_offset); // sh_addr (Adres wirtualny)
    append_8(&prog, code_offset); // sh_offset (Gdzie w pliku)
    append_8(&prog, code_size);   // sh_size
    append_4(&prog, 0); // sh_link
    append_4(&prog, 0); // sh_info
    append_8(&prog, 16); // sh_addralign
    append_8(&prog, 0);  // sh_entsize

    // 2. Sekcja .data
    uint64_t theta_vaddr_data = vaddress + 0x1000 + (data_offset % 0x1000); // Wirtualny adres dla patchowania

    append_4(&prog, 7); // sh_name (offset 7 w .shstrtab -> ".data")
    append_4(&prog, 1); // sh_type = SHT_PROGBITS
    append_8(&prog, 3); // sh_flags = SHF_ALLOC | SHF_WRITE
    append_8(&prog, theta_vaddr_data); // sh_addr
    append_8(&prog, data_offset); // sh_offset
    append_8(&prog, data_size);   // sh_size
    append_4(&prog, 0);
    append_4(&prog, 0);
    append_8(&prog, 8); // sh_addralign
    append_8(&prog, 0);

    // 3. Sekcja .shstrtab
    append_4(&prog, 13); // sh_name (offset 13 w .shstrtab -> ".shstrtab")
    append_4(&prog, 3);  // sh_type = SHT_STRTAB (Tabela stringów)
    append_8(&prog, 0);  // sh_flags = brak (nie ładujemy tego do pamięci)
    append_8(&prog, 0);  // sh_addr = 0
    append_8(&prog, shstrtab_offset); // sh_offset
    append_8(&prog, shstrtab_size);   // sh_size
    append_4(&prog, 0);
    append_4(&prog, 0);
    append_8(&prog, 1); // sh_addralign
    append_8(&prog, 0);

    // fixing the addrs
    uint64_t vaddr_data = vaddress + 0x1000 + (data_offset % 0x1000);

    int32_t rip_offset = (int32_t)(theta_vaddr_data - (vaddress + code_offset + 21));
    edit_4(&prog, (uint32_t)rip_offset, code_offset + 17);
    
    edit_8(&prog, sht_offset, shtabl_offset);

    edit_8(&prog, vaddress + code_offset, program_entry);

    edit_8(&prog, data_offset, code_bytes_filesz);
    edit_8(&prog, data_offset, code_bytes_memsz);
 
    edit_8(&prog, data_offset, data_bytes_offset);
    edit_8(&prog, vaddr_data, data_bytes_vaddr);
    edit_8(&prog, data_size, data_bytes_filesz);
    edit_8(&prog, data_size, data_bytes_memsz);


    FILE* progObj = fopen( "prog", "w");
    for (size_t i = 0; i < vec_len(prog); i++) {
        fputc(prog[i], progObj);
    }
    fclose(progObj);
    vec_free(prog);
}