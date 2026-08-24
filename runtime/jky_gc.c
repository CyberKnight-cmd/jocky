/* JOCKY Runtime — GC / Part of libjocky. / Garbage Collector Implementation */
#include "jky_gc.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct GCObject {
    void *ptr;
    JkyKind kind;
    struct GCObject *next;
} GCObject;

static GCObject *head = NULL;
static void **roots = NULL;
static int roots_count = 0;
static int roots_cap = 0;

void jky_gc_init(void) {
    head = NULL;
    roots_cap = 1024;
    roots = malloc(sizeof(void*) * roots_cap);
    roots_count = 0;
}

void jky_gc_register(void *obj, JkyKind kind) {
    if (!obj) return;
    GCObject *node = malloc(sizeof(GCObject));
    if (!node) return;
    node->ptr = obj;
    node->kind = kind;
    node->next = head;
    head = node;
}

void jky_gc_push_root(void *obj) {
    if (!obj) return;
    if (roots_count >= roots_cap) {
        roots_cap *= 2;
        roots = realloc(roots, sizeof(void*) * roots_cap);
    }
    roots[roots_count++] = obj;
}

void jky_gc_pop_root(void) {
    if (roots_count > 0) roots_count--;
}

static void mark_object(void *obj) {
    if (!obj) return;
    // We would determine kind and mark it, and mark its children.
    // For simplicity in this skeleton, we assume a generic mark field exists at offset 0.
    // Since all our gc-tracked objects have `int gc_mark;` but not always at the same offset,
    // this needs proper type-specific marking logic.
    // E.g., cast to struct with gc_mark at the end, etc.
    // Not fully implemented in skeleton.
}

void jky_gc_collect(void) {
    for (int i = 0; i < roots_count; i++) {
        mark_object(roots[i]);
    }
    // Sweep...
}

size_t jky_gc_heap_size(void) {
    size_t count = 0;
    GCObject *curr = head;
    while(curr) {
        count++;
        curr = curr->next;
    }
    return count * 32; // rough estimate
}
