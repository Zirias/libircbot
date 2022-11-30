#ifndef IRCBOT_INT_CLIENT_H
#define IRCBOT_INT_CLIENT_H

#include <ircbot/decl.h>

C_CLASS_DECL(ClientOpts);
C_CLASS_DECL(Connection);

Connection *Connection_createTcpClient(const ClientOpts *opts)
    ATTR_NONNULL((1));

#endif
