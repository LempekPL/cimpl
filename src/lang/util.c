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
            case 'y':
                const char* filename = va_arg(args, const char*);
                size_t line = va_arg(args, size_t);
                size_t column = va_arg(args, size_t);
                putc('[', stream);
                if (filename != NULL && strlen(filename) > 0) fprintf(stream, "%s:", filename);
                fprintf(stream, "%zu:%zu]", line, column);
                break;
            case 't':
                p++;
                switch (*p) {
                    case 'r':
                        TokenType type = va_arg(args, TokenType);
                        fprint_token_type(stream, type);
                        break;
                    case 't':
                        Token token1 = va_arg(args, Token);
                        fprint_token_type(stream, token1.type);
                        break;
                    default:
                        Token token2 = va_arg(args, Token);
                        fprint_token(stream, token2);
                        putc(*p, stream);
                        break;           
                }
                break;
            case 'd':
                int d = va_arg(args, int);
                fprintf(stream, "%d", d);
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