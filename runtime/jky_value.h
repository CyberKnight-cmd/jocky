/* JOCKY Runtime — [value] / Part of libjocky. / [desc] */
#ifndef JKY_VALUE_H
#define JKY_VALUE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    JKY_NIL=0, JKY_INT, JKY_FLOAT, JKY_BOOL, JKY_BYTE,
    JKY_STRING, JKY_BYTES, JKY_LIST, JKY_MAP, JKY_ERROR, JKY_STRUCT,
} JkyKind;

typedef struct JkyString {
    char *data;   // UTF-8, null-terminated
    int   len;
    int   gc_mark;
} JkyString;

typedef struct JkyBytes {
    uint8_t *data;
    int      len;
    int      cap;
    int      gc_mark;
} JkyBytes;

typedef struct JkyList JkyList;
typedef struct JkyMap JkyMap;
typedef struct JkyError JkyError;
typedef struct JkyVal JkyVal;

struct JkyVal {
    JkyKind kind;
    union {
        int64_t    i;
        double     f;
        int        b;
        uint8_t    byte_v;
        JkyString *str;
        JkyBytes  *bytes;
        JkyList   *list;
        JkyMap    *map;
        JkyError  *err;
        void      *ptr;
    };
};

typedef struct JkyList {
    JkyVal *items;
    int     count;
    int     cap;
    int     gc_mark;
} JkyList;

typedef struct JkyMapEntry {
    JkyString *key;
    JkyVal     value;
} JkyMapEntry;

typedef struct JkyMap {
    JkyMapEntry *buckets;
    int          bucket_count;
    int          entry_count;
    int          gc_mark;
} JkyMap;

typedef struct JkyError {
    int        code;
    JkyString *message;
    JkyString *source;
    int        gc_mark;
} JkyError;

// String functions
JkyString  *jky_string_new(const char *data, int len);
JkyString  *jky_string_from_cstr(const char *s);
JkyString  *jky_string_concat(JkyString *a, JkyString *b);
int         jky_string_equals(JkyString *a, JkyString *b);
const char *jky_string_cstr(JkyString *s);

// Bytes functions
JkyBytes *jky_bytes_new(int cap);
JkyBytes *jky_bytes_from_hex(const char *hex, int hex_len);
JkyBytes *jky_bytes_concat(JkyBytes *a, JkyBytes *b);

// List functions
JkyList *jky_list_new(void);
void     jky_list_push(JkyList *l, JkyVal v);
JkyVal   jky_list_get(JkyList *l, int idx);
int      jky_list_len(JkyList *l);

// Map functions
JkyMap *jky_map_new(void);
void    jky_map_set(JkyMap *m, JkyString *k, JkyVal v);
JkyVal  jky_map_get(JkyMap *m, JkyString *k);
int     jky_map_has(JkyMap *m, JkyString *k);

// Error functions
JkyError *jky_error_new(int code, const char *message, const char *source);
JkyError *jky_error_from(const char *message);

// Conversion
JkyString *jky_val_to_string(JkyVal v);
JkyString *jky_val_to_json(JkyVal v);

#endif // JKY_VALUE_H
