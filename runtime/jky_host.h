/* JOCKY Runtime — Host / Part of libjocky. / Host Information */
#ifndef JKY_HOST_H
#define JKY_HOST_H

#include "jky_value.h"
#include <stdint.h>

typedef struct JkyHostInfo {
    JkyString *hostname;
    JkyString *os;
    JkyString *arch;
    JkyString *os_version;
    JkyString *kernel;
    int64_t    uptime_sec;
    int64_t    boot_time;
    JkyString *username;
    int        is_admin;
    int64_t    total_ram;
    int64_t    avail_ram;
    int        cpu_count;
    JkyString *cpu_model;
} JkyHostInfo;

JkyHostInfo *jky_host_info(JkyError **err);
JkyString   *jky_host_hostname(JkyError **err);
JkyString   *jky_host_os(JkyError **err);
JkyString   *jky_host_arch(JkyError **err);
int64_t      jky_host_uptime(JkyError **err);
JkyString   *jky_host_info_json(JkyHostInfo *info);

#endif // JKY_HOST_H
