#include "3asm.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

generator_result_t generator_result(generator_status_t status, uint32_t at) {
        return (generator_result_t) {status, at};
}

buffer_t *generator_buffer_get(generator_t *generator) {
        if(generator->output->actual_section == REO_LOCATION_CODE) return &generator->output->code_section;
        if(generator->output->actual_section == REO_LOCATION_DATA) return &generator->output->data_section;

        return nullptr;
}

void generator_symbol_grow(generator_t *generator, reo_size_t size) {
        if(generator == nullptr || generator->working_symbol == nullptr) return;
        generator->working_symbol->size += size;
}

bool generator_emit_byte(generator_t *generator, uint8_t data, size_t size) {
        buffer_t *section_buffer = generator_buffer_get(generator);
        if(section_buffer == nullptr) return false;

        for(size_t i = 0; i < size; i++) buffer_append(section_buffer, &data, 1);

        generator_symbol_grow(generator, size);

        return true;
}

bool generator_emit_blank(generator_t *generator, size_t size) {
        if(generator_emit_byte(generator, 0x00, size) == false) { 
                generator->output->block += size;
                generator_symbol_grow(generator, size);
        }

        return true;
}

bool generator_emit_data(generator_t *generator, void *data, size_t size) {
        buffer_t *section_buffer = generator_buffer_get(generator);
        if(section_buffer == nullptr) return false;

        buffer_append(section_buffer, data, size);

        generator_symbol_grow(generator, size);

        return true;
}

generator_result_t generator_section_handle(generator_t *generator, uint32_t i) {
        char *string = nullptr;
        semantizer_stream_peek(generator->source, i, (void *)&string, nullptr);

        if(strcasecmp(string, "code") == 0) generator->output->actual_section = REO_LOCATION_CODE;
        else if(strcasecmp(string, "data") == 0) generator->output->actual_section = REO_LOCATION_DATA;
        else if(strcasecmp(string, "block") == 0) generator->output->actual_section = REO_LOCATION_BLOCK;
        else return generator_result(GENERATOR_INVALID_SECTION, i);

        return generator_result(GENERATOR_OK, i);
}

size_t generator_size_calculate(uint64_t number) {
        if(number == 0) return 1;
        return 1 << ((64 - (number << 63) ? __builtin_ctzll(number) : __builtin_clzll(number)) / 8);
}

generator_result_t generator_store_handle(generator_t *generator, uint32_t i) {
        store_representation_t *representation = nullptr;
        semantizer_stream_peek(generator->source, i, (void *)&representation, nullptr);

        size_t size = representation->size;
        if(size == 0) size = generator_size_calculate(representation->value);
        if(size > 8) size = 8;

        generator_emit_data(generator, &representation->value, size);

        return generator_result(GENERATOR_OK, i);
}

generator_result_t generator_string_handle(generator_t *generator, uint32_t i) {
        char *string = nullptr;
        semantizer_stream_peek(generator->source, i, (void *)&string, nullptr);

        if(generator_emit_data(generator, string, strlen(string) + 1) == false) return generator_result(GENERATOR_BLOCK_WRITE, i);

        return generator_result(GENERATOR_OK, i);
}

generator_result_t generator_blank_handle(generator_t *generator, uint32_t i) {
        uint64_t *size = nullptr;
        semantizer_stream_peek(generator->source, i, (void *)&size, nullptr);

        generator_emit_blank(generator, *size);

        return generator_result(GENERATOR_OK, i);
}

reo_offset_t generator_section_offset(translation_unit_t *output) {
        if(output->actual_section == REO_LOCATION_CODE) return output->code_section.used;
        if(output->actual_section == REO_LOCATION_DATA) return output->data_section.used;
        if(output->actual_section == REO_LOCATION_BLOCK) return output->block;
        return 0;
}

void generator_symbol_switch(generator_t *generator, symbol_t *symbol) {
        generator->working_symbol = symbol;
}

generator_result_t generator_label_define(generator_t *generator, uint32_t i) {
        symbol_t symbol = {nullptr, generator->output->actual_section, generator_section_offset(generator->output), 0};
        semantizer_stream_steal(generator->source, i, (void *)&symbol.name, nullptr);

        buffer_append(&generator->output->symbols, &symbol, 1);

        generator_symbol_switch(generator, buffer_get(&generator->output->symbols, generator->output->symbols.used - 1)); // get last buffer object. implement a helper later

        return generator_result(GENERATOR_OK, i);
}

void generator_operation_code_write(generator_t *generator, const instruction_descriptor_t *descriptor) {
        uint8_t operation_code = descriptor->operation_code;
        generator_emit_byte(generator, operation_code, 1);
}

