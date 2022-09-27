#ifndef IRCBOT_INT_CLIENT_H
#define IRCBOT_INT_CLIENT_H

#include <ircbot/decl.h>

#include <stdint.h>

typedef struct Config Config;
typedef struct Connection Connection;

Connection *Connection_createTcpClient(const Config *config,
	uint8_t readOffset) ATTR_NONNULL((1));

#endif
