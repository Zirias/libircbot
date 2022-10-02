#ifndef IRCBOT_INT_DAEMON_H
#define IRCBOT_INT_DAEMON_H

#include <ircbot/decl.h>

typedef int (*daemon_main)(void *data);

int daemon_run(const daemon_main dmain, void *data,
	const char *pidfile, int waitLaunched) ATTR_NONNULL((1));
void daemon_launched(void);

#endif
