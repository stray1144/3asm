#include "3asm.h"
#include <libvacant/libvacant.h>

void context_minimal_clear(context_t *context) {
        if(context == nullptr) return;

        logger_clear(&context->logger);
}

bool context_minimal_init(context_t *context) {
        if(context == nullptr) return false;

        context_minimal_clear(context);

        bool result;
        
        result = logger_init(&context->logger, LOGGER_DEBUG);
        if(result == false) {
                printf("Couldn't init the logger...\n");
                return false;
        }

        return true;
}

void shutdown(context_t *context, int exit_code) {
        context_minimal_clear(context);
        exit(exit_code);
}

int main(void) {
        context_t context = {0};
        if(context_minimal_init(&context)) shutdown(&context, -1);


        shutdown(&context, 0);
}
