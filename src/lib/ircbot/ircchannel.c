#include <ircbot/hashtable.h>
#include <ircbot/irccommand.h>

#include "event.h"
#include "ircchannel.h"
#include "ircmessage.h"
#include "ircserver.h"
#include "service.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

#define JOINSYNCTICKS 15
#define REJOINTICKS 30

struct IrcChannel
{
    char *name;
    IrcServer *server;
    HashTable *nicks;
    Event *joined;
    Event *parted;
    Event *entered;
    Event *left;
    Event *failed;
    int isJoined;
    int wantJoined;
    int joinsyncticks;
    int rejointicks;
};

static void joinOnConnect(void *receiver, void *sender, void *args);
static void disconnected(void *receiver, void *sender, void *args);
static void waitjoin(void *receiver, void *sender, void *args);
static void waitrejoin(void *receiver, void *sender, void *args);
static void handleMsg(void *receiver, void *sender, void *args);

SOLOCAL IrcChannel *IrcChannel_create(IrcServer *server, const char *name)
{
    IrcChannel *self = IB_xmalloc(sizeof *self);
    self->name = IB_copystr(name);
    self->server = server;
    self->nicks = HashTable_create(8);
    self->joined = Event_create(self);
    self->parted = Event_create(self);
    self->entered = Event_create(self);
    self->left = Event_create(self);
    self->failed = Event_create(self);
    self->isJoined = 0;
    self->wantJoined = 0;

    Event *msgev = IrcServer_msgReceived(server);
    Event_register(msgev, self, handleMsg, MSG_JOIN);
    Event_register(msgev, self, handleMsg, MSG_PART);
    Event_register(msgev, self, handleMsg, MSG_QUIT);
    Event_register(msgev, self, handleMsg, MSG_NICK);
    Event_register(msgev, self, handleMsg, MSG_KICK);
    Event_register(msgev, self, handleMsg, RPL_NAMREPLY);
    Event_register(msgev, self, handleMsg, RPL_ENDOFNAMES);
    Event_register(msgev, self, handleMsg, ERR_NOSUCHCHANNEL);

    return self;
}

SOEXPORT const char *IrcChannel_name(const IrcChannel *self)
{
    return self->name;
}

SOEXPORT const IrcServer *IrcChannel_server(const IrcChannel *self)
{
    return self->server;
}

SOEXPORT int IrcChannel_isJoined(const IrcChannel *self)
{
    return self->isJoined;
}

SOEXPORT const HashTable *IrcChannel_nicks(const IrcChannel *self)
{
    return self->nicks;
}

static void joinOnConnect(void *receiver, void *sender, void *args)
{
    IrcChannel *self = receiver;
    IrcServer *server = sender;
    (void)args;

    if (server != self->server) return;
    Event_unregister(IrcServer_connected(self->server), self,
	    joinOnConnect, 0);
    self->wantJoined = 0;
    IrcChannel_join(self);
}

static void disconnected(void *receiver, void *sender, void *args)
{
    IrcChannel *self = receiver;
    IrcServer *server = sender;
    (void)args;

    if (server != self->server) return;

    Event_unregister(IrcServer_connected(self->server), self,
	    joinOnConnect, 0);
    Event_unregister(Service_tick(), self, waitjoin, 0);
    Event_unregister(Service_tick(), self, waitrejoin, 0);
    Event_unregister(IrcServer_disconnected(self->server), self,
	    disconnected, 0);
    if (self->wantJoined)
    {
	Event_register(IrcServer_connected(self->server), self,
		joinOnConnect, 0);
    }
    if (self->isJoined)
    {
	self->isJoined = 0;
	Event_raise(self->parted, 0, 0);
    }
}

static void waitjoin(void *receiver, void *sender, void *args)
{
    IrcChannel *self = receiver;
    (void)sender;
    (void)args;

    if (!--self->joinsyncticks)
    {
	IrcServer_sendCmd(self->server, MSG_PART, self->name);
	Event_unregister(Service_tick(), self, waitjoin, 0);
	self->rejointicks = REJOINTICKS;
	Event_register(Service_tick(), self, waitrejoin, 0);
    }
}

static void waitrejoin(void *receiver, void *sender, void *args)
{
    IrcChannel *self = receiver;
    (void)sender;
    (void)args;

    if (!--self->rejointicks)
    {
	Event_unregister(Service_tick(), self, waitrejoin, 0);
	self->wantJoined = 0;
	IrcChannel_join(self);
    }
}

SOLOCAL void IrcChannel_join(IrcChannel *self)
{
    if (self->wantJoined) return;
    self->wantJoined = 1;
    if (IrcServer_sendCmd(self->server, MSG_JOIN, self->name) < 0)
    {
	Event_register(IrcServer_connected(self->server), self,
		joinOnConnect, 0);
	return;
    }
    self->joinsyncticks = JOINSYNCTICKS;
    Event_register(Service_tick(), self, waitjoin, 0);
    Event_register(IrcServer_disconnected(self->server), self,
	    disconnected, 0);
}

SOLOCAL void IrcChannel_part(IrcChannel *self)
{
    self->wantJoined = 0;
    Event_unregister(IrcServer_connected(self->server), self,
	    joinOnConnect, 0);
    Event_unregister(Service_tick(), self, waitjoin, 0);
    Event_unregister(Service_tick(), self, waitrejoin, 0);
    Event_unregister(IrcServer_disconnected(self->server), self,
	    disconnected, 0);

    if (self->isJoined)
    {
	IrcServer_sendCmd(self->server, MSG_PART, self->name);
	self->isJoined = 0;
	Event_raise(self->parted, 0, 0);
    }
}

