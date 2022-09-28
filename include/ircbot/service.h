#ifndef IRCBOT_SERVICE_H
#define IRCBOT_SERVICE_H

#include <ircbot/decl.h>

#define MAXPANICHANDLERS 8

C_CLASS_DECL(Event);

typedef struct StartupEventArgs
{
    int rc;
} StartupEventArgs;

typedef void (*PanicHandler)(const char *msg) ATTR_NONNULL((1));

DECLEXPORT Event *Service_startup(void) ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT Event *Service_shutdown(void) ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT Event *Service_tick(void) ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT Event *Service_eventsDone(void) ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT void Service_registerPanic(PanicHandler handler) ATTR_NONNULL((1));
DECLEXPORT void Service_unregisterPanic(PanicHandler handler) ATTR_NONNULL((1));

#endif
