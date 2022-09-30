#include <ircbot/log.h>

#include "ircmessage.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

struct IrcMessage
{
    char *prefix;
    char *command;
    char *params;
};

SOLOCAL IrcMessage *IrcMessage_create(
	const uint8_t *buf, uint16_t size, uint16_t *pos)
{
    uint16_t endpos = (uint16_t)-1;
    for (uint16_t s = *pos; s < size-1; ++s)
    {
	if (buf[s] == 0xd && buf[s+1] == 0xa)
	{
	    endpos = s;
	    break;
	}
    }
    if (endpos == (uint16_t)-1) return 0;
    if (endpos == *pos)
    {
	*pos = endpos + 2;
	return 0;
    }

    logfmt(L_DEBUG, "IrcMessage: received %.*s",
	    (int)(endpos - *pos), (const char *)buf + *pos);

    uint16_t currpos = *pos;
    *pos = endpos + 2;

    IrcMessage *self = xmalloc(sizeof *self);

    uint16_t e;
    if (buf[currpos] == (uint8_t)':')
    {
	for (e = ++currpos; e < endpos; ++e)
	{
	    if (buf[e] == 0x20) break;
	}
	if (e > currpos)
	{
	    self->prefix = xmalloc(e-currpos+1);
	    memcpy(self->prefix, buf+currpos, e-currpos);
	    self->prefix[e-currpos] = 0;
	}
	else self->prefix = 0;
	currpos = e;
	if (buf[currpos] == 0x20) ++currpos;
    }
    else self->prefix = 0;

    for (e = currpos; e < endpos; ++e)
    {
	if (buf[e] == 0x20) break;
    }
    self->command = xmalloc(e-currpos+1);
    memcpy(self->command, buf+currpos, e-currpos);
    self->command[e-currpos] = 0;
    currpos = e;
    if (buf[currpos] == 0x20) ++currpos;

    if (currpos < endpos)
    {
	self->params = xmalloc(endpos-currpos+1);
	memcpy(self->params, buf+currpos, endpos-currpos);
	self->params[endpos-currpos] = 0;
    }
    else self->params = 0;

    return self;
}

SOLOCAL const char *IrcMessage_prefix(const IrcMessage *self)
{
    return self->prefix;
}

SOLOCAL const char *IrcMessage_command(const IrcMessage *self)
{
    return self->command;
}

SOLOCAL const char *IrcMessage_params(const IrcMessage *self)
{
    return self->params;
}

SOLOCAL void IrcMessage_destroy(IrcMessage *self)
{
    if (!self) return;
    free(self->prefix);
    free(self->command);
    free(self->params);
    free(self);
}

