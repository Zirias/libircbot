#include <ircbot/event.h>
#include <ircbot/ircserver.h>
#include <ircbot/log.h>
#include <ircbot/list.h>
#include <ircbot/queue.h>

#include "client.h"
#include "connection.h"
#include "ircchannel.h"
#include "ircmessage.h"
#include "service.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

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
    List *channels;
    List *activeChannels;
    Connection *conn;
    Queue *sendQueue;
    Event *connected;
    Event *disconnected;
    Event *msgReceived;
    Event *joined;
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

static void sendRaw(IrcServer *self, const char *command);
static void sendRawCmd(IrcServer *self, const char *cmd, const char *args);
static void handleMessage(IrcServer *self, const IrcMessage *msg);

SOEXPORT IrcServer *IrcServer_create(const char *id,
	const char *remotehost, int port,
	const char *nick, const char *user, const char *realname)
{
    IrcServer *self = xmalloc(sizeof *self);
    self->id = id;
    self->remotehost = remotehost;
    self->port = port;
    self->name = 0;
    self->nick = copystr(nick);
    self->user = user;
    self->realname = realname;
    self->channels = List_create();
    self->activeChannels = List_create();
    self->conn = 0;
    self->sendQueue = 0;
    self->connected = Event_create(self);
    self->disconnected = Event_create(self);
    self->msgReceived = Event_create(self);
    self->joined = Event_create(self);
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
	logfmt(L_INFO, "IrcServer: connected to %s, waiting for data ...",
		Connection_remoteAddr(self->conn));
	Event_register(Connection_dataReceived(self->conn), self,
		connDataReceived, 0);
	Event_register(Connection_dataSent(self->conn), self,
		connDataSent, 0);
	self->sendQueue = Queue_create();
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
	Queue_destroy(self->sendQueue);
	self->sendQueue = 0;
	Event_raise(self->disconnected, 0, 0);
	logfmt(L_INFO, "IrcServer: [%s] disconnected", servername(self));
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
	logfmt(L_ERROR, "IrcServer: [%s] protocol error.", servername(self));
	self->recvbufsz = 0;
    }
    else
    {
	memmove(self->recvbuf, self->recvbuf + pos, self->recvbufsz - pos);
	self->recvbufsz -= pos;
    }

    if (self->connst == -1)
    {
	logfmt(L_INFO, "IrcServer: [%s] sending login data ...",
		servername(self));
	sendRawCmd(self, "NICK", self->nick);
	char buf[512];
	snprintf(buf, 512, "%s 0 * :%s", self->user, self->realname);
	sendRawCmd(self, "USER", buf);
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
	logmsg(L_DEBUG, "IrcServer: sending confirmed");
	free(self->sendcmd);
	self->sendcmd = 0;
	char *nextcmd = Queue_dequeue(self->sendQueue);
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
	logfmt(L_INFO, "IrcServer: [%s] reconnecting ...", servername(self));
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
	logfmt(L_WARNING,
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
	logfmt(L_INFO, "IrcServer: [%s] pinging idle connection ...",
		servername(self));
	sendRawCmd(self, "PING", self->nick);
    }
    else if (self->idleticks <= -PINGTICKS)
    {
	Event_unregister(Service_tick(), self, checkIdle, 0);
	logfmt(L_WARNING,
		"IrcServer: [%s] timeout waiting for pong, disconnecting ...",
		servername(self));
	Connection_close(self->conn);
    }
}

static void sendRaw(IrcServer *self, const char *command)
{
    char *cmd = copystr(command);
    logfmt(L_DEBUG, "IrcServer: sending %s", cmd);
    if (self->sending)
    {
	Queue_enqueue(self->sendQueue, cmd, free);
    }
    else
    {
	self->sendcmd = cmd;
	self->sending = 1;
	Connection_write(self->conn, (const uint8_t *)self->sendcmd,
		(uint16_t)strlen(self->sendcmd), self);
    }
}

static void sendRawCmd(IrcServer *self, const char *cmd, const char *args)
{
    char buf[513];
    snprintf(buf, 513, "%s %s\r\n", cmd, args);
    sendRaw(self, buf);
}

static inline void destroyIrcChannel(void *chan)
{
    IrcChannel_destroy(chan);
}

