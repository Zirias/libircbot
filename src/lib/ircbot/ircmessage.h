#ifndef IRCBOT_INT_IRCMESSAGE_H
#define IRCBOT_INT_IRCMESSAGE_H

#include <ircbot/decl.h>
#include <ircbot/ircmessage.h>
#include <ircbot/list.h>

#include <stdint.h>

IrcMessage *IrcMessage_create(const uint8_t *buf, uint16_t size, uint16_t *pos)
    ATTR_NONNULL((1)) ATTR_NONNULL((3));
void IrcMessage_destroy(IrcMessage *self);

#endif
