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
        
        result = logger_init(&context->logger, LOGGER_DEBUG);
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

int main(int argc, char **argv) {
        context_t context = {0};
        if(context_minimal_init(&context, argc, argv)) shutdown(&context, -1);


        shutdown(&context, 0);
}
