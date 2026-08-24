/* JOCKY Runtime — Report / Part of libjocky. / Reporting tools */
#ifndef JKY_REPORT_H
#define JKY_REPORT_H

#include "jky_value.h"

void       jky_report_init(void);
void       jky_report_add(JkyString *key, JkyVal val);
JkyString *jky_report_json(void);
void       jky_report_save(JkyString *path, JkyError **err);
void       jky_report_clear(void);

#endif // JKY_REPORT_H
