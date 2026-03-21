#include "3asm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void instruction_mnemonic_destroy(instruction_mnemonic_t *mnemonic) {
        if(mnemonic == nullptr) return;

        if(mnemonic->root) free(mnemonic->root);
        if(mnemonic->flags) free(mnemonic->flags);
        free(mnemonic);
}

size_t instruction_mnemonic_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        instruction_mnemonic_t *representation = calloc(1, sizeof(instruction_mnemonic_t));

        semantizer_stream_steal(semantizer, start, (void *)&representation->root, nullptr);

        semantizer_unit_init(unit, SEMANTIC_INSTRUCTION_MNEMONIC, representation, (semantizer_data_free_t *)instruction_mnemonic_destroy);
        semantizer_stream_trace(semantizer, unit, start);

        return 1;
}

bool register_operand_match(semantizer_t *semantizer, size_t start) {
        bool match = semantizer_stream_match(semantizer, start, SEMANTIC_PERCENT) &&
                     semantizer_stream_match(semantizer, start + 1, SEMANTIC_IDENTIFIER);

        if(!match) return false;

        char *string = nullptr;
        semantizer_stream_peek(semantizer, start + 1, (void *)&string, nullptr);

        return register_find(string) != nullptr;
}

size_t register_operand_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        char *mnemonic = nullptr;
        semantizer_stream_peek(semantizer, start + 1, (void *)&mnemonic, nullptr);

        register_descriptor_t *descriptor = register_find(mnemonic);

        operand_representation_t *representation = calloc(1, sizeof(operand_representation_t));
        representation->kind = descriptor->kind;
        representation->register_encoding = descriptor->encoding;

        semantizer_unit_init(unit, SEMANTIC_OPERAND, representation, free);
        semantizer_stream_trace(semantizer, unit, start);

        return 2;
}

bool immediate_operand_match(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_DOLLAR) &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_NUMBER);
}

size_t immediate_operand_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        int64_t *immediate = nullptr;
        semantizer_stream_peek(semantizer, start + 1, (void *)&immediate, nullptr);

        operand_representation_t *representation = calloc(1, sizeof(operand_representation_t));
        representation->kind = OPERAND_IMMEDIATE;
        representation->payload = *immediate;

        semantizer_unit_init(unit, SEMANTIC_OPERAND, representation, free);
        semantizer_stream_trace(semantizer, unit, start);

        return 2;
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

bool apply_size_match(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_OPERAND) &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_SIZE_DEFINITION);
}

size_t apply_size_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        operand_representation_t *representation = nullptr;
        uint64_t *size = nullptr;

        semantizer_stream_steal(semantizer, start, (void *)&representation, nullptr);
        semantizer_stream_steal(semantizer, start + 1, (void *)&size, nullptr);

        representation->operand_size = *size;

        semantizer_unit_init(unit, SEMANTIC_OPERAND, representation, free);
        semantizer_stream_trace(semantizer, unit, start);

        return 2;
}

bool apply_offset_match(semantizer_t *semantizer, size_t start) {
        bool is_lhs_valid = semantizer_stream_match(semantizer, start, SEMANTIC_LPAREN)&&
                            semantizer_stream_match(semantizer, start + 1, SEMANTIC_OPERAND);
        bool is_operator_valid = semantizer_stream_match(semantizer, start + 2, SEMANTIC_PLUS)||
                                 semantizer_stream_match(semantizer, start + 2, SEMANTIC_MINUS);
        bool is_rhs_valid = semantizer_stream_match(semantizer, start + 3, SEMANTIC_OPERAND)&&
                            semantizer_stream_match(semantizer, start + 4, SEMANTIC_RPAREN);

        return is_lhs_valid && is_operator_valid && is_rhs_valid;
}

size_t apply_offset_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        operand_representation_t *lhs = nullptr;
        operand_representation_t *rhs = nullptr;

        semantizer_stream_peek(semantizer, start + 1, (void *)&lhs, nullptr);
        semantizer_stream_peek(semantizer, start + 3, (void *)&rhs, nullptr);

        operand_representation_t *product = calloc(1, sizeof(operand_representation_t));
        product->kind = lhs->kind | rhs->kind | OPERAND_OFFSET_FLAG;
        product->register_encoding = lhs->register_encoding; // TODO: reject the weird case of (%reg +/- %reg), which implies a cpu "magic register arithmetic operand"
        product->operand_size = rhs->operand_size; // payload size
        product->payload = (semantizer_stream_match(semantizer, start + 2, SEMANTIC_PLUS)) ?
                           lhs->payload + rhs->payload : lhs->payload - rhs->payload;

        semantizer_unit_init(unit, SEMANTIC_OPERAND, product, free);
        semantizer_stream_trace(semantizer, unit, start);

        return 5;
}

