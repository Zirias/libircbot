#ifndef IRCBOT_IRCBOT_H
#define IRCBOT_IRCBOT_H

#include <ircbot/decl.h>

C_CLASS_DECL(IrcServer);
C_CLASS_DECL(ThreadOpts);

DECLEXPORT ThreadOpts *IrcBot_threadOpts(void) ATTR_RETNONNULL;
DECLEXPORT void ThreadOpts_setNThreads(ThreadOpts *self, int num) CMETHOD;
DECLEXPORT void ThreadOpts_setMaxThreads(ThreadOpts *self, int num) CMETHOD;
DECLEXPORT void ThreadOpts_setNPerCpu(ThreadOpts *self, int num) CMETHOD;
DECLEXPORT void ThreadOpts_setDefNThreads(ThreadOpts *self, int num) CMETHOD;
DECLEXPORT void ThreadOpts_setQueueLen(ThreadOpts *self, int num) CMETHOD;
DECLEXPORT void ThreadOpts_setMaxQueueLen(ThreadOpts *self, int num) CMETHOD;
DECLEXPORT void ThreadOpts_setMinQueueLen(ThreadOpts *self, int num) CMETHOD;
DECLEXPORT void ThreadOpts_setQLenPerThread(ThreadOpts *self, int num) CMETHOD;

DECLEXPORT void IrcBot_daemonize(long uid, long gid,
	const char *pidfile, void (*started)(void));

DECLEXPORT void IrcBot_addServer(IrcServer *server) ATTR_NONNULL((1));

DECLEXPORT int IrcBot_run(void);

#endif
