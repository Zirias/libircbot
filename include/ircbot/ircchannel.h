#ifndef IRCBOT_IRCCHANNEL_H
#define IRCBOT_IRCCHANNEL_H

#include <ircbot/decl.h>

C_CLASS_DECL(Event);
C_CLASS_DECL(HashTable);
C_CLASS_DECL(IrcChannel);

DECLEXPORT const char *IrcChannel_name(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT const HashTable *IrcChannel_nicks(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT Event *IrcChannel_joined(IrcChannel *self) CMETHOD;
DECLEXPORT Event *IrcChannel_parted(IrcChannel *self) CMETHOD;
DECLEXPORT Event *IrcChannel_entered(IrcChannel *self) CMETHOD;

#endif
