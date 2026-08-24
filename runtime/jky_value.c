/* JOCKY Runtime — Value / Part of libjocky. / Types and Values */
#include "jky_value.h"
#include "jky_gc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

JkyString *jky_string_new(const char *data, int len) {
    JkyString *s = malloc(sizeof(JkyString));
    if (!s) return NULL;
    s->data = malloc(len + 1);
    if (!s->data) { free(s); return NULL; }
    memcpy(s->data, data, len);
    s->data[len] = '\0';
    s->len = len;
    s->gc_mark = 0;
    jky_gc_register(s, JKY_STRING);
    return s;
}

JkyString *jky_string_from_cstr(const char *s) {
    if (!s) return NULL;
    return jky_string_new(s, (int)strlen(s));
}

JkyString *jky_string_concat(JkyString *a, JkyString *b) {
    if (!a || !b) return NULL;
    int len = a->len + b->len;
    JkyString *s = malloc(sizeof(JkyString));
    if (!s) return NULL;
    s->data = malloc(len + 1);
    if (!s->data) { free(s); return NULL; }
    memcpy(s->data, a->data, a->len);
    memcpy(s->data + a->len, b->data, b->len);
    s->data[len] = '\0';
    s->len = len;
    s->gc_mark = 0;
    jky_gc_register(s, JKY_STRING);
    return s;
}

int jky_string_equals(JkyString *a, JkyString *b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    if (a->len != b->len) return 0;
    return memcmp(a->data, b->data, a->len) == 0;
}

const char *jky_string_cstr(JkyString *s) {
    if (!s) return "";
    return s->data;
}

JkyBytes *jky_bytes_new(int cap) {
    JkyBytes *b = malloc(sizeof(JkyBytes));
    if (!b) return NULL;
    b->data = malloc(cap);
    b->len = 0;
    b->cap = cap;
    b->gc_mark = 0;
    jky_gc_register(b, JKY_BYTES);
    return b;
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

JkyBytes *jky_bytes_from_hex(const char *hex, int hex_len) {
    JkyBytes *b = jky_bytes_new(hex_len / 2);
    if (!b) return NULL;
    b->len = hex_len / 2;
    for (int i = 0; i < b->len; i++) {
        b->data[i] = (hex_val(hex[i*2]) << 4) | hex_val(hex[i*2+1]);
    }
    return b;
}

JkyBytes *jky_bytes_concat(JkyBytes *a, JkyBytes *b) {
    if (!a || !b) return NULL;
    JkyBytes *res = jky_bytes_new(a->len + b->len);
    if (!res) return NULL;
    res->len = a->len + b->len;
    memcpy(res->data, a->data, a->len);
    memcpy(res->data + a->len, b->data, b->len);
    return res;
}

JkyList *jky_list_new(void) {
    JkyList *l = malloc(sizeof(JkyList));
    if (!l) return NULL;
    l->cap = 8;
    l->count = 0;
    l->items = malloc(sizeof(JkyVal) * l->cap);
    l->gc_mark = 0;
    jky_gc_register(l, JKY_LIST);
    return l;
}

void jky_list_push(JkyList *l, JkyVal v) {
    if (!l) return;
    if (l->count >= l->cap) {
        l->cap *= 2;
        l->items = realloc(l->items, sizeof(JkyVal) * l->cap);
    }
    l->items[l->count++] = v;
}

JkyVal jky_list_get(JkyList *l, int idx) {
    if (!l || idx < 0 || idx >= l->count) {
        JkyVal res; res.kind = JKY_NIL; return res;
    }
    return l->items[idx];
}

int jky_list_len(JkyList *l) {
    if (!l) return 0;
    return l->count;
}

JkyMap *jky_map_new(void) {
    JkyMap *m = malloc(sizeof(JkyMap));
    if (!m) return NULL;
    m->bucket_count = 16;
    m->entry_count = 0;
    m->buckets = calloc(m->bucket_count, sizeof(JkyMapEntry));
    m->gc_mark = 0;
    jky_gc_register(m, JKY_MAP);
    return m;
}

static uint32_t hash_string(JkyString *s) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < s->len; i++) {
        hash ^= (uint8_t)s->data[i];
        hash *= 16777619;
    }
    return hash;
}

void jky_map_set(JkyMap *m, JkyString *k, JkyVal v) {
    if (!m || !k) return;
    uint32_t h = hash_string(k) % m->bucket_count;
    // Extremely simplistic open addressing or just chaining placeholder
    // In a real implementation we'd handle collisions properly.
    m->buckets[h].key = k;
    m->buckets[h].value = v;
    m->entry_count++;
}

JkyVal jky_map_get(JkyMap *m, JkyString *k) {
    JkyVal res; res.kind = JKY_NIL;
    if (!m || !k) return res;
    uint32_t h = hash_string(k) % m->bucket_count;
    if (m->buckets[h].key && jky_string_equals(m->buckets[h].key, k)) {
        return m->buckets[h].value;
    }
    return res;
}

int jky_map_has(JkyMap *m, JkyString *k) {
    if (!m || !k) return 0;
    uint32_t h = hash_string(k) % m->bucket_count;
    return (m->buckets[h].key && jky_string_equals(m->buckets[h].key, k));
}

JkyError *jky_error_new(int code, const char *message, const char *source) {
    JkyError *e = malloc(sizeof(JkyError));
    if (!e) return NULL;
    e->code = code;
    e->message = jky_string_from_cstr(message);
    e->source = jky_string_from_cstr(source);
    e->gc_mark = 0;
    jky_gc_register(e, JKY_ERROR);
    return e;
}

JkyError *jky_error_from(const char *message) {
    return jky_error_new(-1, message, "runtime");
}

JkyString *jky_val_to_string(JkyVal v) {
    char buf[128];
    switch(v.kind) {
        case JKY_NIL: return jky_string_from_cstr("nil");
        case JKY_INT: snprintf(buf, sizeof(buf), "%lld", (long long)v.i); return jky_string_from_cstr(buf);
        case JKY_FLOAT: snprintf(buf, sizeof(buf), "%g", v.f); return jky_string_from_cstr(buf);
        case JKY_BOOL: return jky_string_from_cstr(v.b ? "true" : "false");
        case JKY_BYTE: snprintf(buf, sizeof(buf), "%u", v.byte_v); return jky_string_from_cstr(buf);
        case JKY_STRING: return v.str;
        case JKY_BYTES: return jky_string_from_cstr("<bytes>");
        case JKY_LIST: return jky_string_from_cstr("<list>");
        case JKY_MAP: return jky_string_from_cstr("<map>");
        case JKY_ERROR: return v.err ? v.err->message : jky_string_from_cstr("<error>");
        default: return jky_string_from_cstr("<unknown>");
    }
}

JkyString *jky_val_to_json(JkyVal v) {
    // Placeholder minimal implementation
    return jky_val_to_string(v);
}
