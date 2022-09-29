#ifndef IRCBOT_IRCSERVER_H
#define IRCBOT_IRCSERVER_H

#include <ircbot/decl.h>

C_CLASS_DECL(Event);
C_CLASS_DECL(IrcServer);

typedef struct MsgReceivedEventArgs
{
    const char *from;
    const char *to;
    const char *message;
} MsgReceivedEventArgs;

DECLEXPORT IrcServer *IrcServer_create(const char *remotehost, int port,
	const char *nick, const char *user, const char *realname)
    ATTR_RETNONNULL ATTR_NONNULL((1)) ATTR_NONNULL((3));
DECLEXPORT void IrcServer_connect(IrcServer *self) CMETHOD;
DECLEXPORT void IrcServer_disconnect(IrcServer *self) CMETHOD;
DECLEXPORT const char *IrcServer_nick(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT void IrcServer_setNick(IrcServer *self, const char *nick)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT void IrcServer_join(IrcServer *self, const char *channel)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT void IrcServer_part(IrcServer *self, const char *channel)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT int IrcServer_sendMsg(IrcServer *self, const char *to,
	const char *message, int action)
    CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));
DECLEXPORT Event *IrcServer_connected(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT Event *IrcServer_disconnected(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT Event *IrcServer_msgReceived(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT void IrcServer_destroy(IrcServer *self);

#endif
