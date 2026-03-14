// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#if !defined(__3Asm)
#define __3Asm

#include <cxtoolchain/libcxsh.h>
#include <libvacant/libvacant.h>

typedef struct settings_s {
        char *output_file;
        char *entry_symbol;

        bool logger_timestamp;
} settings_t;

typedef struct context_s {
        logger_t logger;
        argument_parser_t AP;
        
        settings_t settings;

        char *file_data;
        buffer_t tokens; // buffer<lexer_token_t>
        lexer_t lexer;
        semantizer_t semantizer;
} context_t;

typedef struct symbol_s {
        char *name;
        uint32_t section;
        reo_offset_t location;
} symbol_t;

enum semantizer_unit_kind_e : uint32_t {
        SEMANTIC_UNKNOWN,
        SEMANTIC_IDENTIFIER,
        SEMANTIC_NUMBER,
        SEMANTIC_COMMENT,
        SEMANTIC_STRING,
        SEMANTIC_NEWLINE,

        SEMANTIC_AT,
        SEMANTIC_DOLLAR,
        SEMANTIC_PERCENT,
        SEMANTIC_COMMA,
        SEMANTIC_DOT,
        SEMANTIC_COLON,

        SEMANTIC_LPAREN,
        SEMANTIC_RPAREN,
        SEMANTIC_LESSER,
        SEMANTIC_GREATER,

        SEMANTIC_PLUS,
        SEMANTIC_MINUS
};

static char *semantic_names[] = {
        "SEMANTIC_UNKNOWN",
        "SEMANTIC_IDENTIFIER",
        "SEMANTIC_NUMBER",
        "SEMANTIC_COMMENT",
        "SEMANTIC_STRING",
        "SEMANTIC_NEWLINE",

        "SEMANTIC_AT",
        "SEMANTIC_DOLLAR",
        "SEMANTIC_PERCENT",
        "SEMANTIC_COMMA",
        "SEMANTIC_DOT",
        "SEMANTIC_COLON",

        "SEMANTIC_LPAREN",
        "SEMANTIC_RPAREN",
        "SEMANTIC_LESSER",
        "SEMANTIC_GREATER",

        "SEMANTIC_PLUS",
        "SEMANTIC_MINUS"
};

#define FORGE_CALLBACK_COUNT 17
extern semantizer_forge_callback_t *forge_callbacks[FORGE_CALLBACK_COUNT];

typedef struct translation_unit_s {
        char *name;
        buffer_t code_section; // buffer<uint8_t>
        buffer_t data_section; // buffer<uint8_t>
        reo_size_t block;
        buffer_t symbols;      // buffer<symbol_t>
        buffer_t relocations;  // buffer<symbol_t>
} translation_unit_t;

bool translation_unit_init(translation_unit_t *translation_unit, char *name);
void translation_unit_clear(translation_unit_t *translation_unit);

bool translation_unit_assemble(context_t *context, translation_unit_t *translation_unit, char *source_path);

#define SYSTEM_LOGGER(name) system_##name(context_t *context, char *prefix, char *format, ...)

void SYSTEM_LOGGER(fatal); // system_fatal()
void SYSTEM_LOGGER(error); // system_error()
void SYSTEM_LOGGER(warn); // system_warn()
void SYSTEM_LOGGER(info); // system_info()
void SYSTEM_LOGGER(verbose); // system_verbose()
void SYSTEM_LOGGER(debug); // system_debug()


#endif
