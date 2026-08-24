/* JOCKY Runtime — GC / Part of libjocky. / Garbage Collector */
#ifndef JKY_GC_H
#define JKY_GC_H

#include <stddef.h>
#include "jky_value.h"

void   jky_gc_init(void);
void   jky_gc_register(void *obj, JkyKind kind);
void   jky_gc_collect(void);
void   jky_gc_push_root(void *obj);
void   jky_gc_pop_root(void);
size_t jky_gc_heap_size(void);

#endif // JKY_GC_H
