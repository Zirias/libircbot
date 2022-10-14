#include <ircbot/hashtable.h>
#include <ircbot/irccommand.h>
#include <ircbot/log.h>
#include <ircbot/list.h>
#include <ircbot/queue.h>

#include "client.h"
#include "connection.h"
#include "event.h"
#include "ircchannel.h"
#include "ircmessage.h"
#include "ircserver.h"
#include "service.h"
#include "util.h"

#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define RECONNTICKS 300
#define QUICKRECONNTICKS 5
#define LOGINTICKS 20
#define IDLETICKS 180
#define PINGTICKS 3

struct IrcServer {
    const char *id;
    const char *remotehost;
    int port;
    char *name;
    char *nick;
    const char *user;
    const char *realname;
    IBHashTable *channels;
    Connection *conn;
    IBQueue *sendQueue;
    Event *connected;
    Event *disconnected;
    Event *msgReceived;
    Event *joined;
    Event *parted;
    char *sendcmd;
    int sending;
    int connst;
    int loginticks;
    int reconnticks;
    int idleticks;
    uint16_t recvbufsz;
    uint8_t recvbuf[8192];
};

#define servername(s) ((s)->name ? (s)->name : (s)->remotehost)

static void connConnected(void *receiver, void *sender, void *args);
static void connClosed(void *receiver, void *sender, void *args);
static void connDataReceived(void *receiver, void *sender, void *args);
static void connDataSent(void *receiver, void *sender, void *args);
static void connWaitReconn(void *receiver, void *sender, void *args);
static void connWaitLogin(void *receiver, void *sender, void *args);
static void checkIdle(void *receiver, void *sender, void *args);
static void chanJoined(void *receiver, void *sender, void *args);
static void chanParted(void *receiver, void *sender, void *args);
static void chanFailed(void *receiver, void *sender, void *args);

static void sendRaw(IrcServer *self, const char *command);
static void sendRawCmd(IrcServer *self, IrcCommand cmd, const char *args);
static void handleMessage(IrcServer *self, const IrcMessage *msg);

SOEXPORT IrcServer *IrcServer_create(const char *id,
	const char *remotehost, int port,
	const char *nick, const char *user, const char *realname)
{
    IrcServer *self = IB_xmalloc(sizeof *self);
    self->id = id;
    self->remotehost = remotehost;
    self->port = port;
    self->name = 0;
    self->nick = IB_copystr(nick);
    self->user = user;
    self->realname = realname;
    self->channels = IBHashTable_create(6);
    self->conn = 0;
    self->sendQueue = 0;
    self->connected = Event_create(self);
    self->disconnected = Event_create(self);
    self->msgReceived = Event_create(self);
    self->joined = Event_create(self);
    self->parted = Event_create(self);
    self->sendcmd = 0;
    self->sending = 0;
    self->connst = 0;
    self->recvbufsz = 0;
    return self;
}

static void connConnected(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    Connection *conn = sender;
    (void) args;

    if (conn == self->conn)
    {
	IBLog_fmt(L_INFO, "IrcServer: connected to %s, waiting for data ...",
		Connection_remoteAddr(self->conn));
	Event_register(Connection_dataReceived(self->conn), self,
		connDataReceived, 0);
	Event_register(Connection_dataSent(self->conn), self,
		connDataSent, 0);
	self->sendQueue = IBQueue_create();
	self->connst = -1;
	self->loginticks = LOGINTICKS;
	Event_register(Service_tick(), self, connWaitLogin, 0);
    }
}

static void connClosed(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    Connection *conn = sender;
    (void) args;

    if (conn == self->conn)
    {
	self->conn = 0;
	self->reconnticks = self->connst ? QUICKRECONNTICKS : RECONNTICKS;
	self->connst = 0;
	IBQueue_destroy(self->sendQueue);
	self->sendQueue = 0;
	Event_raise(self->disconnected, 0, 0);
	IBLog_fmt(L_INFO, "IrcServer: [%s] disconnected", servername(self));
	free(self->name);
	self->name = 0;
	Event_unregister(Service_tick(), self, connWaitLogin, 0);
	Event_register(Service_tick(), self, connWaitReconn, 0);
    }
}

