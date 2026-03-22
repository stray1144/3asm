#include "3asm.h"
#include <stdint.h>
#include <stdio.h>
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

        for(size_t i = 0; i < size; i++) buffer_append(&output->data_section, &data, 1);

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

        buffer_append(&output->data_section, data, size);

        return true;      
}

generator_result_t generator_process(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        generator_result_t result = generator_result(GENERATOR_OK, i);

        switch(semantizer_stream_get(source, i)) {

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
                semantizer_unit_kind_t kind = semantizer_stream_get(source, i);
                result = generator_process(output, source, i);
                if(result.status != GENERATOR_OK) break;
        }

        return result;
}










