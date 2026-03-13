#include "3asm.h"

#include <stdlib.h>
#include <string.h>

bool forge_identifier_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if(token->kind != LEXER_TOKEN_IDENTIFIER) return false;

        semantizer_unit_init(unit, SEMANTIC_IDENTIFIER, strndup(token->string.base, token->string.length), free);

        return true;
}

bool forge_number_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if((token->kind != LEXER_TOKEN_INTEGER_LITERAL) && (token->kind != LEXER_TOKEN_HEXADECIMAL_LITERAL)) return false;

        semantizer_unit_init(unit, SEMANTIC_NUMBER, &token->number, SEMANTIZER_DATA_FREE_NONE);

        return true;
}

bool forge_comment_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if(token->kind != LEXER_TOKEN_COMMENT) return false;

        semantizer_unit_init(unit, SEMANTIC_COMMENT, strndup(token->string.base, token->string.length), free);

        return true;
}

bool forge_newline_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if(token->kind != LEXER_TOKEN_NEWLINE) return false;

        semantizer_unit_init(unit, SEMANTIC_NEWLINE, nullptr, SEMANTIZER_DATA_FREE_NONE);

        return true;
}

semantizer_forge_callback_t *forge_callbacks[FORGE_CALLBACK_COUNT] = {
        forge_identifier_handle,
        forge_number_handle,
        forge_comment_handle,
        forge_newline_handle
};
