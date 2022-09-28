#ifndef IRCBOT_INT_CLIENT_H
#define IRCBOT_INT_CLIENT_H

#include <ircbot/decl.h>

typedef struct Connection Connection;

Connection *Connection_createTcpClient(const char *remotehost, int port,
	int numerichosts) ATTR_NONNULL((1));

#endif
