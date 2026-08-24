#ifndef JOCKY_LEXER_H
#define JOCKY_LEXER_H

#include <stdint.h>
#include <stdbool.h>

// Token definitions exactly as specified in Chapter 16
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_KEYWORD_FN,
    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_IMPORT,
    TOKEN_KEYWORD_AUTO,
    TOKEN_KEYWORD_CONST,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BYTES_LITERAL,
    TOKEN_SEMICOLON,
    TOKEN_ARROW,       // ->
    TOKEN_BANG,        // !
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_ERROR        // Diagnostic error token
} JkyTokenType;

typedef struct {
    JkyTokenType type;
    const char  *start;       // Pointer into source buffer
    int          length;      // Token character length
    int          line;        // Source line number (1-indexed)
    int          column;      // Source column number (1-indexed)
    union {
        int64_t  int_val;
        double   float_val;
    } literal;
} JkyToken;

// Lexer state structure
typedef struct {
    const char *start;        // Beginning of the current token
    const char *current;      // Current character being looked at
    int line;
    int column;
} JkyLexer;

// Function prototypes
void jky_lexer_init(JkyLexer *lexer, const char *source);
JkyToken jky_lexer_next_token(JkyLexer *lexer);

#endif // JOCKY_LEXER_H
