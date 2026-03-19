#include "3asm.h"
#include <strings.h>

static char *directive_mnemonics[] = {
        "section",
        "store",
        "string",
        "blank",
        nullptr
};

bool directive_mnemonic_exists(char *mnemonic) {
        if(mnemonic == nullptr) return false;

        size_t i = 0;
        while(directive_mnemonics[i]) {
                if(strcasecmp(mnemonic, directive_mnemonics[i]) == 0) return true;
                i++;
        }
        return false;
}