static void connDataReceived(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    Connection *conn = sender;
    DataReceivedEventArgs *dra = args;

    if (conn != self->conn) return;

    self->idleticks = IDLETICKS;

    memcpy(self->recvbuf + self->recvbufsz, dra->buf, dra->size);
    self->recvbufsz += dra->size;
    uint16_t pos = 0;
    uint16_t oldpos = (uint16_t)-1;
    while (oldpos != pos)
    {
	oldpos = pos;
	IrcMessage *msg = IrcMessage_create(self->recvbuf,
		self->recvbufsz, &pos);
	if (msg) handleMessage(self, msg);
	IrcMessage_destroy(msg);
    }
    if (pos == self->recvbufsz)
    {
	self->recvbufsz = 0;
    }
    else if (self->recvbufsz - pos > 4096)
    {
	IBLog_fmt(L_ERROR, "IrcServer: [%s] protocol error.", servername(self));
	self->recvbufsz = 0;
    }
    else
    {
	memmove(self->recvbuf, self->recvbuf + pos, self->recvbufsz - pos);
	self->recvbufsz -= pos;
    }

    if (self->connst == -1)
    {
	IBLog_fmt(L_INFO, "IrcServer: [%s] sending login data ...",
		servername(self));
	sendRawCmd(self, MSG_NICK, self->nick);
	const char *user = self->user;
	const char *realname = self->realname;
	struct passwd *passwd = 0;
	char nmbuf[64];
	if (!user)
	{
	    passwd = getpwuid(getuid());
	    user = passwd ? passwd->pw_name : "nobody";
	}
	if (!realname)
	{
	    if (!passwd) passwd = getpwnam(user);
	    if (!passwd) passwd = getpwuid(getuid());
	    if (passwd)
	    {
		char *rawnm = passwd->pw_gecos;
		rawnm[strcspn(rawnm, ",")] = 0;
		size_t amperpos = strcspn(rawnm, "&");
		if (rawnm[amperpos])
		{
		    char *nmtail = rawnm+amperpos+1;
		    rawnm[amperpos] = 0;
		    snprintf(nmbuf, 64, "%s%s%s",
			    rawnm, passwd->pw_name, nmtail);
		    nmbuf[amperpos] = toupper(nmbuf[amperpos]);
		}
		else
		{
		    snprintf(nmbuf, 64, "%s", rawnm);
		}
		realname = nmbuf;
	    }
	    else realname = self->nick;
	}
	char buf[512];
	snprintf(buf, 512, "%s 0 * :%s", user, realname);
	sendRawCmd(self, MSG_USER, buf);
	self->connst = -2;
    }
}

static void connDataSent(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    Connection *conn = sender;
    (void) args;

    if (conn == self->conn)
    {
	IBLog_msg(L_DEBUG, "IrcServer: sending confirmed");
	free(self->sendcmd);
	self->sendcmd = 0;
	char *nextcmd = IBQueue_dequeue(self->sendQueue);
	if (nextcmd)
	{
	    self->sendcmd = nextcmd;
	    Connection_write(self->conn, (const uint8_t *)self->sendcmd,
		    (uint16_t)strlen(self->sendcmd), self);
	}
	else self->sending = 0;
    }
}

static void connWaitReconn(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    (void)sender;
    (void)args;

    if (--self->reconnticks <= 0)
    {
	IBLog_fmt(L_INFO, "IrcServer: [%s] reconnecting ...", servername(self));
	if (IrcServer_connect(self) < 0)
	{
	    self->reconnticks = RECONNTICKS;
	}
	else
	{
	    Event_unregister(Service_tick(), self, connWaitReconn, 0);
	}
    }
}

static void connWaitLogin(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    (void)sender;
    (void)args;

    if (--self->loginticks <= 0)
    {
	Event_unregister(Service_tick(), self, connWaitLogin, 0);
	IBLog_fmt(L_WARNING,
		"IrcServer: [%s] timeout waiting for login, disconnecting ...",
		servername(self));
	Connection_close(self->conn);
    }
}

static void checkIdle(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    (void)sender;
    (void)args;

    if (--self->idleticks == 0)
    {
	IBLog_fmt(L_INFO, "IrcServer: [%s] pinging idle connection ...",
		servername(self));
	sendRawCmd(self, MSG_PING, self->nick);
    }
    else if (self->idleticks <= -PINGTICKS)
    {
	Event_unregister(Service_tick(), self, checkIdle, 0);
	IBLog_fmt(L_WARNING,
		"IrcServer: [%s] timeout waiting for pong, disconnecting ...",
		servername(self));
	Connection_close(self->conn);
    }
}

static void chanJoined(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    IrcChannel *chan = sender;
    (void) args;

    Event_raise(self->joined, 0, chan);
}

static void chanParted(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    IrcChannel *chan = sender;
    (void) args;

    Event_raise(self->parted, 0, chan);
}

