/* JOCKY Runtime — Print / Part of libjocky. / Standard output formatting */
#include "jky_value.h"
#include <stdio.h>

void jky_print_str(JkyString *s) {
    if (s) printf("%s", s->data);
}

void jky_print_int(int64_t i) {
    printf("%lld", (long long)i);
}

void jky_print_float(double f) {
    printf("%g", f);
}

void jky_print_bool(int b) {
    printf(b ? "true" : "false");
}

void jky_print_list(JkyList *l) {
    printf("[");
    for (int i = 0; i < l->count; i++) {
        // recursive call theoretically
        printf("...");
        if (i < l->count - 1) printf(", ");
    }
    printf("]");
}

void jky_print_map(JkyMap *m) {
    printf("{...}");
}

void jky_print(JkyVal v) {
    switch (v.kind) {
        case JKY_NIL: printf("nil"); break;
        case JKY_INT: jky_print_int(v.i); break;
        case JKY_FLOAT: jky_print_float(v.f); break;
        case JKY_BOOL: jky_print_bool(v.b); break;
        case JKY_BYTE: printf("%u", v.byte_v); break;
        case JKY_STRING: jky_print_str(v.str); break;
        case JKY_BYTES: printf("<bytes:%d>", v.bytes->len); break;
        case JKY_LIST: jky_print_list(v.list); break;
        case JKY_MAP: jky_print_map(v.map); break;
        case JKY_ERROR: 
            if (v.err) printf("error: %s", jky_string_cstr(v.err->message));
            break;
        default: printf("<unknown>"); break;
    }
    printf("\n");
}
