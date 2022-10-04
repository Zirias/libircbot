#include <ircbot/event.h>
#include <ircbot/hashtable.h>

#include "ircchannel.h"
#include "ircmessage.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

struct IrcChannel
{
    char *name;
    HashTable *nicks;
    Event *joined;
    Event *parted;
    Event *entered;
};

SOLOCAL IrcChannel *IrcChannel_create(const char *name)
{
    IrcChannel *self = xmalloc(sizeof *self);
    self->name = copystr(name);
    self->nicks = HashTable_create(8);
    self->joined = Event_create(self);
    self->parted = Event_create(self);
    self->entered = Event_create(self);
    return self;
}

SOEXPORT const char *IrcChannel_name(const IrcChannel *self)
{
    return self->name;
}

SOEXPORT const HashTable *IrcChannel_nicks(const IrcChannel *self)
{
    return self->nicks;
}

SOLOCAL void IrcChannel_handleMessage(IrcChannel *self, const IrcMessage *msg)
{
    const char *cmd = IrcMessage_command(msg);
    const List *params = IrcMessage_params(msg);
    if (!strcmp(cmd, "JOIN") && List_size(params)
	    && !strcmp(List_at(params, 0), self->name))
    {
	char buf[128];
	sscanf(IrcMessage_prefix(msg), "%127[^!]", buf);
	HashTable_set(self->nicks, buf, self->name, 0);
	Event_raise(self->joined, 0, buf);
    }
    else if ((!strcmp(cmd, "PART") && List_size(params)
		&& !strcmp(List_at(params, 0), self->name))
	    || !strcmp(cmd, "QUIT"))
    {
	char buf[128];
	sscanf(IrcMessage_prefix(msg), "%127[^!]", buf);
	HashTable_delete(self->nicks, buf);
	Event_raise(self->parted, 0, buf);
    }
    else if (!strcmp(cmd, "NICK") && List_size(params) == 1)
    {
	char buf[128];
	sscanf(IrcMessage_prefix(msg), "%127[^!]", buf);
	if (HashTable_get(self->nicks, buf))
	{
	    HashTable_delete(self->nicks, buf);
	    HashTable_set(self->nicks, List_at(params, 0), self->name, 0);
	}
    }
    else if (!strcmp(cmd, "353") && List_size(params) == 4
	    && !strcmp(List_at(params, 2), self->name))
    {
	char *nicklist = copystr(List_at(params, 3));
	char *i = nicklist;
	char *nick = strsep(&i, " ");
	while (nick)
	{
	    HashTable_set(self->nicks, nick, self->name, 0);
	    nick = strsep(&i, " ");
	}
	free(nicklist);
    }
    else if (!strcmp(cmd, "366") && List_size(params) > 1
	    && !strcmp(List_at(params, 1), self->name))
    {
	Event_raise(self->entered, 0, 0);
    }
}

SOEXPORT Event *IrcChannel_joined(IrcChannel *self)
{
    return self->joined;
}

SOEXPORT Event *IrcChannel_parted(IrcChannel *self)
{
    return self->parted;
}

SOEXPORT Event *IrcChannel_entered(IrcChannel *self)
{
    return self->entered;
}

SOLOCAL void IrcChannel_destroy(IrcChannel *self)
{
    if (!self) return;
    Event_destroy(self->entered);
    Event_destroy(self->parted);
    Event_destroy(self->joined);
    HashTable_destroy(self->nicks);
    free(self->name);
    free(self);
}

