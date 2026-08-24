/* JOCKY Runtime — Evidence / Part of libjocky. / Evidence Implementation */
#include "jky_evidence.h"
#include "jky_crypto.h"
#include <stdlib.h>
#include <time.h>

static JkyString *current_timestamp(void) {
    char buf[64];
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);
    return jky_string_from_cstr(buf);
}

JkyCase *jky_evidence_open(JkyString *case_id, JkyError **err) {
    JkyCase *c = malloc(sizeof(JkyCase));
    if (!c) {
        if (err) *err = jky_error_from("Memory allocation failed");
        return NULL;
    }
    c->case_id = case_id;
    c->timestamp = current_timestamp();
    c->tool_version = jky_string_from_cstr("1.0.0");
    c->collector_hash = jky_string_from_cstr("unknown"); // Could hash self binary
    c->host = jky_host_info(NULL);
    c->artifacts = jky_list_new();
    c->sealed = 0;
    return c;
}

void jky_evidence_add(JkyCase *c, JkyString *name, JkyString *data_json, JkyError **err) {
    if (c->sealed) {
        if (err) *err = jky_error_from("Case is sealed");
        return;
    }
    JkyArtifact *a = malloc(sizeof(JkyArtifact));
    a->name = name;
    a->data_json = data_json;
    a->sha256 = jky_sha256_hex((const uint8_t*)data_json->data, data_json->len);
    a->timestamp = current_timestamp();
    
    JkyVal val; val.kind = JKY_STRUCT; val.ptr = a;
    jky_list_push(c->artifacts, val);
}

void jky_evidence_add_val(JkyCase *c, JkyString *name, JkyVal val, JkyError **err) {
    JkyString *json = jky_val_to_json(val);
    jky_evidence_add(c, name, json, err);
}

JkyString *jky_evidence_hash(JkyCase *c, JkyError **err) {
    if (err) *err = jky_error_from("Not fully implemented");
    return jky_string_from_cstr("hash_placeholder");
}

void jky_evidence_seal(JkyCase *c, JkyError **err) {
    c->sealed = 1;
}

void jky_evidence_export(JkyCase *c, JkyString *path, JkyError **err) {
    if (err) *err = jky_error_from("Export not fully implemented");
}
