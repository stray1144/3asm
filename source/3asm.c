#include "3asm.h"
#include <libvacant/libvacant.h>

void context_minimal_clear(context_t *context) {
        if(context == nullptr) return;

        logger_clear(&context->logger);
}

bool context_minimal_init(context_t *context) {
        if(context == nullptr) return false;

        context_minimal_clear(context);

        logger_init(&context->logger, LOGGER_WARN);

        return true;
}

int main(void) {
        context_t context = {0};
        context_minimal_init(&context);

        context_minimal_clear(&context);
}
