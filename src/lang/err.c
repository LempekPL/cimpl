#include "err.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// void print_err_file(const char* filepath, const char* msg, const Token token) {

// }

void print_err(const char* format, ...) {
    FILE* stream = stderr;

    va_list args;
    va_start(args, format);
    fprintf(stream, "[ERROR] ");



    for (const char* p = format; *p != '\0'; p++) {
        if (*p != '%') {
            putc(*p, stream);
            continue;
        }
        p++;
        switch (*p) {
            case 'q':
                const char* filename = va_arg(args, const char*);
                size_t line = va_arg(args, size_t);
                size_t column = va_arg(args, size_t);
                putc('[', stream);
                if (filename != NULL && strlen(filename) > 0) fprintf(stream, "%s:", filename);
                fprintf(stream, "%zu:%zu]", line, column);
                break;
            case 't':
                p++;
                Token token = va_arg(args, Token);
                switch (*p) {
                    case 't':
                        fprint_token_type(stream, token.type);
                        break;
                    default:
                        fprint_token(stream, token);
                        break;           
                }
                break;
            case 'c':
                char c = va_arg(args, int);
                putc(c, stream);
                break;
            case 's':
                const char* str = va_arg(args, const char*);
                fprintf(stream, "%s", str);
                break;
            case '%':
                fprintf(stream, "%%");
                break;
            default:
                putc(*p, stream);
                break;
        }
    }
    va_end(args);
}