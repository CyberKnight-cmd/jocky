/* JOCKY Runtime — Evidence / Part of libjocky. / Forensics evidence collection */
#ifndef JKY_EVIDENCE_H
#define JKY_EVIDENCE_H

#include "jky_value.h"
#include "jky_host.h"
#include <stdint.h>

typedef struct JkyArtifact {
    JkyString *name;
    JkyString *data_json;
    JkyString *sha256;
    JkyString *timestamp;
} JkyArtifact;

typedef struct JkyCase {
    JkyString   *case_id;
    JkyString   *timestamp;
    JkyString   *tool_version;
    JkyString   *collector_hash;
    JkyHostInfo *host;
    JkyList     *artifacts;
    int          sealed;
} JkyCase;

JkyCase   *jky_evidence_open(JkyString *case_id, JkyError **err);
void       jky_evidence_add(JkyCase *c, JkyString *name, JkyString *data_json, JkyError **err);
void       jky_evidence_add_val(JkyCase *c, JkyString *name, JkyVal val, JkyError **err);
JkyString *jky_evidence_hash(JkyCase *c, JkyError **err);
void       jky_evidence_seal(JkyCase *c, JkyError **err);
void       jky_evidence_export(JkyCase *c, JkyString *path, JkyError **err);

#endif // JKY_EVIDENCE_H
