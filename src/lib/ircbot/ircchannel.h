#ifndef IRCBOT_INT_IRCCHANNEL_H
#define IRCBOT_INT_IRCCHANNEL_H

#include <ircbot/ircchannel.h>

C_CLASS_DECL(Event);

IrcChannel *IrcChannel_create(IrcServer *server, const char *name)
    ATTR_NONNULL((1)) ATTR_NONNULL((2)) ATTR_RETNONNULL;
void IrcChannel_destroy(IrcChannel *self);
void IrcChannel_join(IrcChannel *self) CMETHOD;
void IrcChannel_part(IrcChannel *self) CMETHOD;
Event *IrcChannel_joined(IrcChannel *self) CMETHOD;
Event *IrcChannel_parted(IrcChannel *self) CMETHOD;
Event *IrcChannel_entered(IrcChannel *self) CMETHOD;
Event *IrcChannel_left(IrcChannel *self) CMETHOD;
Event *IrcChannel_failed(IrcChannel *self) CMETHOD;

#endif
