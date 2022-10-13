#ifndef IRCBOT_INT_IRCCHANNEL_H
#define IRCBOT_INT_IRCCHANNEL_H

#include <ircbot/ircchannel.h>

typedef struct IrcMessage IrcMessage;

IrcChannel *IrcChannel_create(IrcServer *server, const char *name)
    ATTR_NONNULL((1)) ATTR_NONNULL((2)) ATTR_RETNONNULL;
void IrcChannel_destroy(IrcChannel *self);

#endif
