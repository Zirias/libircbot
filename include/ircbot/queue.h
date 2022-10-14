#ifndef IRCBOT_QUEUE_H
#define IRCBOT_QUEUE_H

#include <ircbot/decl.h>

C_CLASS_DECL(IBQueue);

DECLEXPORT IBQueue *IBQueue_create(void) ATTR_RETNONNULL;
DECLEXPORT void IBQueue_enqueue(IBQueue *self, void *obj,
	void (*deleter)(void *)) CMETHOD ATTR_NONNULL((2));
DECLEXPORT void *IBQueue_dequeue(IBQueue *self) CMETHOD;
DECLEXPORT void IBQueue_destroy(IBQueue *self);

#endif