static void handleMessage(IrcServer *self, const IrcMessage *msg)
{
    const char *cmd = IrcMessage_command(msg);
    const List *params = IrcMessage_params(msg);
    if (!strcmp(cmd, "PRIVMSG") && List_size(params) == 2)
    {
	MsgReceivedEventArgs ea = {
	    .from = IrcMessage_prefix(msg),
	    .to = List_at(params, 0),
	    .message = List_at(params, 1)
	};
	if (ea.from)
	{
	    logfmt(L_DEBUG, "IrcServer: message from %s to %s: %s",
		    ea.from, ea.to, ea.message);
	}
	else
	{
	    logfmt(L_DEBUG, "IrcServer: message to %s: %s", ea.to, ea.message);
	}
	Event_raise(self->msgReceived, 0, &ea);
    }
    else if (!strcmp(cmd, "004") && List_size(params) > 2)
    {
	free(self->name);
	self->name = copystr(List_at(params, 1));
	logfmt(L_INFO, "IrcServer: [%s] connected and logged in", self->name);
	self->connst = 1;
	Event_unregister(Service_tick(), self, connWaitLogin, 0);
	Event_register(Service_tick(), self, checkIdle, 0);
	if (List_size(self->channels))
	{
	    ListIterator *i = List_iterator(self->channels);
	    while (ListIterator_moveNext(i))
	    {
		sendRawCmd(self, "JOIN", ListIterator_current(i));
	    }
	    ListIterator_destroy(i);
	}
	Event_raise(self->connected, 0, 0);
    }
    else if (!strcmp(cmd, "432"))
    {
	size_t nicklen = strlen(self->nick);
	self->nick = xrealloc(self->nick, nicklen+2);
	strcpy(self->nick+nicklen, "_");
	sendRawCmd(self, "NICK", self->nick);
    }
    else if (!strcmp(cmd, "PING"))
    {
	sendRawCmd(self, "PONG", IrcMessage_rawParams(msg));
    }
    else if (!strcmp(cmd, "JOIN") && List_size(params)
	    && !strncmp(IrcMessage_prefix(msg), self->nick, strlen(self->nick))
	    && strcspn(IrcMessage_prefix(msg), "!") == strlen(self->nick))
    {
	IrcChannel *channel = IrcChannel_create(List_at(params, 0));
	List_append(self->activeChannels, channel, destroyIrcChannel);
	Event_raise(self->joined, 0, channel);
    }
    else if (!strcmp(cmd, "JOIN") || !strcmp(cmd, "PART")
	    || !strcmp(cmd, "QUIT") || !strcmp(cmd, "NICK")
	    || !strcmp(cmd, "353") || !strcmp(cmd, "366"))
    {
	ListIterator *i = List_iterator(self->activeChannels);
	while (ListIterator_moveNext(i))
	{
	    IrcChannel_handleMessage(ListIterator_current(i), msg);
	}
	ListIterator_destroy(i);
    }
}

SOEXPORT int IrcServer_connect(IrcServer *self)
{
    if (self->conn) return 0;
    logmsg(L_DEBUG, "IrcServer: initiating TCP connection");
    self->conn = Connection_createTcpClient(self->remotehost, self->port, 1);
    if (!self->conn) return -1;
    Event_register(Connection_connected(self->conn), self, connConnected, 0);
    Event_register(Connection_closed(self->conn), self, connClosed, 0);
    return 0;
}

SOEXPORT void IrcServer_disconnect(IrcServer *self)
{
    if (!self->conn) return;
    sendRaw(self, "QUIT :bye.\r\n");
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

SOEXPORT const List *IrcServer_channels(const IrcServer *self)
{
    return self->activeChannels;
}

SOEXPORT IrcChannel *IrcServer_channel(
	const IrcServer *self, const char *channel)
{
    if (!self->activeChannels) return 0;
    IrcChannel *res = 0;
    ListIterator *i = List_iterator(self->activeChannels);
    while (ListIterator_moveNext(i))
    {
	IrcChannel *curr = ListIterator_current(i);
	if (!strcmp(IrcChannel_name(curr), channel))
	{
	    res = curr;
	    break;
	}
    }
    ListIterator_destroy(i);
    return res;
}

SOEXPORT void IrcServer_setNick(IrcServer *self, const char *nick)
{
    if (strcmp(nick, self->nick))
    {
	free(self->nick);
	self->nick = copystr(nick);
	if (self->connst > 0)
	{
	    sendRawCmd(self, "NICK", self->nick);
	}
    }
}

SOEXPORT void IrcServer_join(IrcServer *self, const char *channel)
{
    List_append(self->channels, copystr(channel), free);
    if (self->connst > 0) sendRawCmd(self, "JOIN", channel);
}

static int compareChannel(void *a, const void *b)
{
    const char *ca = a;
    const char *cb = b;
    return !strcmp(ca, cb);
}

SOEXPORT void IrcServer_part(IrcServer *self, const char *channel)
{
    List_removeAll(self->channels, compareChannel, channel);
    if (self->connst > 0) sendRawCmd(self, "PART", channel);
}

SOEXPORT int IrcServer_sendMsg(IrcServer *self,
	const char *to, const char *message, int action)
{
    if (self->connst <= 0) return -1;
    if (strlen(to) > 255)
    {
	logfmt(L_ERROR, "IrcServer: [%s] Invalid message recipient %s",
		servername(self), to);
	return -1;
    }
    char rawmsg[513];
    int idx = sprintf(rawmsg,
	    action ? "PRIVMSG %s :\001ACTION " : "PRIVMSG %s :", to);
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

SOEXPORT Event *IrcServer_connected(IrcServer *self)
{
    return self->connected;
}

SOEXPORT Event *IrcServer_disconnected(IrcServer *self)
{
    return self->disconnected;
}

SOEXPORT Event *IrcServer_msgReceived(IrcServer *self)
{
    return self->msgReceived;
}

SOEXPORT Event *IrcServer_joined(IrcServer *self)
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
	Queue_destroy(self->sendQueue);
	Connection_close(self->conn);
    }
    Event_destroy(self->connected);
    Event_destroy(self->disconnected);
    Event_destroy(self->msgReceived);
    Event_destroy(self->joined);
    List_destroy(self->channels);
    List_destroy(self->activeChannels);
    free(self->sendcmd);
    free(self->name);
    free(self->nick);
    free(self);
}
