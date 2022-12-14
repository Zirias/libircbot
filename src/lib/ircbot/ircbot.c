#include <ircbot/hashtable.h>
#include <ircbot/irccommand.h>
#include <ircbot/list.h>
#include <ircbot/log.h>

#include "daemon.h"
#include "event.h"
#include "ircbot.h"
#include "ircchannel.h"
#include "ircmessage.h"
#include "ircserver.h"
#include "service.h"
#include "threadpool.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

struct IrcBotResponse
{
    IBList *messages;
};

struct IrcBotEvent
{
    IrcServer *server;
    char *origin;
    char *command;
    char *from;
    char *arg;
    IrcBotResponse response;
    IrcBotEventType type;
};

typedef struct IrcBotEventHandler
{
    IrcBotHandler handler;
    const char *serverId;
    const char *origin;
    const char *filter;
    IrcBotEventType type;
} IrcBotEventHandler;

typedef struct IrcBotResponseMessage
{
    char *to;
    char *msg;
    int action;
} IrcBotResponseMessage;

typedef struct HandlerThreadProcArg
{
    IrcBotEventHandler *hdl;
    IrcBotEvent *e;
} HandlerThreadProcArg;

static IBThreadOpts threadOpts = {
    .nThreads = 0,
    .maxThreads = 128,
    .nPerCpu = 2,
    .defNThreads = 8,
    .queueLen = 0,
    .maxQueueLen = 1024,
    .minQueueLen = 64,
    .qLenPerThread = 2
};

static DaemonOpts daemonOpts = {
    .started = 0,
    .pidfile = 0,
    .uid = -1,
    .gid = -1,
    .daemonize = 0
};

static int (*startupfunc)(void) = 0;
static void (*shutdownfunc)(void) = 0;
static IBList *servers = 0;
static IBList *handlers = 0;

static IrcBotEvent *createBotEvent(IrcBotEventType type, IrcServer *server,
	const char *origin, const char *command, const char *from,
	const char *arg);
static void destroyBotEvent(IrcBotEvent *e);
static IrcBotEventHandler *findHandler(IrcBotEventType type,
	const char *serverId, const char *origin, const char *filter);
static void handlerThreadProc(void *arg);
static void executeHandler(IrcBotEventHandler *hdl, IrcBotEvent *e);
static void destroyMessage(void *message);

static void handlerJobFinished(void *receiver, void *sender, void *args);
static void startup(void *receiver, void *sender, void *args);
static void shutdownok(void *receiver, void *sender, void *args);
static void shutdown(void *receiver, void *sender, void *args);
static void msgReceived(void *receiver, void *sender, void *args);
static void userJoined(void *receiver, void *sender, void *args);
static void userParted(void *receiver, void *sender, void *args);
static void chanJoined(void *receiver, void *sender, void *args);
static void connected(void *receiver, void *sender, void *args);

static IrcBotEvent *createBotEvent(IrcBotEventType type, IrcServer *server,
	const char *origin, const char *command, const char *from,
	const char *arg)
{
    IrcBotEvent *e = IB_xmalloc(sizeof *e);
    e->server = server;
    e->origin = IB_copystr(origin);
    e->command = IB_copystr(command);
    if (from)
    {
	size_t bangpos = strcspn(from, "!");
	e->from = IB_xmalloc(bangpos+1);
	strncpy(e->from, from, bangpos);
	e->from[bangpos] = 0;
    }
    else e->from = 0;
    e->arg = IB_copystr(arg);
    e->response.messages = IBList_create();
    e->type = type;
    return e;
}

static void destroyBotEvent(IrcBotEvent *e)
{
    if (!e) return;
    IBList_destroy(e->response.messages);
    free(e->arg);
    free(e->from);
    free(e->command);
    free(e->origin);
    free(e);
}

static IrcBotEventHandler *findHandler(IrcBotEventType type,
	const char *serverId, const char *origin, const char *filter)
{
    if (!handlers) return 0;

    IrcBotEventHandler *hdl = 0;
    IBListIterator *i = IBList_iterator(handlers);
    while (IBListIterator_moveNext(i))
    {
	IrcBotEventHandler *h = IBListIterator_current(i);

	if (h->type != type)
	    continue;
	if (h->serverId && (!serverId || strcmp(serverId, h->serverId)))
	    continue;
	if (h->origin && (!origin || strcmp(origin, h->origin)))
	    continue;
	if (h->filter && (!filter || strcmp(filter, h->filter)))
	    continue;

	hdl = h;
	break;
    }
    IBListIterator_destroy(i);
    return hdl;
}

