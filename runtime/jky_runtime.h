/* JOCKY Runtime — Master Header / Part of libjocky. / Main includes */
#ifndef JKY_RUNTIME_H
#define JKY_RUNTIME_H

#include "jky_value.h"
#include "jky_gc.h"
#include "jky_host.h"
#include "jky_process.h"
#include "jky_network.h"
#include "jky_fs.h"
#include "jky_evidence.h"
#include "jky_report.h"
#include "jky_crypto.h"
#include "jky_log.h"

#define JKY_OK   NULL
#define JKY_ERR(msg) jky_error_from(msg)
#define JKY_STR(s)   jky_string_from_cstr(s)

void jky_print(JkyVal v);
void jky_print_str(JkyString *s);
void jky_print_int(int64_t i);
void jky_print_float(double f);
void jky_print_bool(int b);
void jky_print_list(JkyList *l);
void jky_print_map(JkyMap *m);

#endif // JKY_RUNTIME_H
