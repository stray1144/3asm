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
        lexer_t lexer;
        semantizer_t semantizer;
} context_t;

#define SYSTEM_LOGGER(name) system_##name(context_t *context, char *prefix, char *format, ...)

void SYSTEM_LOGGER(fatal); // system_fatal()
void SYSTEM_LOGGER(error); // system_error()
void SYSTEM_LOGGER(warn); // system_warn()
void SYSTEM_LOGGER(info); // system_info()
void SYSTEM_LOGGER(verbose); // system_verbose()
void SYSTEM_LOGGER(debug); // system_debug()


#endif