static void handlerThreadProc(void *arg)
{
    HandlerThreadProcArg *tparg = arg;
    tparg->hdl->handler(tparg->e);
}

static void executeHandler(IrcBotEventHandler *hdl, IrcBotEvent *e)
{
    HandlerThreadProcArg *tparg = IB_xmalloc(sizeof *tparg);
    tparg->hdl = hdl;
    tparg->e = e;
    ThreadJob *job = ThreadJob_create(handlerThreadProc, tparg, 30);
    Event_register(ThreadJob_finished(job), 0, handlerJobFinished, 0);
    ThreadPool_enqueue(job);

}

static void destroyMessage(void *message)
{
    if (!message) return;
    IrcBotResponseMessage *msg = message;
    free(msg->msg);
    free(msg->to);
    free(msg);
}

static void handlerJobFinished(void *receiver, void *sender, void *args)
{
    (void)receiver;

    ThreadJob *job = sender;
    HandlerThreadProcArg *tparg = args;

    if (ThreadJob_hasCompleted(job))
    {
	IBListIterator *i = IBList_iterator(tparg->e->response.messages);
	while (IBListIterator_moveNext(i))
	{
	    IrcBotResponseMessage *message = IBListIterator_current(i);
	    IrcServer_sendMsg(tparg->e->server, message->to,
		    message->msg, message->action);
	}
	IBListIterator_destroy(i);
    }
    else IBLog_msg(L_WARNING, "IrcBot: a handler timed out.");

    destroyBotEvent(tparg->e);
    free(tparg);
}

static void startup(void *receiver, void *sender, void *args)
{
    (void)receiver;
    (void)sender;

    StartupEventArgs *ea = args;

    IBListIterator *i = IBList_iterator(servers);
    while (IBListIterator_moveNext(i))
    {
	IrcServer *server = IBListIterator_current(i);
	if (IrcServer_connect(server) < 0)
	{
	    ea->rc = EXIT_FAILURE;
	    break;
	}
	Event_register(IrcServer_connected(server), 0, connected, 0);
	Event_register(IrcServer_joined(server), 0, chanJoined, 0);
	Event_register(IrcServer_msgReceived(server), 0,
		msgReceived, MSG_PRIVMSG);
    }
    IBListIterator_destroy(i);

    if (startupfunc && ea->rc != EXIT_FAILURE)
    {
	ea->rc = startupfunc();
    }

    IBLog_setAsync(1);
    if (daemonOpts.daemonize && ea->rc != EXIT_FAILURE)
    {
	if (daemonOpts.started) daemonOpts.started();
	daemon_launched();
    }
}

static void shutdownok(void *receiver, void *sender, void *args)
{
    (void)receiver;
    (void)args;

    IrcServer *server = sender;
    Event_unregister(IrcServer_disconnected(server), 0, shutdownok, 0);
    Service_shutdownUnlock();
}

static void shutdown(void *receiver, void *sender, void *args)
{
    (void)receiver;
    (void)sender;
    (void)args;

    IBListIterator *i = IBList_iterator(servers);
    while (IBListIterator_moveNext(i))
    {
	IrcServer *server = IBListIterator_current(i);
	Event_register(IrcServer_disconnected(server), 0, shutdownok, 0);
	Service_shutdownLock();
	IrcServer_disconnect(server);
    }
    IBListIterator_destroy(i);

    if (shutdownfunc) shutdownfunc();
}

