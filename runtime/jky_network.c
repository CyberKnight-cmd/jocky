/* JOCKY Runtime — Network / Part of libjocky. / Network Information Implementation */
#include "jky_network.h"
#include <stdlib.h>

JkyList *jky_net_connections(JkyError **err) {
    if (err) *err = jky_error_from("Not implemented");
    return jky_list_new();
}

JkyList *jky_net_interfaces(JkyError **err) {
    if (err) *err = jky_error_from("Not implemented");
    return jky_list_new();
}

JkyString *jky_net_conn_json(JkyNetConn *c) {
    return jky_string_from_cstr("{}");
}

JkyString *jky_net_interface_json(JkyNetInterface *iface) {
    return jky_string_from_cstr("{}");
}
