#ifndef IRCBOT_INT_IRCMESSAGE_H
#define IRCBOT_INT_IRCMESSAGE_H

#include <ircbot/decl.h>

#include <stdint.h>

typedef struct IrcMessage IrcMessage;

IrcMessage *IrcMessage_create(const uint8_t *buf, uint16_t size, uint16_t *pos)
    ATTR_NONNULL((1)) ATTR_NONNULL((3));
const char *IrcMessage_prefix(const IrcMessage *self)
    CMETHOD ATTR_PURE;
const char *IrcMessage_command(const IrcMessage *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
const char *IrcMessage_params(const IrcMessage *self)
    CMETHOD ATTR_PURE;
void IrcMessage_destroy(IrcMessage *self);

#endif
