#include "3asm.h"
#include <stdio.h>

bool directive_mnemonic_match(semantizer_t *semantizer, size_t start) {
        bool match = semantizer_stream_match(semantizer, start, SEMANTIC_AT) &&
                     semantizer_stream_match(semantizer, start + 1, SEMANTIC_IDENTIFIER);

        if(!match) return false;

        char *string = nullptr;
        semantizer_stream_peek(semantizer, start + 1, (void *)&string, nullptr);

        return directive_mnemonic_exists(string);
}

size_t directive_mnemonic_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        semantizer_unit_init(unit, SEMANTIC_DIRECTIVE_MNEMONIC, nullptr, nullptr);
        semantizer_stream_steal(semantizer, start + 1, &unit->data, &unit->data_free);

        return 2;
}


#define PATTERN(name, level) {name##_match, name##_reduct, level}

semantizer_pattern_t patterns[PATTERN_COUNT] = {
        PATTERN(directive_mnemonic, 0),
}; 
