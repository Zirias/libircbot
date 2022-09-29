#include <ircbot/config.h>
#include <ircbot/event.h>
#include <ircbot/ircbot.h>
#include <ircbot/ircserver.h>

#include "service.h"

static void startup(void *receiver, void *sender, void *args)
{
    (void)sender;
    (void)args;

    IrcServer *server = receiver;
    IrcServer_connect(server);
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

SOEXPORT int IrcBot_run(Config *config, IrcServer *server)
{
    Service_init(config);
    Event_register(Service_startup(), server, startup, 0);
    Event_register(Service_shutdown(), server, shutdown, 0);

    int rc = Service_run();
    Service_done();
    return rc;
}