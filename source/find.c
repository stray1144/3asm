#include "3asm.h"
#include <strings.h>
#include <sys/types.h>

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

const instruction_descriptor_t *instruction_find(char *mnemonic) {
	for(size_t i = 0; i < INSTRUCTION_COUNT; i++) {
		if(strcasecmp(mnemonic, instruction_lut[i].mnemonic) == 0) return &instruction_lut[i];
	}
	return nullptr;
}

ssize_t instruction_flag_find(char symbol, instruction_descriptor_t *descriptor) {
	for(ssize_t i = 0; i < descriptor->flag_count; i++) {
		if(descriptor->flags[i].symbol == symbol) return i; 
	}

	return -1;
}

register_descriptor_t *register_find(char *mnemonic) {
	for(size_t i = 0; i < REGISTER_COUNT; i++) {
		if(strcasecmp(mnemonic, register_lut[i].mnemonic) == 0) return &register_lut[i];
	}
	return nullptr;
}
