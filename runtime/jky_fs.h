/* JOCKY Runtime — Filesystem / Part of libjocky. / FS Operations */
#ifndef JKY_FS_H
#define JKY_FS_H

#include "jky_value.h"
#include <stdint.h>

typedef struct JkyFSEntry {
    JkyString *name;
    JkyString *path;
    int        is_dir;
    int64_t    size;
    int64_t    mtime;
    int64_t    ctime;
    int64_t    atime;
    JkyString *permissions;
    JkyString *owner;
    JkyString *sha256;
} JkyFSEntry;

JkyList    *jky_fs_list(JkyString *path, JkyError **err);
JkyFSEntry *jky_fs_metadata(JkyString *path, JkyError **err);
JkyString  *jky_fs_hash(JkyString *path, JkyError **err);
JkyBytes   *jky_fs_read(JkyString *path, JkyError **err);
int         jky_fs_exists(JkyString *path);
JkyString  *jky_fs_entry_json(JkyFSEntry *e);

#endif // JKY_FS_H
