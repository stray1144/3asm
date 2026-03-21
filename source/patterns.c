#include "3asm.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

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
        semantizer_stream_trace(semantizer, unit, start);

        return 2;
}


bool instruction_mnemonic_match(semantizer_t *semantizer, size_t start) {
        bool match = semantizer_stream_match(semantizer, start, SEMANTIC_IDENTIFIER);

        if(!match) return false;

        char *string = nullptr;
        semantizer_stream_peek(semantizer, start, (void *)&string, nullptr);

        return instruction_find(string) != nullptr;
}

size_t instruction_mnemonic_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        semantizer_unit_init(unit, SEMANTIC_INSTRUCTION_MNEMONIC, nullptr, nullptr);
        semantizer_stream_steal(semantizer, start, &unit->data, &unit->data_free);
        semantizer_stream_trace(semantizer, unit, start);

        return 1;
}


bool size_definition_match(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_LESSER) &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_NUMBER) &&
                semantizer_stream_match(semantizer, start + 2, SEMANTIC_GREATER);
} 

size_t size_definition_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        semantizer_unit_init(unit, SEMANTIC_SIZE_DEFINITION, nullptr, SEMANTIZER_DATA_FREE_NONE);
        semantizer_stream_steal(semantizer, start + 1, &unit->data, nullptr);
        semantizer_stream_trace(semantizer, unit, start);

        return 3;
}

bool section_directive_match(semantizer_t *semantizer, size_t start) {
        if(semantizer_stream_match(semantizer, start, SEMANTIC_DIRECTIVE_MNEMONIC) == false) return false;

        char *string = nullptr;
        semantizer_stream_peek(semantizer, start, (void *)&string, nullptr);

        return  strcasecmp(string, "section") == 0 &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_IDENTIFIER) &&
                semantizer_stream_match(semantizer, start + 2, SEMANTIC_NEWLINE);
}

size_t section_directive_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        semantizer_unit_init(unit, SEMANTIC_SECTION_DIRECTIVE, nullptr, SEMANTIZER_DATA_FREE_NONE);
        semantizer_stream_steal(semantizer, start + 1, &unit->data, &unit->data_free);
        semantizer_stream_trace(semantizer, unit, start);

        return 3;
}





bool string_directive_match(semantizer_t *semantizer, size_t start) {
        if(semantizer_stream_match(semantizer, start, SEMANTIC_DIRECTIVE_MNEMONIC) == false) return false;

        char *string = nullptr;
        semantizer_stream_peek(semantizer, start, (void *)&string, nullptr);

        return  strcasecmp(string, "string") == 0 &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_STRING) &&
                semantizer_stream_match(semantizer, start + 2, SEMANTIC_NEWLINE);
}

size_t string_directive_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        semantizer_unit_init(unit, SEMANTIC_STRING_DIRECTIVE, nullptr, SEMANTIZER_DATA_FREE_NONE);
        semantizer_stream_steal(semantizer, start + 1, &unit->data, &unit->data_free);
        semantizer_stream_trace(semantizer, unit, start);

        return 3;
}

bool blank_directive_match(semantizer_t *semantizer, size_t start) {
        if(semantizer_stream_match(semantizer, start, SEMANTIC_DIRECTIVE_MNEMONIC) == false) return false;

        char *string = nullptr;
        semantizer_stream_peek(semantizer, start, (void *)&string, nullptr);

        bool is_end_valid = (semantizer_stream_match(semantizer, start + 1, SEMANTIC_SIZE_DEFINITION) && 
                             semantizer_stream_match(semantizer, start + 2, SEMANTIC_NEWLINE)) ||
                             semantizer_stream_match(semantizer, start + 1, SEMANTIC_NEWLINE);

        return  strcasecmp(string, "blank") == 0 &&
                is_end_valid;
}

size_t blank_directive_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        semantizer_unit_init(unit, SEMANTIC_BLANK_DIRECTIVE, nullptr, SEMANTIZER_DATA_FREE_NONE);
        semantizer_stream_steal(semantizer, start + 1, &unit->data, nullptr);
        semantizer_stream_trace(semantizer, unit, start);

        return 3;
}

bool label_definition_match(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_IDENTIFIER) &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_COLON);
}

size_t label_definition_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        semantizer_unit_init(unit, SEMANTIC_LABEL_DEFINITION, nullptr, SEMANTIZER_DATA_FREE_NONE);
        semantizer_stream_steal(semantizer, start, &unit->data, &unit->data_free);
        semantizer_stream_trace(semantizer, unit, start);

        return 2;
}





#define PATTERN(name, level) {name##_match, name##_reduct, level}

semantizer_pattern_t patterns[PATTERN_COUNT] = {
        PATTERN(directive_mnemonic, 0),
        // PATTERN(register_mnemonic, 0),
        PATTERN(instruction_mnemonic, 0),

        PATTERN(size_definition, 0),

        PATTERN(section_directive, 1),
        // store_directive
        PATTERN(string_directive, 1),
        PATTERN(blank_directive, 1),

        PATTERN(label_definition, 2)
}; 
