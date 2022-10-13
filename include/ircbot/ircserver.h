#ifndef IRCBOT_IRCSERVER_H
#define IRCBOT_IRCSERVER_H

#include <ircbot/decl.h>

C_CLASS_DECL(HashTable);
C_CLASS_DECL(IrcChannel);
C_CLASS_DECL(IrcServer);

DECLEXPORT IrcServer *IrcServer_create(const char *id,
	const char *remotehost, int port,
	const char *nick, const char *user, const char *realname)
    ATTR_RETNONNULL ATTR_NONNULL((1))
    ATTR_NONNULL((2)) ATTR_NONNULL((4));
DECLEXPORT const char *IrcServer_id(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT const char *IrcServer_name(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT const char *IrcServer_nick(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT const HashTable *IrcServer_channels(const IrcServer *self)
    CMETHOD;
DECLEXPORT void IrcServer_destroy(IrcServer *self);

#endif