static void msgReceived(void *receiver, void *sender, void *args)
{
    (void)receiver;

    IrcServer *server = sender;
    const IrcMessage *msg = args;
    const IBList *params = IrcMessage_params(msg);

    if (IBList_size(params) != 2) return;
    const char *from = IrcMessage_prefix(msg);
    const char *to = IBList_at(params, 0);
    const char *message = IBList_at(params, 1);

    IrcChannel *channel = IBHashTable_get(IrcServer_channels(server), to);

    if (message[0] == '!' || !channel)
    {
	if (message[0] == '!') ++message;
	size_t cmdlen = strcspn(message, " \t\r\n");
	if (cmdlen && cmdlen < 64)
	{
	    char cmd[64];
	    strncpy(cmd, message, cmdlen);
	    cmd[cmdlen] = 0;
	    IrcBotEventHandler *hdl = findHandler(IBET_BOTCOMMAND,
		    IrcServer_id(server), to, cmd);
	    if (!hdl && !channel)
	    {
		hdl = findHandler(IBET_BOTCOMMAND, IrcServer_id(server),
			ORIGIN_PRIVATE, cmd);
	    }
	    else if (!hdl && channel)
	    {
		hdl = findHandler(IBET_BOTCOMMAND, IrcServer_id(server),
			ORIGIN_CHANNEL, cmd);
	    }
	    if (hdl)
	    {
		const char *arg = 0;
		if (message[cmdlen] && message[cmdlen+1])
		{
		    arg = message+cmdlen+1;
		}
		IrcBotEvent *e = createBotEvent(IBET_BOTCOMMAND, server,
			to, cmd, from, arg);
		executeHandler(hdl, e);
		return;
	    }
	}
    }

    IrcBotEventHandler *hdl = findHandler(IBET_PRIVMSG, IrcServer_id(server),
	    to, 0);
    if (!hdl && !channel)
    {
	hdl = findHandler(IBET_PRIVMSG, IrcServer_id(server),
		ORIGIN_PRIVATE, 0);
    }
    else if (!hdl && channel)
    {
	hdl = findHandler(IBET_PRIVMSG, IrcServer_id(server),
		ORIGIN_CHANNEL, 0);
    }
    if (hdl)
    {
	IrcBotEvent *e = createBotEvent(IBET_PRIVMSG, server,
		to, 0, from, IBList_at(params, 1));
	executeHandler(hdl, e);
    }
}

static void userJoined(void *receiver, void *sender, void *args)
{
    IrcServer *server = receiver;
    IrcChannel *channel = sender;
    const char *nick = args;

    IrcBotEventHandler *hdl = findHandler(IBET_JOINED, IrcServer_id(server),
	    IrcChannel_name(channel), nick);
    if (hdl)
    {
	IrcBotEvent *e = createBotEvent(IBET_JOINED, server,
		IrcChannel_name(channel), 0, 0, nick);
	executeHandler(hdl, e);
    }
}

static void userParted(void *receiver, void *sender, void *args)
{
    IrcServer *server = receiver;
    IrcChannel *channel = sender;
    const char *nick = args;

    IrcBotEventHandler *hdl = findHandler(IBET_PARTED, IrcServer_id(server),
	    IrcChannel_name(channel), nick);
    if (hdl)
    {
	IrcBotEvent *e = createBotEvent(IBET_PARTED, server,
		IrcChannel_name(channel), 0, 0, nick);
	executeHandler(hdl, e);
    }
}

static void chanJoined(void *receiver, void *sender, void *args)
{
    (void)receiver;

    IrcServer *server = sender;
    IrcChannel *channel = args;

    Event_register(IrcChannel_entered(channel), server, userJoined, 0);
    Event_register(IrcChannel_left(channel), server, userParted, 0);

    IrcBotEventHandler *hdl = findHandler(IBET_CHANJOINED,
	    IrcServer_id(server), IrcChannel_name(channel), 0);
    if (hdl)
    {
	IrcBotEvent *e = createBotEvent(IBET_CHANJOINED, server,
		IrcChannel_name(channel), 0, 0, 0);
	executeHandler(hdl, e);
    }
}

static void connected(void *receiver, void *sender, void *args)
{
    (void)receiver;
    (void)args;

    IrcServer *server = sender;
    IrcBotEventHandler *hdl = findHandler(IBET_CONNECTED,
	    IrcServer_id(server), 0, 0);
    if (hdl)
    {
	IrcBotEvent *e = createBotEvent(IBET_CONNECTED, server, 0, 0, 0, 0);
	executeHandler(hdl, e);
    }
}

SOEXPORT IBThreadOpts *IrcBot_threadOpts(void)
{
    return &threadOpts;
}

SOEXPORT void IBThreadOpts_setNThreads(IBThreadOpts *self, int num)
{
    self->nThreads = num;
}

SOEXPORT void IBThreadOpts_setMaxThreads(IBThreadOpts *self, int num)
{
    self->maxThreads = num;
}

SOEXPORT void IBThreadOpts_setNPerCpu(IBThreadOpts *self, int num)
{
    self->nPerCpu = num;
}

