// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#include "3asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

size_t file_size_get(char *path) {
        struct stat data;
        stat(path, &data);

        return data.st_size;
}

char *file_read(char *path) {
        size_t size = file_size_get(path);
        char *buffer = calloc(size + 1, 1);

        FILE *handle = fopen(path, "r");
        if(handle == nullptr) return nullptr;

        fread(buffer, 1, size, handle);
        fclose(handle);

        return buffer;
}

void context_assembler_clear(context_t *context) {
        if(context == nullptr) return;

        free(context->file_data);
        context->file_data = nullptr;

        lexer_clear(&context->lexer);
        buffer_clear(&context->tokens);

        semantizer_clear(&context->semantizer);
}

bool context_assembler_init(context_t *context, char *source_path) {
        if(context == nullptr) return false;

        context_assembler_clear(context);

        context->file_data = file_read(source_path);
        if(context->file_data == nullptr) {
                system_error(context, "file", "Couldn't read %s...", source_path);
                return false;
        }

        system_debug(context, "file", "%s", context->file_data);
        // TODO: multiline logger format

        if(lexer_init(&context->lexer, context->file_data) == false) {
                system_fatal(context, "lexer", "Couldn't init the lexer...");
                return false;
        }

        if(buffer_init(&context->tokens, sizeof(lexer_token_t)) == false) {
                system_fatal(context, "lexer", "Couldn't init the token buffer...");
                return false;
        }

        if(semantizer_init(&context->semantizer) == false) {
                system_fatal(context, "semantizer", "Couldn't init the semantizer...");
                return false;
        }

        return true;
}

void translation_unit_clear(translation_unit_t *translation_unit) {
        if(translation_unit == nullptr) return;

        buffer_clear(&translation_unit->code_section);
        buffer_clear(&translation_unit->data_section);


        for(size_t i = 0; i < translation_unit->symbols.used; i++) {
                symbol_t *symbol = buffer_get(&translation_unit->symbols, i);
                free(symbol->name);
        }
        buffer_clear(&translation_unit->symbols);
       
        for(size_t i = 0; i < translation_unit->relocations.used; i++) {
                symbol_t *symbol = buffer_get(&translation_unit->relocations, i);
                free(symbol->name);
        }
        buffer_clear(&translation_unit->relocations);

        memset(translation_unit, 0, sizeof(translation_unit_t));
}

bool translation_unit_init(translation_unit_t *translation_unit, char *name) {
        if(translation_unit == nullptr) return false;

        translation_unit_clear(translation_unit);

        translation_unit->name = name;

        bool result = true;
        result &= buffer_init(&translation_unit->code_section, sizeof(uint8_t));
        result &= buffer_init(&translation_unit->data_section, sizeof(uint8_t));

        result &= buffer_init(&translation_unit->symbols, sizeof(symbol_t));
        result &= buffer_init(&translation_unit->relocations, sizeof(symbol_t));

        translation_unit->actual_section = REO_LOCATION_CODE;

        return result;
}

static char *lexer_token_kind_names[LEXER_TOKEN_KIND_COUNT] = {
        "undefined",
        "comment",
        "identifier",
        "float",
        "hexadecimal",
        "integer",
        "string",
        "punctuation",
        "newline"
};

void tokens_get(context_t *context) {
        lexer_token_t token = {0};

        while(lex(&context->lexer, &token)) {
                system_debug(context, "lexer", "%s@%d", lexer_token_kind_names[token.kind], token.position);
                buffer_append(&context->tokens, &token, 1);
        }
}

void system_semantizer(context_t *context, char *message) {
        system_debug(context, "semantizer", "%s", message);
}

bool translation_unit_assemble(context_t *context, translation_unit_t *translation_unit, char *source_path) {
        if(source_path == nullptr) {
                system_error(context, "file", "No file");
                return false;
        }

        if(context_assembler_init(context, source_path) == false) {
                context_assembler_clear(context);
                return false;
        }

        system_verbose(context, "file", "Assembling %s...", source_path);

        tokens_get(context);

        semantizer_debug_setup(&context->semantizer, 
                               true, 
                               semantic_names, 9, 
                               (semantizer_log_callback_t *)system_semantizer, context);

        semantizer_forge_setup(&context->semantizer, forge_callbacks, FORGE_CALLBACK_COUNT);
        semantizer_forge_result_t forge_result = semantizer_forge_atomize(&context->semantizer, context->tokens.data, context->tokens.used);
        if(forge_result.status != FORGE_SUCCESS) {
                lexer_token_t *at = buffer_get(&context->tokens, forge_result.at);
                lexer_position_t line;
                lexer_position_t column;
                lexer_position_resolve(&context->lexer, at->position, &line, &column);
                system_error(context, "semantizer", "%s:%d:%d Unhandled %s token", source_path, line, column, lexer_token_kind_names[at->kind]);
                context_assembler_clear(context);
                return false;
        } // TODO: this block is too complex. reduce

        system_debug(context, "semantizer", "Starting semantizing...");

        semantizer_pattern_setup(&context->semantizer, patterns, PATTERN_COUNT, LEVEL_COUNT);
        semantize(&context->semantizer);

        generator_t generator = {0};
        generator.source = &context->semantizer;
        generator.output = translation_unit;

        generator_result_t generator_result = generate(&generator);
        if(generator_result.status  != GENERATOR_OK) {
                semantizer_unit_t *at = buffer_get(&context->semantizer.stream, generator_result.at);
                lexer_position_t line;
                lexer_position_t column;
                lexer_position_resolve(&context->lexer, at->position, &line, &column);
                system_error(context, "semantizer", "%s:%d:%d %s", source_path, line, column, generator_error_messages[generator_result.status]);
                context_assembler_clear(context);
                return false;
        }

        return true;
}
