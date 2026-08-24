#ifndef JOCKY_CODEGEN_H
#define JOCKY_CODEGEN_H

#include "ast.h"

// Dynamic string buffer to hold our generated C code entirely in RAM
typedef struct {
    char *data;
    size_t capacity;
    size_t length;
} JkyBuffer;

void jky_buffer_init(JkyBuffer *buf);
void jky_buffer_append(JkyBuffer *buf, const char *str);
void jky_buffer_free(JkyBuffer *buf);

// Generates C source string from the validated AST
void jky_codegen_generate(JkyBuffer *buf, JkyAstNode *root);

// Pipes the in-memory buffer to the backend compiler
void jky_codegen_compile_and_pipe(JkyBuffer *buf, const char *output_exe);

#endif // JOCKY_CODEGEN_H