static void chanFailed(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    IrcChannel *chan = sender;
    (void) args;

    const char *channel = IrcChannel_name(chan);
    Event_unregister(IrcChannel_joined(chan), self, chanJoined, 0);
    Event_unregister(IrcChannel_parted(chan), self, chanParted, 0);
    Event_unregister(IrcChannel_failed(chan), self, chanFailed, 0);
    IrcChannel_destroy(chan);
    IBHashTable_delete(self->channels, channel);
}

static void sendRaw(IrcServer *self, const char *command)
{
    char *cmd = IB_copystr(command);
    IBLog_fmt(L_DEBUG, "IrcServer: sending %s", cmd);
    if (self->sending)
    {
	IBQueue_enqueue(self->sendQueue, cmd, free);
    }
    else
    {
	self->sendcmd = cmd;
	self->sending = 1;
	Connection_write(self->conn, (const uint8_t *)self->sendcmd,
		(uint16_t)strlen(self->sendcmd), self);
    }
}

static void sendRawCmd(IrcServer *self, IrcCommand cmd, const char *args)
{
    char buf[513];
    snprintf(buf, 513, "%s %s\r\n", IrcCommand_str(cmd), args);
    sendRaw(self, buf);
}

static inline void destroyIrcChannel(void *chan)
{
    IrcChannel_destroy(chan);
}

static void handleMessage(IrcServer *self, const IrcMessage *msg)
{
    IrcCommand cmd = IrcMessage_command(msg);
    const IBList *params = IrcMessage_params(msg);

    switch (cmd)
    {
	case RPL_MYINFO:
	    if (IBList_size(params) > 2)
	    {
		free(self->name);
		self->name = IB_copystr(IBList_at(params, 1));
		IBLog_fmt(L_INFO, "IrcServer: [%s] connected and logged in",
			self->name);
		self->connst = 1;
		Event_unregister(Service_tick(), self, connWaitLogin, 0);
		Event_register(Service_tick(), self, checkIdle, 0);
		Event_raise(self->connected, 0, 0);
	    }
	    break;

	case ERR_NICKNAMEINUSE:
	    {
		size_t nicklen = strlen(self->nick);
		self->nick = IB_xrealloc(self->nick, nicklen+2);
		strcpy(self->nick+nicklen, "_");
		sendRawCmd(self, MSG_NICK, self->nick);
	    }
	    break;

	case MSG_PING:
	    sendRawCmd(self, MSG_PONG, IrcMessage_rawParams(msg));
	    break;

	case MSG_NICK:
	    if (IBList_size(params)
		    && !strncmp(IrcMessage_prefix(msg),
			self->nick, strlen(self->nick))
		    && strcspn(IrcMessage_prefix(msg),
			"!") == strlen(self->nick))
	    {
		free(self->nick);
		self->nick = IB_copystr(IBList_at(params, 0));
	    }
	    break;

	case MSG_JOIN:
	    if (IBList_size(params)
		    && !strncmp(IrcMessage_prefix(msg),
			self->nick, strlen(self->nick))
		    && strcspn(IrcMessage_prefix(msg),
			"!") == strlen(self->nick))
	    {
		const char *chan = IBList_at(params, 0);
		if (!IBHashTable_get(self->channels, chan))
		{
		    IrcChannel *channel = IrcChannel_create(self,
			    IBList_at(params, 0));
		    Event_register(IrcChannel_joined(channel), self,
			    chanJoined, 0);
		    Event_register(IrcChannel_parted(channel), self,
			    chanParted, 0);
		    Event_register(IrcChannel_failed(channel), self,
			    chanFailed, 0);
		    IBHashTable_set(self->channels, chan,
			    channel, destroyIrcChannel);
		}
	    }
	    break;

	default: ;
    }

    Event_raise(self->msgReceived, cmd, (void *)msg);
}

SOLOCAL int IrcServer_connect(IrcServer *self)
{
    if (self->conn) return 0;
    IBLog_msg(L_DEBUG, "IrcServer: initiating TCP connection");
    self->conn = Connection_createTcpClient(self->remotehost, self->port, 1);
    if (!self->conn) return -1;
    Event_register(Connection_connected(self->conn), self, connConnected, 0);
    Event_register(Connection_closed(self->conn), self, connClosed, 0);
    return 0;
}

SOLOCAL void IrcServer_disconnect(IrcServer *self)
{
    Event_unregister(Service_tick(), self, connWaitLogin, 0);
    Event_unregister(Service_tick(), self, connWaitReconn, 0);
    if (self->connst > 0) sendRawCmd(self, MSG_QUIT, ":bye.");
    else if (self->conn) Connection_close(self->conn);
}

