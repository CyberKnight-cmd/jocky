#include "arena.h"
#include <stdlib.h>
#include <stdio.h>

void jky_arena_init(JkyArena *arena, size_t size) {
    arena->memory = (char*)malloc(size);
    if (!arena->memory) {
        fprintf(stderr, "Fatal error: Failed to allocate arena memory.\n");
        exit(1);
    }
    arena->size = size;
    arena->current_offset = 0;
}

void *jky_arena_alloc(JkyArena *arena, size_t size) {
    // 8-byte alignment for safety on 64-bit platforms
    size_t aligned_size = (size + 7) & ~7;
    
    if (arena->current_offset + aligned_size > arena->size) {
        fprintf(stderr, "Fatal error: Arena out of memory.\n");
        exit(1); // In a real compiler we'd allocate a new chunk, but for v0.1 we use one huge block
    }
    
    void *ptr = arena->memory + arena->current_offset;
    arena->current_offset += aligned_size;
    return ptr;
}

void jky_arena_free(JkyArena *arena) {
    if (arena->memory) {
        free(arena->memory);
        arena->memory = NULL;
    }
    arena->size = 0;
    arena->current_offset = 0;
}
