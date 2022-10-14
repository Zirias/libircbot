#ifndef IRCBOT_IRCMESSAGE_H
#define IRCBOT_IRCMESSAGE_H

#include <ircbot/decl.h>
#include <ircbot/irccommand.h>

C_CLASS_DECL(IrcMessage);
C_CLASS_DECL(IBList);

DECLEXPORT const char *IrcMessage_prefix(const IrcMessage *self)
    CMETHOD ATTR_PURE;
DECLEXPORT IrcCommand IrcMessage_command(const IrcMessage *self)
    CMETHOD ATTR_PURE;
DECLEXPORT const char *IrcMessage_rawCmd(const IrcMessage *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT const IBList *IrcMessage_params(const IrcMessage *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT const char *IrcMessage_rawParams(const IrcMessage *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;

#endif
