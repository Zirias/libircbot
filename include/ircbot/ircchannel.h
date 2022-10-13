#ifndef IRCBOT_IRCCHANNEL_H
#define IRCBOT_IRCCHANNEL_H

#include <ircbot/decl.h>

C_CLASS_DECL(Event);
C_CLASS_DECL(HashTable);
C_CLASS_DECL(IrcChannel);
C_CLASS_DECL(IrcServer);

DECLEXPORT const char *IrcChannel_name(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT const IrcServer *IrcChannel_server(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT int IrcChannel_isJoined(const IrcChannel *self) CMETHOD;
DECLEXPORT const HashTable *IrcChannel_nicks(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT void IrcChannel_join(IrcChannel *self) CMETHOD;
DECLEXPORT void IrcChannel_part(IrcChannel *self) CMETHOD;
DECLEXPORT Event *IrcChannel_joined(IrcChannel *self) CMETHOD;
DECLEXPORT Event *IrcChannel_parted(IrcChannel *self) CMETHOD;
DECLEXPORT Event *IrcChannel_entered(IrcChannel *self) CMETHOD;
DECLEXPORT Event *IrcChannel_left(IrcChannel *self) CMETHOD;
DECLEXPORT Event *IrcChannel_failed(IrcChannel *self) CMETHOD;

#endif
