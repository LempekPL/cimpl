#include "ir.h"

// label fn
// stmt
// stmt

Option generate_ir(Program* program) {
    IRdata* data = NULL;
    IRcode* code = NULL;

    for (size_t i = 0; i < vec_len(program->items); i++) {
        switch (program->items[0].type) {
            case ITEM_FUNC:
                
                break;
            case ITEM_CONST:
                break;
        }
    }

    IRprogram* ir_prog = malloc(sizeof(IRprogram));
    ir_prog->code = code;
    ir_prog->data = data;
    return OptionSome(ir_prog);
}


