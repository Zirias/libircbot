#ifndef IRCBOT_INT_QUEUE_H
#define IRCBOT_INT_QUEUE_H

#include <ircbot/decl.h>

typedef struct Queue Queue;

Queue *Queue_create(void) ATTR_RETNONNULL;
void Queue_enqueue(Queue *self, void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2));
void *Queue_dequeue(Queue *self) CMETHOD;
void Queue_destroy(Queue *self);

#endif
