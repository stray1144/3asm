// Copyright (C) 2026 Stray1144
// SPDX-License-Identifier: GPL-3.0-or-later

#include "3asm.h"
#include <libvacant/libvacant.h>
#include <stdio.h>
#include <stdlib.h>

void context_minimal_clear(context_t *context) {
        if(context == nullptr) return;

        logger_clear(&context->logger);
        argument_parser_clear(&context->AP);
}

bool context_minimal_init(context_t *context, int argc, char **argv) {
        if(context == nullptr) return false;

        context_minimal_clear(context);

        bool result;
        
        result = logger_init(&context->logger, LOGGER_WARN);
        if(result == false) {
                printf("Couldn't init the logger...\n");
                return false;
        }

        result = argument_parser_init(&context->AP, argc, argv);
        if(result == false) {
                system_fatal(context, "parameters", "Couldn't init the argument parser...");
                return false;
        }

        return true;
}

void shutdown(context_t *context, int exit_code) {
        context_minimal_clear(context);
        exit(exit_code);
}

void help(context_t *context) {
        system_error(context, "help", "not implemented");
}

int main(int argc, char **argv) {
        context_t context = {0};
        if(!context_minimal_init(&context, argc, argv)) shutdown(&context, -1);

        if(parameter_probe(&context.AP, "help", PARAMETER_FLAG)) {
                help(&context);
                shutdown(&context, 0);
        }

        if(parameter_probe(&context.AP, "verbose", PARAMETER_FLAG)) logger_level_change(&context.logger, LOGGER_VERBOSE);
        if(parameter_probe(&context.AP, "debug", PARAMETER_FLAG)) logger_level_change(&context.logger, LOGGER_DEBUG);
        if(parameter_probe(&context.AP, "quiet", PARAMETER_FLAG)) logger_level_change(&context.logger, LOGGER_SILENT);
        if(parameter_probe(&context.AP, "timestamp", PARAMETER_FLAG)) context.settings.logger_timestamp = true;

        translation_unit_t translation_unit = {0};

        char *path = parameter_positional_get(&context.AP, 1);

        if(translation_unit_init(&translation_unit, path) == false) {
                system_error(&context, "translation unit", "Couldn't init translation unit...");
                shutdown(&context, -1);
        }

        translation_unit_assemble(&context, &translation_unit, path);

        translation_unit_clear(&translation_unit);

        shutdown(&context, 0);
}
