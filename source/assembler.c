// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#include "3asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

size_t file_size_get(char *path) {
        struct stat data;
        stat(path, &data);

        return data.st_size;
}

char *file_read(char *path) {
        size_t size = file_size_get(path);
        char *buffer = calloc(size, 1);

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

        bool result;
        result = lexer_init(&context->lexer, context->file_data);
        if(result == false) {
                system_fatal(context, "lexer", "Couldn't init the lexer...");
                return false;
        }



        return true;
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

        context_assembler_clear(context);

        return true;
}
