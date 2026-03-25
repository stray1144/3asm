#include "3asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool forge_identifier_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if(token->kind != LEXER_TOKEN_IDENTIFIER) return false;

        semantizer_unit_init(unit, SEMANTIC_IDENTIFIER, strndup(token->string.base, token->string.length), free);
        semantizer_token_trace(unit, token);

        return true;
}

bool forge_number_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if((token->kind != LEXER_TOKEN_INTEGER_LITERAL) && (token->kind != LEXER_TOKEN_HEXADECIMAL_LITERAL)) return false;

        semantizer_unit_init(unit, SEMANTIC_NUMBER, &token->number, SEMANTIZER_DATA_FREE_NONE);
        semantizer_token_trace(unit, token);

        return true;
}

bool forge_comment_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if(token->kind != LEXER_TOKEN_COMMENT) return false;

        semantizer_unit_init(unit, SEMANTIC_COMMENT, strndup(token->string.base, token->string.length), free);
        semantizer_token_trace(unit, token);

        return true;
}

bool forge_string_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if(token->kind != LEXER_TOKEN_STRING_LITERAL) return false;

        semantizer_unit_init(unit, SEMANTIC_STRING, strndup(token->string.base + 1, token->string.length - 2), free);
        semantizer_token_trace(unit, token);

        return true;
}

bool forge_newline_handle(semantizer_unit_t *unit, lexer_token_t *token) {
        if(token->kind != LEXER_TOKEN_NEWLINE) return false;

        semantizer_unit_init(unit, SEMANTIC_NEWLINE, nullptr, SEMANTIZER_DATA_FREE_NONE);
        semantizer_token_trace(unit, token);

        return true;
}

// clever shit. i'm lazy
#define PUNCTUATION_HANDLER_NAME(name) forge_##name##_handle
#define PUNCTUATION_HANDLER(name) PUNCTUATION_HANDLER_NAME(name) (semantizer_unit_t *unit, lexer_token_t *token)
#define DEFINE_PUNCTUATION_HANDLER(name, ascii, semantic_kind) \
bool PUNCTUATION_HANDLER(name) { \
        if(token->kind != LEXER_TOKEN_PUNCTUATION || token->character != ascii) return false; \
        semantizer_unit_init(unit, semantic_kind, nullptr, SEMANTIZER_DATA_FREE_NONE); \
        semantizer_token_trace(unit, token); \
        return true; \
}

DEFINE_PUNCTUATION_HANDLER(at, '@', SEMANTIC_AT)
DEFINE_PUNCTUATION_HANDLER(dollar, '$', SEMANTIC_DOLLAR)
DEFINE_PUNCTUATION_HANDLER(percent, '%', SEMANTIC_PERCENT)
DEFINE_PUNCTUATION_HANDLER(comma, ',', SEMANTIC_COMMA)
DEFINE_PUNCTUATION_HANDLER(dot, '.', SEMANTIC_DOT)
DEFINE_PUNCTUATION_HANDLER(colon, ':', SEMANTIC_COLON)

DEFINE_PUNCTUATION_HANDLER(lparen, '(', SEMANTIC_LPAREN)
DEFINE_PUNCTUATION_HANDLER(rparen, ')', SEMANTIC_RPAREN)
DEFINE_PUNCTUATION_HANDLER(lesser, '<', SEMANTIC_LESSER)
DEFINE_PUNCTUATION_HANDLER(greater, '>', SEMANTIC_GREATER)

DEFINE_PUNCTUATION_HANDLER(plus, '+', SEMANTIC_PLUS)
DEFINE_PUNCTUATION_HANDLER(minus, '-', SEMANTIC_MINUS)

semantizer_forge_callback_t *forge_callbacks[FORGE_CALLBACK_COUNT] = {
        forge_identifier_handle,
        forge_number_handle,
        forge_comment_handle,
        forge_string_handle,
        forge_newline_handle,

        PUNCTUATION_HANDLER_NAME(at),
        PUNCTUATION_HANDLER_NAME(dollar),
        PUNCTUATION_HANDLER_NAME(percent),
        PUNCTUATION_HANDLER_NAME(comma),
        PUNCTUATION_HANDLER_NAME(dot),
        PUNCTUATION_HANDLER_NAME(colon),

        PUNCTUATION_HANDLER_NAME(lparen),
        PUNCTUATION_HANDLER_NAME(rparen),
        PUNCTUATION_HANDLER_NAME(lesser),
        PUNCTUATION_HANDLER_NAME(greater),

        PUNCTUATION_HANDLER_NAME(plus),
        PUNCTUATION_HANDLER_NAME(minus),
};
