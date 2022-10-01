#ifndef IRCBOT_QUEUE_H
#define IRCBOT_QUEUE_H

#include <ircbot/decl.h>

typedef struct Queue Queue;

DECLEXPORT Queue *Queue_create(void) ATTR_RETNONNULL;
DECLEXPORT void Queue_enqueue(Queue *self, void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT void *Queue_dequeue(Queue *self) CMETHOD;
DECLEXPORT void Queue_destroy(Queue *self);

#endif