bool apply_flags_match(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_INSTRUCTION_MNEMONIC) &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_DOT) &&
                semantizer_stream_match(semantizer, start + 2, SEMANTIC_IDENTIFIER);
}

size_t apply_flags_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        instruction_mnemonic_t *mnemonic = nullptr;
        char *flags = nullptr;

        semantizer_stream_steal(semantizer, start, (void *)&mnemonic, nullptr);
        semantizer_stream_steal(semantizer, start + 2, (void *)&flags, nullptr);

        mnemonic->flags = flags;

        semantizer_unit_init(unit, SEMANTIC_INSTRUCTION_MNEMONIC, mnemonic, (semantizer_data_free_t *)instruction_mnemonic_destroy);
        semantizer_stream_trace(semantizer, unit, start);

        return 3;
}

bool operand_list_separator_check(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_COMMA) ||
                semantizer_stream_match(semantizer, start, SEMANTIC_NEWLINE);

}

void buffer_destroy(buffer_t *buffer) {
        if(buffer == nullptr) return;

        buffer_clear(buffer);
        free(buffer);
}

bool operand_list_match(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_OPERAND) &&
                operand_list_separator_check(semantizer, start + 1);
}

size_t operand_list_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        buffer_t *list = calloc(1, sizeof(buffer_t));
        buffer_init(list, sizeof(operand_representation_t));

        size_t count = 0;

        while(true) {
                size_t cursor = start + (count * 2);
                operand_representation_t *representation = nullptr;

                if(operand_list_match(semantizer, cursor) == false) break;

                semantizer_stream_steal(semantizer, cursor, (void *)&representation, nullptr);
                buffer_append(list, representation, 1);
                free(representation);

                count++;

                if(semantizer_stream_match(semantizer, cursor + 1, SEMANTIC_NEWLINE)) break;
        }

        semantizer_unit_init(unit, SEMANTIC_OPERAND_LIST, list, (semantizer_data_free_t *)buffer_destroy);
        semantizer_stream_trace(semantizer, unit, start);

        return (count * 2) - 1;
}

void instruction_representation_destroy(instruction_representation_t *representation) {
        if(representation == nullptr) return;

        instruction_mnemonic_destroy(representation->mnemonic);
        buffer_destroy(representation->operand_list);
        free(representation);
}

bool instruction_match(semantizer_t *semantizer, size_t start) {
        return  semantizer_stream_match(semantizer, start, SEMANTIC_INSTRUCTION_MNEMONIC) &&
                semantizer_stream_match(semantizer, start + 1, SEMANTIC_OPERAND_LIST);
}

size_t instruction_reduct(semantizer_t *semantizer, semantizer_unit_t *unit, size_t start) {
        instruction_representation_t *representation = calloc(1, sizeof(instruction_representation_t));

        semantizer_stream_steal(semantizer, start, (void *)&representation->mnemonic, nullptr);
        semantizer_stream_steal(semantizer, start + 1, (void *)&representation->operand_list, nullptr);

        semantizer_unit_init(unit, SEMANTIC_INSTRUCTION, representation, (semantizer_data_free_t *)instruction_representation_destroy);
        semantizer_stream_trace(semantizer, unit, start);

        return 2;
}

#define PATTERN(name, level) {name##_match, name##_reduct, level}

semantizer_pattern_t patterns[PATTERN_COUNT] = {
        PATTERN(directive_mnemonic, 0),
        PATTERN(instruction_mnemonic, 0),
        PATTERN(size_definition, 0),
        PATTERN(register_operand, 0),
        PATTERN(immediate_operand, 0),

        PATTERN(section_directive, 1),
        // store_directive
        PATTERN(string_directive, 1),
        PATTERN(blank_directive, 1),

        PATTERN(label_definition, 2),

        PATTERN(apply_size, 3),
        PATTERN(apply_offset, 3),
        PATTERN(apply_flags, 3),

        PATTERN(operand_list, 4),

        PATTERN(instruction, 5),
}; 
