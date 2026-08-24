#ifndef JOCKY_ARENA_H
#define JOCKY_ARENA_H

#include <stddef.h>

typedef struct {
    char *memory;
    size_t size;
    size_t current_offset;
} JkyArena;

void jky_arena_init(JkyArena *arena, size_t size);
void *jky_arena_alloc(JkyArena *arena, size_t size);
void jky_arena_free(JkyArena *arena);

#endif // JOCKY_ARENA_H
