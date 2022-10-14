#include <ircbot/queue.h>

#include "util.h"

#include <stdlib.h>
#include <string.h>

#define QUEUEINITIALCAPA 8

typedef struct QueueEntry
{
    void *obj;
    void (*deleter)(void *);
} QueueEntry;

struct IBQueue
{
    QueueEntry *entries;
    size_t count;
    size_t capa;
    size_t front;
    size_t back;
};

static inline void expand(IBQueue *self)
{
    size_t newcapa = 2*self->capa;
    self->entries = IB_xrealloc(self->entries,
	    newcapa * sizeof *self->entries);
    if (self->front)
    {
	memcpy(self->entries+self->front+self->capa, self->entries+self->front,
		self->capa-self->front);
	self->front+=self->capa;
    }
    else self->back = self->capa;
    self->capa = newcapa;
}

SOEXPORT IBQueue *IBQueue_create(void)
{
    IBQueue *self = IB_xmalloc(sizeof *self);
    self->count = 0;
    self->capa = QUEUEINITIALCAPA;
    self->front = 0;
    self->back = 0;
    self->entries = IB_xmalloc(self->capa * sizeof *self->entries);
    return self;
}

SOEXPORT void IBQueue_enqueue(IBQueue *self, void *obj,
	void (*deleter)(void *))
{
    if (self->count == self->capa) expand(self);
    self->entries[self->back].obj = obj;
    self->entries[self->back].deleter = deleter;
    if (++self->back == self->capa) self->back = 0;
    ++self->count;
}

SOEXPORT void *IBQueue_dequeue(IBQueue *self)
{
    if (!self->count) return 0;
    void *obj = self->entries[self->front].obj;
    if (++self->front == self->capa) self->front = 0;
    --self->count;
    return obj;
}

SOEXPORT void IBQueue_destroy(IBQueue *self)
{
    if (!self) return;
    if (self->count) for (size_t i = 0, pos = self->front; i < self->count; ++i)
    {
	if (self->entries[pos].deleter)
	{
	    self->entries[pos].deleter(self->entries[pos].obj);
	}
	if (++pos == self->capa) pos = 0;
    }
    free(self->entries);
    free(self);
}

