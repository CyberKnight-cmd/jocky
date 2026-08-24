#ifndef JOCKY_SEMA_H
#define JOCKY_SEMA_H

#include "ast.h"
#include <stdbool.h>

typedef enum {
    TYPE_INT,
    TYPE_VOID,
    TYPE_UNKNOWN
} JkyType;

// Represents an entry in the Symbol Table
typedef struct JkySymbol {
    const char *name;
    int name_length;
    JkyType type;
    struct JkySymbol *next; // Used for hash collision chaining
} JkySymbol;

#define SYMBOL_TABLE_SIZE 256

// The Semantic Analyzer State
typedef struct {
    JkySymbol *buckets[SYMBOL_TABLE_SIZE]; // The Hash Table
    JkyArena *arena; // We allocate symbols from our Arena!
    bool had_error;
} JkySema;

void jky_sema_init(JkySema *sema, JkyArena *arena);
void jky_sema_analyze(JkySema *sema, JkyAstNode *root);

#endif // JOCKY_SEMA_H
