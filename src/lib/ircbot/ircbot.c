#include <ircbot/event.h>
#include <ircbot/ircserver.h>
#include <ircbot/list.h>

#include "ircbot.h"
#include "service.h"
#include "util.h"

#include <stdlib.h>

static ThreadOpts threadOpts = {
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

static List *servers = 0;

static void startup(void *receiver, void *sender, void *args)
{
    (void)sender;

    IrcServer *server = receiver;
    StartupEventArgs *ea = args;

    if (IrcServer_connect(server) < 0) ea->rc = EXIT_FAILURE;
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
    (void)sender;
    (void)args;

    IrcServer *server = receiver;
    Event_register(IrcServer_disconnected(server), 0, shutdownok, 0);
    Service_shutdownLock();
    IrcServer_disconnect(server);
}

SOEXPORT ThreadOpts *IrcBot_threadOpts(void)
{
    return &threadOpts;
}

SOEXPORT void ThreadOpts_setNThreads(ThreadOpts *self, int num)
{
    self->nThreads = num;
}

SOEXPORT void ThreadOpts_setMaxThreads(ThreadOpts *self, int num)
{
    self->maxThreads = num;
}

SOEXPORT void ThreadOpts_setNPerCpu(ThreadOpts *self, int num)
{
    self->nPerCpu = num;
}

SOEXPORT void ThreadOpts_setDefNThreads(ThreadOpts *self, int num)
{
    self->defNThreads = num;
}

SOEXPORT void ThreadOpts_setQueueLen(ThreadOpts *self, int num)
{
    self->queueLen = num;
}

SOEXPORT void ThreadOpts_setMaxQueueLen(ThreadOpts *self, int num)
{
    self->maxQueueLen = num;
}

SOEXPORT void ThreadOpts_setMinQueueLen(ThreadOpts *self, int num)
{
    self->minQueueLen = num;
}

SOEXPORT void ThreadOpts_setQLenPerThread(ThreadOpts *self, int num)
{
    self->qLenPerThread = num;
}

SOEXPORT void IrcBot_daemonize(long uid, long gid,
	const char *pidfile, void (*started)(void))
{
    daemonOpts.started = started;
    free(daemonOpts.pidfile);
    daemonOpts.pidfile = copystr(pidfile);
    daemonOpts.uid = uid;
    daemonOpts.gid = gid;
    daemonOpts.daemonize = 1;
}

static void destroyServer(void *server)
{
    IrcServer_destroy(server);
}

SOEXPORT void IrcBot_addServer(IrcServer *server)
{
    if (!servers) servers = List_create();
    List_append(servers, server, destroyServer);
}

SOEXPORT int IrcBot_run(void)
{
    if (!servers || !List_size(servers)) return EXIT_FAILURE;

    Service_init(&daemonOpts);
    Service_setTickInterval(1000);

    ListIterator *s = List_iterator(servers);
    while (ListIterator_moveNext(s))
    {
	IrcServer *server = ListIterator_current(s);
	Event_register(Service_startup(), server, startup, 0);
	Event_register(Service_shutdown(), server, shutdown, 0);
    }
    ListIterator_destroy(s);

    int rc = Service_run();
    Service_done();

    List_destroy(servers);
    servers = 0;

    return rc;
}