SOEXPORT void IBThreadOpts_setDefNThreads(IBThreadOpts *self, int num)
{
    self->defNThreads = num;
}

SOEXPORT void IBThreadOpts_setQueueLen(IBThreadOpts *self, int num)
{
    self->queueLen = num;
}

SOEXPORT void IBThreadOpts_setMaxQueueLen(IBThreadOpts *self, int num)
{
    self->maxQueueLen = num;
}

SOEXPORT void IBThreadOpts_setMinQueueLen(IBThreadOpts *self, int num)
{
    self->minQueueLen = num;
}

SOEXPORT void IBThreadOpts_setQLenPerThread(IBThreadOpts *self, int num)
{
    self->qLenPerThread = num;
}

SOEXPORT void IrcBot_daemonize(long uid, long gid,
	const char *pidfile, void (*started)(void))
{
    daemonOpts.started = started;
    daemonOpts.pidfile = pidfile;
    daemonOpts.uid = uid;
    daemonOpts.gid = gid;
    daemonOpts.daemonize = 1;
}

SOEXPORT void IrcBot_startup(int (*startup)(void))
{
    startupfunc = startup;
}

SOEXPORT void IrcBot_shutdown(void (*shutdown)(void))
{
    shutdownfunc = shutdown;
}

SOEXPORT void IrcBot_addHandler(IrcBotEventType eventType,
	const char *serverId, const char *origin, const char *filter,
	IrcBotHandler handler)
{
    IrcBotEventHandler *hdl = IB_xmalloc(sizeof *hdl);
    hdl->handler = handler;
    hdl->serverId = serverId;
    hdl->origin = origin;
    hdl->filter = filter;
    hdl->type = eventType;
    if (!handlers) handlers = IBList_create();
    IBList_append(handlers, hdl, free);
}

static inline void destroyServer(void *server)
{
    IrcServer_destroy(server);
}

SOEXPORT void IrcBot_addServer(IrcServer *server)
{
    if (!servers) servers = IBList_create();
    IBList_append(servers, server, destroyServer);
}

static int daemonrun(void *data)
{
    (void)data;

    if (!servers) return EXIT_FAILURE;

    int rc = EXIT_FAILURE;

    if (Service_init(&daemonOpts) >= 0)
    {
	Service_setTickInterval(1000);
	Event_register(Service_startup(), 0, startup, 0);
	Event_register(Service_shutdown(), 0, shutdown, 0);

	if (ThreadPool_init(&threadOpts) >= 0)
	{
	    rc = Service_run();
	    ThreadPool_done();
	}

	Service_done();
    }

    IBList_destroy(servers);
    servers = 0;
    IBList_destroy(handlers);
    handlers = 0;

    return rc;
}

SOEXPORT int IrcBot_run(void)
{
    return daemonOpts.daemonize
	? daemon_run(daemonrun, 0, daemonOpts.pidfile, 1)
	: daemonrun(0);
}

SOEXPORT IrcBotEventType IrcBotEvent_type(const IrcBotEvent *self)
{
    return self->type;
}

SOEXPORT const IrcServer *IrcBotEvent_server(const IrcBotEvent *self)
{
    return self->server;
}

SOEXPORT const IrcChannel *IrcBotEvent_channel(const IrcBotEvent *self)
{
    return IBHashTable_get(IrcServer_channels(self->server), self->origin);
}

SOEXPORT const char *IrcBotEvent_origin(const IrcBotEvent *self)
{
    return self->origin;
}

SOEXPORT const char *IrcBotEvent_command(const IrcBotEvent *self)
{
    return self->command;
}

SOEXPORT const char *IrcBotEvent_from(const IrcBotEvent *self)
{
    return self->from;
}

SOEXPORT const char *IrcBotEvent_arg(const IrcBotEvent *self)
{
    return self->arg;
}

SOEXPORT IrcBotResponse *IrcBotEvent_response(IrcBotEvent *self)
{
    return &self->response;
}

SOEXPORT void IrcBotResponse_addMsg(IrcBotResponse *self,
	const char *to, const char *msg, int action)
{
    IrcBotResponseMessage *message = IB_xmalloc(sizeof *message);
    message->to = IB_copystr(to);
    message->msg = IB_copystr(msg);
    message->action = action;
    IBList_append(self->messages, message, destroyMessage);
}

