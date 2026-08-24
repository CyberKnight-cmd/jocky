/* JOCKY Runtime — Host / Part of libjocky. / Host Information Implementation */
#include "jky_host.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

JkyHostInfo *jky_host_info(JkyError **err) {
    JkyHostInfo *info = malloc(sizeof(JkyHostInfo));
    if (!info) {
        if (err) *err = jky_error_from("Memory allocation failed");
        return NULL;
    }
    memset(info, 0, sizeof(JkyHostInfo));
    
    info->hostname = jky_host_hostname(err);
    info->os = jky_host_os(err);
    info->arch = jky_host_arch(err);
    info->uptime_sec = jky_host_uptime(err);
    
    // Stub remaining fields
    info->os_version = jky_string_from_cstr("unknown");
    info->kernel = jky_string_from_cstr("unknown");
    info->username = jky_string_from_cstr("user");
    info->cpu_model = jky_string_from_cstr("Generic CPU");
    
    return info;
}

JkyString *jky_host_hostname(JkyError **err) {
    char buf[256];
#ifdef _WIN32
    DWORD size = sizeof(buf);
    if (GetComputerNameA(buf, &size)) {
        return jky_string_from_cstr(buf);
    }
#else
    if (gethostname(buf, sizeof(buf)) == 0) {
        return jky_string_from_cstr(buf);
    }
#endif
    if (err) *err = jky_error_from("Failed to get hostname");
    return jky_string_from_cstr("unknown");
}

JkyString *jky_host_os(JkyError **err) {
#ifdef _WIN32
    return jky_string_from_cstr("windows");
#else
    return jky_string_from_cstr("linux");
#endif
}

JkyString *jky_host_arch(JkyError **err) {
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        return jky_string_from_cstr("x86_64");
    return jky_string_from_cstr("unknown");
#else
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        return jky_string_from_cstr(buffer.machine);
    }
    if (err) *err = jky_error_from("Failed to get arch");
    return jky_string_from_cstr("unknown");
#endif
}

int64_t jky_host_uptime(JkyError **err) {
#ifdef _WIN32
    return GetTickCount64() / 1000;
#else
    return 0; // Requires sysinfo or /proc/uptime parsing
#endif
}

JkyString *jky_host_info_json(JkyHostInfo *info) {
    if (!info) return jky_string_from_cstr("{}");
    // Minimal JSON stub
    char buf[512];
    snprintf(buf, sizeof(buf), "{\"hostname\":\"%s\",\"os\":\"%s\"}",
             jky_string_cstr(info->hostname), jky_string_cstr(info->os));
    return jky_string_from_cstr(buf);
}
