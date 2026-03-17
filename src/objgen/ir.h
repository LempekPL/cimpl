#ifndef IR_H
#define IR_H
#include "../util.h"
#include "../lang/parser.h"

typedef enum {
    IR_GLOBAL_DATA,
    IR_STORE,
    IR_FUNCTION
} IRType;

typedef struct {
    char* name;
    size_t offset;
} IRFunc;

typedef struct {
    IRType type;
    union {
        size_t offset;
        IRFunc func;
    } data;
} IRcode;

typedef struct {
    char* data;
    size_t offset;
} IRdata;

typedef struct {
    IRcode* code;
    IRdata* data;
} IRprogram;

Option generate_ir(Program* program);

#endif // IR_H