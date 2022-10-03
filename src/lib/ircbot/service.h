#ifndef IRCBOT_INT_SERVICE_H
#define IRCBOT_INT_SERVICE_H

#include <ircbot/decl.h>
#include <ircbot/service.h>

typedef struct DaemonOpts DaemonOpts;

int Service_init(const DaemonOpts *options) ATTR_NONNULL((1));
Event *Service_readyRead(void) ATTR_RETNONNULL ATTR_PURE;
Event *Service_readyWrite(void) ATTR_RETNONNULL ATTR_PURE;
void Service_registerRead(int id);
void Service_unregisterRead(int id);
void Service_registerWrite(int id);
void Service_unregisterWrite(int id);
int Service_setTickInterval(unsigned msec);
int Service_run(void);
void Service_quit(void);
void Service_shutdownLock(void);
void Service_shutdownUnlock(void);
void Service_panic(const char *msg) ATTR_NONNULL((1)) ATTR_NORETURN;
void Service_done(void);

#endif