SOEXPORT const char *IrcServer_id(const IrcServer *self)
{
    return self->id;
}

SOEXPORT const char *IrcServer_name(const IrcServer *self)
{
    return servername(self);
}

SOEXPORT const char *IrcServer_nick(const IrcServer *self)
{
    return self->nick;
}

SOEXPORT const IBHashTable *IrcServer_channels(const IrcServer *self)
{
    return self->channels;
}

SOLOCAL void IrcServer_setNick(IrcServer *self, const char *nick)
{
    if (strcmp(nick, self->nick))
    {
	free(self->nick);
	self->nick = IB_copystr(nick);
	if (self->connst > 0)
	{
	    sendRawCmd(self, MSG_NICK, self->nick);
	}
    }
}

SOEXPORT void IrcServer_join(IrcServer *self, const char *channel)
{
    IrcChannel *chan = IBHashTable_get(self->channels, channel);
    if (!chan)
    {
	chan = IrcChannel_create(self, channel);
	Event_register(IrcChannel_joined(chan), self, chanJoined, 0);
	Event_register(IrcChannel_parted(chan), self, chanParted, 0);
	Event_register(IrcChannel_failed(chan), self, chanFailed, 0);
	IBHashTable_set(self->channels, channel, chan, destroyIrcChannel);
    }
    IrcChannel_join(chan);
}

SOEXPORT void IrcServer_part(IrcServer *self, const char *channel)
{
    IrcChannel *chan = IBHashTable_get(self->channels, channel);
    if (chan)
    {
	IrcChannel_part(chan);
	Event_unregister(IrcChannel_joined(chan), self, chanJoined, 0);
	Event_unregister(IrcChannel_parted(chan), self, chanParted, 0);
	Event_unregister(IrcChannel_failed(chan), self, chanFailed, 0);
	IrcChannel_destroy(chan);
	IBHashTable_delete(self->channels, channel);
    }
}

SOLOCAL int IrcServer_sendCmd(IrcServer *self, IrcCommand cmd,
	const char *args)
{
    if (self->connst <= 0) return -1;
    sendRawCmd(self, cmd, args);
    return 0;
}

SOLOCAL int IrcServer_sendMsg(IrcServer *self,
	const char *to, const char *message, int action)
{
    if (self->connst <= 0) return -1;
    if (strlen(to) > 255)
    {
	IBLog_fmt(L_ERROR, "IrcServer: [%s] Invalid message recipient %s",
		servername(self), to);
	return -1;
    }
    char rawmsg[513];
    const char *cmd = IrcCommand_str(MSG_PRIVMSG);
    int idx = sprintf(rawmsg,
	    action ? "%s %s :\001ACTION " : "%s %s :", cmd, to);
    size_t maxchunk = sizeof rawmsg - idx - 3 - !!action;
    size_t msglen = strlen(message);
    char *tgt = rawmsg + idx;
    while (msglen)
    {
	size_t chunksz = (msglen > maxchunk) ? maxchunk : msglen;
	strncpy(tgt, message, chunksz);
	strcpy(tgt+chunksz, &"\001\r\n"[!action]);
	sendRaw(self, rawmsg);
	message += chunksz;
	msglen -= chunksz;
    }
    return 0;
}

SOLOCAL Event *IrcServer_connected(IrcServer *self)
{
    return self->connected;
}

SOLOCAL Event *IrcServer_disconnected(IrcServer *self)
{
    return self->disconnected;
}

SOLOCAL Event *IrcServer_msgReceived(IrcServer *self)
{
    return self->msgReceived;
}

SOLOCAL Event *IrcServer_joined(IrcServer *self)
{
    return self->joined;
}

SOEXPORT void IrcServer_destroy(IrcServer *self)
{
    if (!self) return;
    if (self->conn)
    {
	Event_unregister(Connection_dataReceived(self->conn), self,
		connDataReceived, 0);
	Event_unregister(Connection_dataSent(self->conn), self,
		connDataSent, 0);
	Event_unregister(Connection_connected(self->conn), self,
		connConnected, 0);
	Event_unregister(Connection_closed(self->conn), self,
		connClosed, 0);
	IrcServer_disconnect(self);
	IBQueue_destroy(self->sendQueue);
	Connection_close(self->conn);
    }
    Event_destroy(self->connected);
    Event_destroy(self->disconnected);
    Event_destroy(self->msgReceived);
    Event_destroy(self->joined);
    Event_destroy(self->parted);
    IBHashTable_destroy(self->channels);
    free(self->sendcmd);
    free(self->name);
    free(self->nick);
    free(self);
}
