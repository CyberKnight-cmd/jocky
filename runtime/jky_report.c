/* JOCKY Runtime — Report / Part of libjocky. / Report Implementation */
#include "jky_report.h"
#include <stdio.h>
#include <stdlib.h>

static JkyMap *global_report = NULL;

void jky_report_init(void) {
    if (!global_report) {
        global_report = jky_map_new();
    }
}

void jky_report_add(JkyString *key, JkyVal val) {
    jky_report_init();
    jky_map_set(global_report, key, val);
}

JkyString *jky_report_json(void) {
    if (!global_report) return jky_string_from_cstr("{}");
    // Minimal JSON dict implementation
    return jky_string_from_cstr("{}"); // Stub
}

void jky_report_save(JkyString *path, JkyError **err) {
    if (!path) {
        if (err) *err = jky_error_from("Invalid path");
        return;
    }
    FILE *f = fopen(path->data, "wb");
    if (!f) {
        if (err) *err = jky_error_from("Cannot open file");
        return;
    }
    JkyString *json = jky_report_json();
    fwrite(json->data, 1, json->len, f);
    fclose(f);
}

void jky_report_clear(void) {
    global_report = jky_map_new();
}
