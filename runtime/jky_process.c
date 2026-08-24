/* JOCKY Runtime — Process / Part of libjocky. / Process Information Implementation */
#include "jky_process.h"
#include <stdlib.h>
#include <string.h>

JkyList *jky_process_list(JkyError **err) {
    // Stub implementation
    return jky_list_new();
}

JkyProcessEntry *jky_process_info(int64_t pid, JkyError **err) {
    if (err) *err = jky_error_from("Not implemented");
    return NULL;
}

JkyList *jky_process_modules(int64_t pid, JkyError **err) {
    return jky_list_new();
}

JkyProcessEntry *jky_process_find(JkyString *name, int *found, JkyError **err) {
    if (found) *found = 0;
    if (err) *err = jky_error_from("Not implemented");
    return NULL;
}

JkyString *jky_process_entry_json(JkyProcessEntry *p) {
    return jky_string_from_cstr("{}");
}

JkyString *jky_process_list_json(JkyList *procs) {
    return jky_string_from_cstr("[]");
}
