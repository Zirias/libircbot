#include <ircbot/event.h>
#include <ircbot/ircserver.h>
#include <ircbot/log.h>

#include "client.h"
#include "connection.h"
#include "ircmessage.h"
#include "queue.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

struct IrcServer {
    const char *remotehost;
    int port;
    char *nick;
    const char *user;
    const char *realname;
    Connection *conn;
    Queue *sendQueue;
    Event *connected;
    Event *disconnected;
    Event *msgReceived;
    char *sendcmd;
    int sending;
    uint16_t recvbufsz;
    uint8_t recvbuf[8192];
};

static void connConnected(void *receiver, void *sender, void *args);
static void connClosed(void *receiver, void *sender, void *args);
static void connDataReceived(void *receiver, void *sender, void *args);
static void connDataSent(void *receiver, void *sender, void *args);

static void sendRaw(IrcServer *self, const char *command);
static void handleMessage(IrcServer *self, const IrcMessage *msg);

SOEXPORT IrcServer *IrcServer_create(const char *remotehost, int port,
	const char *nick, const char *user, const char *realname)
{
    IrcServer *self = xmalloc(sizeof *self);
    self->remotehost = remotehost;
    self->port = port;
    self->nick = copystr(nick);
    self->user = user;
    self->realname = realname;
    self->conn = 0;
    self->sendQueue = 0;
    self->connected = Event_create(self);
    self->disconnected = Event_create(self);
    self->msgReceived = Event_create(self);
    self->sendcmd = 0;
    self->sending = 0;
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
	Event_register(Connection_dataReceived(self->conn), self,
		connDataReceived, 0);
	Event_register(Connection_dataSent(self->conn), self,
		connDataSent, 0);
	self->sendQueue = Queue_create();

	char buf[512];
	snprintf(buf, 512, "NICK %s\r\n", self->nick);
	sendRaw(self, buf);
	snprintf(buf, 512, "USER %s 0 * :%s\r\n", self->user, self->realname);
	sendRaw(self, buf);
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
	Queue_destroy(self->sendQueue);
	self->sendQueue = 0;
	Event_raise(self->disconnected, 0, 0);
    }
}

static void connDataReceived(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    Connection *conn = sender;
    DataReceivedEventArgs *dra = args;

    if (conn != self->conn) return;

    memcpy(self->recvbuf + self->recvbufsz, dra->buf, dra->size);
    self->recvbufsz += dra->size;
    uint16_t pos = 0;
    uint16_t oldpos = (uint16_t)-1;
    while (oldpos != pos)
    {
	oldpos = pos;
	IrcMessage *msg = IrcMessage_create(self->recvbuf,
		self->recvbufsz - pos, &pos);
	if (msg) handleMessage(self, msg);
	IrcMessage_destroy(msg);
    }
    if (pos == self->recvbufsz)
    {
	self->recvbufsz = 0;
    }
    else if (self->recvbufsz - pos > 4096)
    {
	logmsg(L_ERROR, "IrcServer: protocol error.");
	self->recvbufsz = 0;
    }
    else
    {
	memmove(self->recvbuf, self->recvbuf + pos, self->recvbufsz - pos);
	self->recvbufsz -= pos;
    }
}

static void connDataSent(void *receiver, void *sender, void *args)
{
    IrcServer *self = receiver;
    Connection *conn = sender;
    (void) args;

    if (conn == self->conn)
    {
	free(self->sendcmd);
	char *nextcmd = Queue_dequeue(self->sendQueue);
	if (nextcmd)
	{
	    self->sendcmd = nextcmd;
	    Connection_write(self->conn, (const uint8_t *)self->sendcmd,
		    (uint16_t)strlen(self->sendcmd), 0);
	}
	else self->sending = 0;
    }
}

static void sendRaw(IrcServer *self, const char *command)
{
    char *cmd = copystr(command);
    if (self->sending)
    {
	Queue_enqueue(self->sendQueue, cmd, free);
    }
    else
    {
	self->sendcmd = cmd;
	self->sending = 1;
	Connection_write(self->conn, (const uint8_t *)self->sendcmd,
		(uint16_t)strlen(self->sendcmd), 0);
    }
}

static void handleMessage(IrcServer *self, const IrcMessage *msg)
{
    (void) self;
    (void) msg;
}

void IrcServer_connect(IrcServer *self)
{
    if (self->conn) return;
    self->conn = Connection_createTcpClient(self->remotehost, self->port, 1);
    Event_register(Connection_connected(self->conn), self, connConnected, 0);
    Event_register(Connection_closed(self->conn), self, connClosed, 0);
}

