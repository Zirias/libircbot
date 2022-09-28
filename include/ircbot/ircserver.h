#ifndef IRCBOT_IRCSERVER_H
#define IRCBOT_IRCSERVER_H

#include <ircbot/decl.h>

C_CLASS_DECL(Event);
C_CLASS_DECL(IrcServer);

typedef struct MsgReceivedEventArgs
{
    const char *nick;
    const char *channel;
    const char *message;
} MsgReceivedEventArgs;

IrcServer *IrcServer_create(const char *remotehost, int port,
	const char *nick, const char *user, const char *realname)
    ATTR_RETNONNULL ATTR_NONNULL((1)) ATTR_NONNULL((3));
void IrcServer_connect(IrcServer *self) CMETHOD;
void IrcServer_disconnect(IrcServer *self) CMETHOD;
const char *IrcServer_nick(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;
void IrcServer_setNick(IrcServer *self, const char *nick)
    CMETHOD ATTR_NONNULL((2));
void IrcServer_join(IrcServer *self, const char *channel)
    CMETHOD ATTR_NONNULL((2));
void IrcServer_part(IrcServer *self, const char *channel)
    CMETHOD ATTR_NONNULL((2));
int IrcServer_sendNick(IrcServer *self, const char *nick,
	const char *message) CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));
int IrcServer_sendChannel(IrcServer *self, const char *channel,
	const char *message) CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));
Event *IrcServer_connected(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
Event *IrcServer_disconnected(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
Event *IrcServer_msgReceived(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
void IrcServer_destroy(IrcServer *self);

#endif
