#ifndef JOCKY_PARSER_H
#define JOCKY_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    JkyLexer *lexer;
    JkyToken current_token;
    JkyToken previous_token;
    JkyArena *arena;
    bool had_error;
} JkyParser;

void jky_parser_init(JkyParser *parser, JkyLexer *lexer, JkyArena *arena);
JkyAstNode* jky_parse(JkyParser *parser);

#endif // JOCKY_PARSER_H