bool generator_flag_write(generator_t *generator, instruction_representation_t *representation, const instruction_descriptor_t *descriptor) {
        if(descriptor->flag_count == 0) return true;

        uint8_t flag_byte = 0;

        if(representation->mnemonic->flags == nullptr) goto write_byte;

        for(size_t i = 0; i < strlen(representation->mnemonic->flags); i++) {
                ssize_t index = instruction_flag_find(representation->mnemonic->flags[i], descriptor);
                if(index == -1) return false;
                flag_byte |= descriptor->flags[index].encoding;
        }

write_byte:
        generator_emit_byte(generator, flag_byte, 1);

        return true;
}

bool generator_offset_validate(operand_representation_t *operand) {
        if(operand->kind & OPERAND_REGISTER && operand->kind & OPERAND_REFERENCE) return false;
        if(operand->kind & OPERAND_VECTOR && operand->kind & OPERAND_REFERENCE) return false;

        if(operand->kind & OPERAND_REGISTER && operand->kind & OPERAND_VECTOR) return false;
        if(operand->kind & OPERAND_VECTOR && operand->kind & OPERAND_OFFSET_FLAG) return false;

        return true;
}

uint8_t generator_size_encode(uint32_t size) {
        return (31 - __builtin_clzll(size));
}



void generator_operand_size(operand_representation_t *operand) {
        if(operand->operand_size != 0) return; // the developer may have given a size, trust them.

        operand->operand_size = generator_size_calculate(operand->payload); 

        if(operand->kind & OPERAND_REFERENCE) operand->operand_size = 8;
}

void generator_operand_write(generator_t *generator, operand_representation_t *operand, operand_kind_t kind) {
        generator_operand_size(operand);

        encoded_descriptor_t eod = {operand->kind & (OPERAND_REGISTER | OPERAND_VECTOR) ? operand->register_descriptor->encoding : 0, generator_size_encode(operand->operand_size)};

        generator_emit_data(generator, &eod, 1);

        if(operand->kind & OPERAND_REFERENCE) {
                symbol_t symbol = {operand->symbol_name, generator->output->actual_section, generator_section_offset(generator->output), 0};

                buffer_append(&generator->output->relocations, &symbol, 1);
        }
        
        if(kind & (OPERAND_IMMEDIATE | OPERAND_OFFSET_FLAG)) generator_emit_data(generator, &operand->payload, operand->operand_size); 
}

generator_result_t generator_instruction_write(generator_t *generator, uint32_t semantic_index) {
        instruction_representation_t *representation = nullptr;
        semantizer_stream_peek(generator->source, semantic_index, (void *)&representation, nullptr);

        const instruction_descriptor_t *descriptor = instruction_find(representation->mnemonic->root);

        size_t operand_count = (representation->operand_list) ? representation->operand_list->used : 0;

        if(operand_count != descriptor->operand_count) return generator_result(GENERATOR_INVALID_OPERAND_COUNT, semantic_index);

        generator_operation_code_write(generator, descriptor);
        if(generator_flag_write(generator, representation, descriptor) == false) return generator_result(GENERATOR_INVALID_FLAGS, semantic_index); 

        for(size_t i = 0; i < operand_count; i++) {
                operand_representation_t *operand = buffer_get(representation->operand_list, i);
                if(generator_offset_validate(operand) == false) return generator_result(GENERATOR_WRONG_OFFSET, semantic_index);
                generator_operand_write(generator, operand, descriptor->operands[i]);
        }

        return generator_result(GENERATOR_OK, semantic_index);
}

generator_result_t generator_process(generator_t *generator, uint32_t i) {
        generator_result_t result = generator_result(GENERATOR_OK, i);

        switch(semantizer_stream_get(generator->source, i)) {
                case SEMANTIC_SECTION_DIRECTIVE:
                result = generator_section_handle(generator, i);
                break;

                case SEMANTIC_STORE_DIRECTIVE:
                result = generator_store_handle(generator, i);
                break;

                case SEMANTIC_STRING_DIRECTIVE:
                result = generator_string_handle(generator, i);
                break;

                case SEMANTIC_BLANK_DIRECTIVE:
                result = generator_blank_handle(generator, i);
                break;

                case SEMANTIC_LABEL_DEFINITION:
                result = generator_label_define(generator, i);
                break;

                case SEMANTIC_INSTRUCTION:
                result = generator_instruction_write(generator, i);
                break;

                case SEMANTIC_COMMENT:
                break;

                case SEMANTIC_NEWLINE:
                break;

                default:
                result = generator_result(GENERATOR_UNEXPECTED, i);
                break;
        }

        return result;
}

generator_result_t generate(generator_t *generator) {
        generator_result_t result = generator_result(GENERATOR_OK, 0);

        for(uint32_t i = 0; i < semantizer_stream_size(generator->source); i++) {
                result = generator_process(generator, i);
                if(result.status != GENERATOR_OK) break;
        }

        return result;
}
