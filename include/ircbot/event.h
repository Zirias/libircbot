#ifndef IRCBOT_EVENT_H
#define IRCBOT_EVENT_H

#include <ircbot/decl.h>

typedef void (*EventHandler)(void *receiver, void *sender, void *args);

C_CLASS_DECL(Event);

DECLEXPORT Event *Event_create(void *sender) ATTR_RETNONNULL;
DECLEXPORT void Event_register(Event *self, void *receiver,
	EventHandler handler, int id) CMETHOD ATTR_NONNULL((3));
DECLEXPORT void Event_unregister(Event *self, void *receiver,
	EventHandler handler, int id) CMETHOD ATTR_NONNULL((3));
DECLEXPORT void Event_raise(Event *self, int id, void *args) CMETHOD;
DECLEXPORT void Event_destroy(Event *self);

#endif
