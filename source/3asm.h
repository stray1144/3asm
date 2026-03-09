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


#endif
