#include "3asm.h"
#include <stdio.h>

void output_symbol_build(reo_file_t *file, symbol_t *symbol) {
        reo_symbol_add(file, reo_string_add(file, symbol->name), symbol->location, (reo_symbol_type_t) {false, false, false, symbol->section, 0});
}

void output_relocation_build(reo_file_t *file, symbol_t *relocation) {
        reo_relocation_add(file, reo_string_add(file, relocation->name), relocation->location, REO_RELOCATION_ABSOLUTE);
}

void translation_unit_write(context_t *context, translation_unit_t *translation_unit) {
        reo_file_t file = {0};
        reo_file_init(&file);

        reo_type_set(&file, REO_TYPE_RELOCATABLE);

        reo_code_write(&file, buffer_get(&translation_unit->code_section, 0), translation_unit->code_section.used);
        reo_data_add(&file, buffer_get(&translation_unit->data_section, 0), translation_unit->data_section.used);
        reo_block_reserve(&file, translation_unit->block);

        for(size_t i = 0; i < translation_unit->symbols.used; i++) {
                symbol_t *symbol = buffer_get(&translation_unit->symbols, i);
                output_symbol_build(&file, symbol);
        }

        for(size_t i = 0; i < translation_unit->relocations.used; i++) {
                symbol_t *relocation = buffer_get(&translation_unit->relocations, i);
                output_relocation_build(&file, relocation);
        }

        reo_file_save(&file, context->settings.output_file);
        reo_file_clear(&file);
}
