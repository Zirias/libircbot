#ifndef IRCBOT_INT_IRCCHANNEL_H
#define IRCBOT_INT_IRCCHANNEL_H

#include <ircbot/ircchannel.h>

typedef struct IrcMessage IrcMessage;

IrcChannel *IrcChannel_create(const char *name)
    ATTR_NONNULL((1)) ATTR_RETNONNULL;
void IrcChannel_handleMessage(IrcChannel *self, const IrcMessage *msg)
    CMETHOD ATTR_NONNULL((2));
void IrcChannel_destroy(IrcChannel *self);

#endif
