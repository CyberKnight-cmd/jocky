/* JOCKY Runtime — Log / Part of libjocky. / Log Implementation */
#include "jky_log.h"
#include <time.h>

static JkyLogLevel current_min_level = JKY_LOG_INFO;
static FILE *log_output = NULL;

void jky_log_init(JkyLogLevel min_level) {
    current_min_level = min_level;
    if (!log_output) log_output = stdout;
}

void jky_log_set_output(FILE *f) {
    log_output = f ? f : stdout;
}

static void log_print(JkyLogLevel level, const char *level_str, JkyString *msg) {
    if (level < current_min_level) return;
    if (!log_output) log_output = stdout;
    
    char timebuf[64];
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", t);
    
    fprintf(log_output, "[%s] [%s] %s\n", timebuf, level_str, jky_string_cstr(msg));
}

void jky_log_info(JkyString *msg) {
    log_print(JKY_LOG_INFO, "INFO ", msg);
}

void jky_log_warn(JkyString *msg) {
    log_print(JKY_LOG_WARN, "WARN ", msg);
}

void jky_log_error(JkyString *msg) {
    log_print(JKY_LOG_ERROR, "ERROR", msg);
}
