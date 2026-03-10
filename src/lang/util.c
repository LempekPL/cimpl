#include "util.h"
#include "token.h"
#include <string.h>
#include <stdarg.h>

void fprintf_ext_internal(FILE* const stream, const char* const format, va_list args) {
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
                        putc(*p, stream);
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
}

void print_err(const char* format, ...) {
    FILE* stream = stderr;
    fprintf(stream, "[ERROR] ");
    va_list args;
    va_start(args, format);
    fprintf_ext_internal(stream, format, args);
    va_end(args);
}

void print_ext(const char* format, ...) {
    FILE* stream = stdout;
    va_list args;
    va_start(args, format);
    fprintf_ext_internal(stream, format, args);
    va_end(args);
}

void fprint_ext(FILE* const stream, const char* const format, ...) {
    va_list args;
    va_start(args, format);
    fprintf_ext_internal(stream, format, args);
    va_end(args);
}