static void handleMsg(void *receiver, void *sender, void *args)
{
    IrcChannel *self = receiver;
    IrcServer *server = sender;
    const IrcMessage *msg = args;

    if (server != self->server) return;

    IrcCommand cmd = IrcMessage_command(msg);
    const List *params = IrcMessage_params(msg);
    char buf[128];

    switch (cmd)
    {
	case MSG_JOIN:
	    if (List_size(params) && !strcmp(List_at(params, 0), self->name))
	    {
		sscanf(IrcMessage_prefix(msg), "%127[^!]", buf);
		if (strcmp(buf, IrcServer_nick(self->server)))
		{
		    HashTable_set(self->nicks, buf, self->name, 0);
		    Event_raise(self->entered, 0, buf);
		}
	    }
	    break;

	case MSG_PART:
	    if (!List_size(params) || strcmp(List_at(params, 0), self->name))
		break;
	    ATTR_FALLTHROUGH;
	case MSG_QUIT:
	    sscanf(IrcMessage_prefix(msg), "%127[^!]", buf);
	    if (HashTable_delete(self->nicks, buf))
	    {
		Event_raise(self->left, 0, buf);
	    }
	    break;

	case MSG_NICK:
	    if (List_size(params) == 1)
	    {
		sscanf(IrcMessage_prefix(msg), "%127[^!]", buf);
		if (HashTable_delete(self->nicks, buf))
		{
		    HashTable_set(self->nicks, List_at(params, 0),
			    self->name, 0);
		}
	    }
	    break;

	case MSG_KICK:
	    if (List_size(params) > 1
		    && !strcmp(List_at(params, 0), self->name))
	    {
		const char *nick = List_at(params, 1);
		if (!strcmp(nick, IrcServer_nick(self->server)))
		{
		    self->isJoined = 0;
		    if (self->wantJoined)
		    {
			self->rejointicks = REJOINTICKS;
			Event_register(Service_tick(), self, waitrejoin, 0);
		    }
		    Event_raise(self->parted, 0, 0);
		}
		else if (HashTable_delete(self->nicks, nick))
		{
		    char *ea = IB_copystr(nick);
		    Event_raise(self->left, 0, ea);
		    free(ea);
		}
	    }
	    break;

	case RPL_NAMREPLY:
	    if (List_size(params) == 4
		    && !strcmp(List_at(params, 2), self->name))
	    {
		char *nicklist = IB_copystr(List_at(params, 3));
		char *i = nicklist;
		char *nick = strsep(&i, " ");
		while (nick)
		{
		    if (*nick && ((*nick != '@' && *nick != '%'
				    && *nick != '@') || *++nick)
			    && strcmp(nick, IrcServer_nick(self->server)))
		    {
			HashTable_set(self->nicks, nick, self->name, 0);
		    }
		    nick = strsep(&i, " ");
		}
		free(nicklist);
	    }
	    break;

	case RPL_ENDOFNAMES:
	    if (List_size(params) > 1
		    && !strcmp(List_at(params, 1), self->name))
	    {
		self->isJoined = 1;
		Event_unregister(Service_tick(), self, waitjoin, 0);
		Event_unregister(Service_tick(), self, waitrejoin, 0);
		Event_raise(self->joined, 0, 0);
	    }
	    break;

	case ERR_NOSUCHCHANNEL:
	    if (List_size(params) && !strcmp(List_at(params, 0), self->name))
	    {
		Event_unregister(IrcServer_connected(self->server), self,
			joinOnConnect, 0);
		Event_unregister(Service_tick(), self, waitjoin, 0);
		Event_unregister(Service_tick(), self, waitrejoin, 0);
		Event_unregister(IrcServer_disconnected(self->server), self,
			disconnected, 0);
		Event_raise(self->failed, 0, 0);
	    }
	    break;

	default: ;
    }
}

SOLOCAL Event *IrcChannel_joined(IrcChannel *self)
{
    return self->joined;
}

SOLOCAL Event *IrcChannel_parted(IrcChannel *self)
{
    return self->parted;
}

SOLOCAL Event *IrcChannel_entered(IrcChannel *self)
{
    return self->entered;
}

SOLOCAL Event *IrcChannel_left(IrcChannel *self)
{
    return self->left;
}

SOLOCAL Event *IrcChannel_failed(IrcChannel *self)
{
    return self->failed;
}

SOLOCAL void IrcChannel_destroy(IrcChannel *self)
{
    if (!self) return;
    Event *msgev = IrcServer_msgReceived(self->server);
    Event_unregister(msgev, self, handleMsg, ERR_NOSUCHCHANNEL);
    Event_unregister(msgev, self, handleMsg, RPL_ENDOFNAMES);
    Event_unregister(msgev, self, handleMsg, RPL_NAMREPLY);
    Event_unregister(msgev, self, handleMsg, MSG_KICK);
    Event_unregister(msgev, self, handleMsg, MSG_NICK);
    Event_unregister(msgev, self, handleMsg, MSG_QUIT);
    Event_unregister(msgev, self, handleMsg, MSG_PART);
    Event_unregister(msgev, self, handleMsg, MSG_JOIN);
    Event_destroy(self->failed);
    Event_destroy(self->left);
    Event_destroy(self->entered);
    Event_destroy(self->parted);
    Event_destroy(self->joined);
    HashTable_destroy(self->nicks);
    free(self->name);
    free(self);
}

