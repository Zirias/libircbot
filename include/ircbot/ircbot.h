#ifndef IRCBOT_IRCBOT_H
#define IRCBOT_IRCBOT_H

#include <ircbot/decl.h>

C_CLASS_DECL(IrcBotEvent);
C_CLASS_DECL(IrcBotResponse);
C_CLASS_DECL(IrcChannel);
C_CLASS_DECL(IrcServer);
C_CLASS_DECL(IBThreadOpts);

typedef enum IrcBotEventType
{
    IBET_BOTCOMMAND,
    IBET_PRIVMSG,
    IBET_CONNECTED,
    IBET_CHANJOINED,
    IBET_JOINED,
    IBET_PARTED
} IrcBotEventType;

typedef void (*IrcBotHandler)(IrcBotEvent *event) ATTR_NONNULL((1));

#define ORIGIN_PRIVATE ":"
#define ORIGIN_CHANNEL "#"

DECLEXPORT IBThreadOpts *IrcBot_threadOpts(void) ATTR_RETNONNULL;
DECLEXPORT void IBThreadOpts_setNThreads(IBThreadOpts *self, int num)
    CMETHOD;
DECLEXPORT void IBThreadOpts_setMaxThreads(IBThreadOpts *self, int num)
    CMETHOD;
DECLEXPORT void IBThreadOpts_setNPerCpu(IBThreadOpts *self, int num)
    CMETHOD;
DECLEXPORT void IBThreadOpts_setDefNThreads(IBThreadOpts *self, int num)
    CMETHOD;
DECLEXPORT void IBThreadOpts_setQueueLen(IBThreadOpts *self, int num)
    CMETHOD;
DECLEXPORT void IBThreadOpts_setMaxQueueLen(IBThreadOpts *self, int num)
    CMETHOD;
DECLEXPORT void IBThreadOpts_setMinQueueLen(IBThreadOpts *self, int num)
    CMETHOD;
DECLEXPORT void IBThreadOpts_setQLenPerThread(IBThreadOpts *self, int num)
    CMETHOD;

DECLEXPORT void IrcBot_daemonize(long uid, long gid,
	const char *pidfile, void (*started)(void));
DECLEXPORT void IrcBot_startup(int (*startup)(void));
DECLEXPORT void IrcBot_shutdown(void (*shutdown)(void));

DECLEXPORT void IrcBot_addHandler(IrcBotEventType eventType,
	const char *serverId, const char *origin, const char *filter,
	IrcBotHandler handler);

DECLEXPORT void IrcBot_addServer(IrcServer *server) ATTR_NONNULL((1));

DECLEXPORT int IrcBot_run(void);

DECLEXPORT IrcBotEventType IrcBotEvent_type(const IrcBotEvent *self) CMETHOD;
DECLEXPORT const IrcServer *IrcBotEvent_server(const IrcBotEvent *self)
    CMETHOD;
DECLEXPORT const IrcChannel *IrcBotEvent_channel(const IrcBotEvent *self)
    CMETHOD;
DECLEXPORT const char *IrcBotEvent_origin(const IrcBotEvent *self) CMETHOD;
DECLEXPORT const char *IrcBotEvent_command(const IrcBotEvent *self) CMETHOD;
DECLEXPORT const char *IrcBotEvent_from(const IrcBotEvent *self) CMETHOD;
DECLEXPORT const char *IrcBotEvent_arg(const IrcBotEvent *self) CMETHOD;
DECLEXPORT IrcBotResponse *IrcBotEvent_response(IrcBotEvent *self)
    CMETHOD ATTR_RETNONNULL;

DECLEXPORT void IrcBotResponse_addMsg(IrcBotResponse *self,
	const char *to, const char *msg, int action)
    CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));

#endif
