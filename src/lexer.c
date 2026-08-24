#include "lexer.h"
#include <string.h>
#include <ctype.h>

void jky_lexer_init(JkyLexer *lexer, const char *source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
}

static bool is_at_end(JkyLexer *lexer) {
    return *lexer->current == '\0';
}

static char advance(JkyLexer *lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->current[-1];
}

static char peek(JkyLexer *lexer) {
    return *lexer->current;
}

// Look at the NEXT character without consuming it
static char peek_next(JkyLexer *lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

static void skip_whitespace(JkyLexer *lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 1;
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    // A single-line comment goes to the end of the line.
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) advance(lexer);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static JkyToken make_token(JkyLexer *lexer, JkyTokenType type) {
    JkyToken token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    // Calculate the column where the token started
    token.column = lexer->column - token.length;
    return token;
}

static JkyToken error_token(JkyLexer *lexer, const char* message) {
    JkyToken token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

// Checks if the current token matches a keyword, given we already know the first character.
static JkyTokenType check_keyword(JkyLexer *lexer, int total_length, const char *suffix, JkyTokenType type) {
    // 1. Check total length first (1 CPU cycle). If it doesn't match, bail out immediately.
    if (lexer->current - lexer->start != total_length) {
        return TOKEN_IDENTIFIER;
    }
    
    // 2. Compare the suffix. 
    // total_length - 1 is the length of the suffix (e.g., "uto" is 3 chars, total "auto" is 4)
    if (memcmp(lexer->start + 1, suffix, total_length - 1) == 0) {
        return type;
    }
    
    return TOKEN_IDENTIFIER;
}

static JkyTokenType identifier_type(JkyLexer *lexer) {
    switch (lexer->start[0]) {
        case 'a': return check_keyword(lexer, 4, "uto", TOKEN_KEYWORD_AUTO);
        case 'c': return check_keyword(lexer, 5, "onst", TOKEN_KEYWORD_CONST);
        case 'e': return check_keyword(lexer, 4, "lse", TOKEN_KEYWORD_ELSE);
        case 'f':
            if (lexer->current - lexer->start == 2) return TOKEN_KEYWORD_FN; // "fn" has no suffix
            if (lexer->current - lexer->start == 3) return check_keyword(lexer, 3, "or", TOKEN_KEYWORD_FOR);
            break;
        case 'i':
            if (lexer->current - lexer->start == 2) return TOKEN_KEYWORD_IF; // "if" has no suffix
            if (lexer->current - lexer->start == 6) return check_keyword(lexer, 6, "mport", TOKEN_KEYWORD_IMPORT);
            break;
        case 'r': return check_keyword(lexer, 6, "eturn", TOKEN_KEYWORD_RETURN);
        case 's': return check_keyword(lexer, 6, "truct", TOKEN_KEYWORD_STRUCT);
        case 'w': return check_keyword(lexer, 5, "hile", TOKEN_KEYWORD_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

JkyToken jky_lexer_next_token(JkyLexer *lexer) {
    skip_whitespace(lexer);

    lexer->start = lexer->current;

    if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);

    char c = advance(lexer);

    if (isalpha(c) || c == '_') {
        while (isalnum(peek(lexer)) || peek(lexer) == '_') advance(lexer);
        return make_token(lexer, identifier_type(lexer));
    }

    if (isdigit(c)) {
        while (isdigit(peek(lexer))) advance(lexer);
        // Note: Minimal integer parsing
        return make_token(lexer, TOKEN_INT_LITERAL);
    }

    if (c == '"') {
        while (peek(lexer) != '"' && !is_at_end(lexer)) {
            if (peek(lexer) == '\n') lexer->line++;
            advance(lexer);
        }
        if (is_at_end(lexer)) return error_token(lexer, "Unterminated string.");
        advance(lexer); // closing quote
        return make_token(lexer, TOKEN_STRING_LITERAL);
    }

    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case '!': return make_token(lexer, TOKEN_BANG);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '/': return make_token(lexer, TOKEN_SLASH);
        case '<': return make_token(lexer, TOKEN_LESS);
        case '>': return make_token(lexer, TOKEN_GREATER);
        case '.': return make_token(lexer, TOKEN_DOT);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '=':
            if (peek(lexer) == '=') {
                advance(lexer);
                return make_token(lexer, TOKEN_EQUAL_EQUAL);
            }
            return make_token(lexer, TOKEN_EQUAL);
        case '-':
            if (peek(lexer) == '>') {
                advance(lexer);
                return make_token(lexer, TOKEN_ARROW);
            }
            return make_token(lexer, TOKEN_MINUS);
    }

    return error_token(lexer, "Unexpected character.");
}
