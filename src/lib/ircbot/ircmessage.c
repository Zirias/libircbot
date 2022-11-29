#define _DEFAULT_SOURCE
#include <ircbot/irccommand.h>
#include <ircbot/list.h>
#include <ircbot/log.h>

#include "ircmessage.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

struct IrcMessage
{
    char *prefix;
    char *rawCmd;
    char *rawParams;
    IBList *params;
    IrcCommand command;
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

    IBLog_fmt(L_DEBUG, "IrcMessage: received %.*s",
	    (int)(endpos - *pos), (const char *)buf + *pos);

    uint16_t currpos = *pos;
    *pos = endpos + 2;

    IrcMessage *self = IB_xmalloc(sizeof *self);

    uint16_t e;
    if (buf[currpos] == (uint8_t)':')
    {
	for (e = ++currpos; e < endpos; ++e)
	{
	    if (buf[e] == 0x20) break;
	}
	if (e > currpos)
	{
	    self->prefix = IB_xmalloc(e-currpos+1);
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
    self->rawCmd = IB_xmalloc(e-currpos+1);
    memcpy(self->rawCmd, buf+currpos, e-currpos);
    self->rawCmd[e-currpos] = 0;
    self->command = IrcCommand_parse(self->rawCmd);
    currpos = e;
    if (buf[currpos] == 0x20) ++currpos;

    self->params = IBList_create();
    if (currpos < endpos)
    {
	self->rawParams = IB_xmalloc(endpos-currpos+1);
	memcpy(self->rawParams, buf+currpos, endpos-currpos);
	self->rawParams[endpos-currpos] = 0;
	for (char *p = self->rawParams; p;)
	{
	    if (*p == ':')
	    {
		IBList_append(self->params, IB_copystr(++p), free);
		break;
	    }
	    else IBList_append(self->params, IB_copystr(strsep(&p, " ")), free);
	}
	memcpy(self->rawParams, buf+currpos, endpos-currpos);
    }
    else self->rawParams = 0;

    return self;
}

SOEXPORT const char *IrcMessage_prefix(const IrcMessage *self)
{
    return self->prefix;
}

SOEXPORT IrcCommand IrcMessage_command(const IrcMessage *self)
{
    return self->command;
}

SOEXPORT const char *IrcMessage_rawCmd(const IrcMessage *self)
{
    return self->rawCmd;
}

SOEXPORT const IBList *IrcMessage_params(const IrcMessage *self)
{
    return self->params;
}

SOEXPORT const char *IrcMessage_rawParams(const IrcMessage *self)
{
    return self->rawParams;
}

SOLOCAL void IrcMessage_destroy(IrcMessage *self)
{
    if (!self) return;
    free(self->prefix);
    free(self->rawCmd);
    free(self->rawParams);
    IBList_destroy(self->params);
    free(self);
}

