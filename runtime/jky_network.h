/* JOCKY Runtime — Network / Part of libjocky. / Network Information */
#ifndef JKY_NETWORK_H
#define JKY_NETWORK_H

#include "jky_value.h"
#include <stdint.h>

typedef struct JkyNetConn {
    JkyString *protocol;
    JkyString *local_addr;
    JkyString *remote_addr;
    JkyString *state;
    int64_t    pid;
} JkyNetConn;

typedef struct JkyNetInterface {
    JkyString *name;
    JkyString *mac;
    JkyList   *addrs;
    int        is_up;
    int        is_loopback;
} JkyNetInterface;

JkyList   *jky_net_connections(JkyError **err);
JkyList   *jky_net_interfaces(JkyError **err);
JkyString *jky_net_conn_json(JkyNetConn *c);
JkyString *jky_net_interface_json(JkyNetInterface *iface);

#endif // JKY_NETWORK_H
