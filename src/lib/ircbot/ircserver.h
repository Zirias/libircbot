#ifndef IRCBOT_INT_IRCSERVER_H
#define IRCBOT_INT_IRCSERVER_H

#include <ircbot/irccommand.h>
#include <ircbot/ircserver.h>

C_CLASS_DECL(Event);

int IrcServer_connect(IrcServer *self) CMETHOD;
void IrcServer_disconnect(IrcServer *self) CMETHOD;
void IrcServer_setNick(IrcServer *self, const char *nick)
    CMETHOD ATTR_NONNULL((2));
int IrcServer_sendCmd(IrcServer *self, IrcCommand cmd,
	const char *args)
    CMETHOD ATTR_NONNULL((3));
int IrcServer_sendMsg(IrcServer *self, const char *to,
	const char *message, int action)
    CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));
Event *IrcServer_connected(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
Event *IrcServer_disconnected(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
Event *IrcServer_msgReceived(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
Event *IrcServer_joined(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
Event *IrcServer_parted(IrcServer *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;

#endif
