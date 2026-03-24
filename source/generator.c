#include "3asm.h"
#include <stdint.h>
#include <string.h>
#include <strings.h>

generator_result_t generator_result(generator_status_t status, uint32_t at) {
        return (generator_result_t) {status, at};
}


bool generator_emit_byte(translation_unit_t *output, uint8_t data, size_t size) {
        if(output->actual_section == REO_LOCATION_BLOCK) return false;

        buffer_t *section_buffer = nullptr;

        if(output->actual_section == REO_LOCATION_CODE) section_buffer = &output->code_section;
        if(output->actual_section == REO_LOCATION_DATA) section_buffer = &output->data_section;

        for(size_t i = 0; i < size; i++) buffer_append(section_buffer, &data, 1);

        return true;
}

bool generator_emit_blank(translation_unit_t *output, size_t size) {
        if(generator_emit_byte(output, 0x00, size) == false) output->block += size;

        return true;
}

bool generator_emit_data(translation_unit_t *output, void *data, size_t size) {
        if(output->actual_section == REO_LOCATION_BLOCK) return false;

        buffer_t *section_buffer = nullptr;

        if(output->actual_section == REO_LOCATION_CODE) section_buffer = &output->code_section;
        if(output->actual_section == REO_LOCATION_DATA) section_buffer = &output->data_section;

        buffer_append(section_buffer, data, size);

        return true;      
}

generator_result_t generator_section_handle(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        char *string = nullptr;
        semantizer_stream_peek(source, i, (void *)&string, nullptr);

        if(strcasecmp(string, "code") == 0) output->actual_section = REO_LOCATION_CODE;
        else if(strcasecmp(string, "data") == 0) output->actual_section = REO_LOCATION_DATA;
        else if(strcasecmp(string, "block") == 0) output->actual_section = REO_LOCATION_BLOCK;
        else return generator_result(GENERATOR_INVALID_SECTION, i);

        return generator_result(GENERATOR_OK, i);
}

generator_result_t generator_string_handle(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        char *string = nullptr;
        semantizer_stream_peek(source, i, (void *)&string, nullptr);

        if(generator_emit_data(output, string, strlen(string) + 1) == false) return generator_result(GENERATOR_BLOCK_WRITE, i);

        return generator_result(GENERATOR_OK, i);
}

generator_result_t generator_blank_handle(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        uint64_t *size = nullptr;
        semantizer_stream_peek(source, i, (void *)&size, nullptr);

        generator_emit_blank(output, *size);

        return generator_result(GENERATOR_OK, i);
}

reo_offset_t generator_section_offset(translation_unit_t *output) {
        if(output->actual_section == REO_LOCATION_CODE) return output->code_section.used;
        if(output->actual_section == REO_LOCATION_DATA) return output->data_section.used;
        if(output->actual_section == REO_LOCATION_BLOCK) return output->block;
        return 0;
}

generator_result_t generator_label_define(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        symbol_t symbol = {nullptr, output->actual_section, generator_section_offset(output)};
        semantizer_stream_steal(source, i, (void *)&symbol.name, nullptr);

        buffer_append(&output->symbols, &symbol, 1);

        return generator_result(GENERATOR_OK, i);
}

void generator_operation_code_write(translation_unit_t *output, const instruction_descriptor_t *descriptor) {
        uint8_t operation_code = descriptor->operation_code;
        generator_emit_byte(output, operation_code, 1);
}

bool generator_flag_write(translation_unit_t *output, instruction_representation_t *representation, const instruction_descriptor_t *descriptor) {
        if(descriptor->flag_count == 0) return true;

        uint8_t flag_byte = 0;

        if(representation->mnemonic->flags == nullptr) goto write_byte;

        for(size_t i = 0; i < strlen(representation->mnemonic->flags); i++) {
                ssize_t index = instruction_flag_find(representation->mnemonic->flags[i], descriptor);
                if(index == -1) return false;
                flag_byte |= descriptor->flags[index].encoding;
        }

write_byte:
        generator_emit_byte(output, flag_byte, 1);

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

void generator_size_calculate(operand_representation_t *operand) {
        if(operand->operand_size != 0) return; // the developer may have given a size, trust them.

        operand->operand_size = 1;

        if(operand->payload == 0) return;
        operand->operand_size = 1 << ((64 - (operand->payload << 63) ? __builtin_ctzll(operand->payload) : __builtin_clzll(operand->payload)) / 8);

        if(operand->kind & OPERAND_REFERENCE) operand->operand_size = 8;
}

void generator_operand_write(translation_unit_t *output, operand_representation_t *operand, operand_kind_t kind) {
        generator_size_calculate(operand);

        encoded_descriptor_t eod = {operand->kind & (OPERAND_REGISTER | OPERAND_VECTOR) ? operand->register_descriptor->encoding : 0, generator_size_encode(operand->operand_size)};

        generator_emit_data(output, &eod, 1);

        if(operand->kind & OPERAND_REFERENCE) {
                symbol_t symbol = {operand->symbol_name, output->actual_section, generator_section_offset(output)};

                buffer_append(&output->relocations, &symbol, 1);
        }
        
        if(kind & (OPERAND_IMMEDIATE | OPERAND_OFFSET_FLAG)) generator_emit_data(output, &operand->payload, operand->operand_size); 
}

generator_result_t generator_instruction_write(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        instruction_representation_t *representation = nullptr;
        semantizer_stream_peek(source, i, (void *)&representation, nullptr);

        const instruction_descriptor_t *descriptor = instruction_find(representation->mnemonic->root);

        size_t operand_count = (representation->operand_list) ? representation->operand_list->used : 0;

        if(operand_count != descriptor->operand_count) return generator_result(GENERATOR_INVALID_OPERAND_COUNT, i);

        generator_operation_code_write(output, descriptor);
        if(generator_flag_write(output, representation, descriptor) == false) return generator_result(GENERATOR_INVALID_FLAGS, i); 

        for(size_t j = 0; j < operand_count; j++) {
                operand_representation_t *operand = buffer_get(representation->operand_list, j);
                if(generator_offset_validate(operand) == false) return generator_result(GENERATOR_WRONG_OFFSET, i);
                generator_operand_write(output, operand, descriptor->operands[j]);
        }

        return generator_result(GENERATOR_OK, i);
}

generator_result_t generator_process(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        generator_result_t result = generator_result(GENERATOR_OK, i);

        switch(semantizer_stream_get(source, i)) {
                case SEMANTIC_SECTION_DIRECTIVE:
                result = generator_section_handle(output, source, i);
                break;

                case SEMANTIC_STRING_DIRECTIVE:
                result = generator_string_handle(output, source, i);
                break;

                case SEMANTIC_BLANK_DIRECTIVE:
                result = generator_blank_handle(output, source, i);
                break;

                case SEMANTIC_LABEL_DEFINITION:
                result = generator_label_define(output, source, i);
                break;

                case SEMANTIC_INSTRUCTION:
                result = generator_instruction_write(output, source, i);
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

generator_result_t generate(translation_unit_t *output, semantizer_t *source) {
        generator_result_t result = generator_result(GENERATOR_OK, 0);

        for(uint32_t i = 0; i < semantizer_stream_size(source); i++) {
                result = generator_process(output, source, i);
                if(result.status != GENERATOR_OK) break;
        }

        return result;
}










