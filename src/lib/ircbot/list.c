#include <ircbot/list.h>

#include "util.h"

#include <stdlib.h>
#include <string.h>

#define INITIALCAPA 8

typedef struct IBListItem
{
    void *obj;
    void (*deleter)(void *);
} IBListItem;

struct IBList
{
    IBListItem *items;
    size_t capa;
    size_t count;
};

struct IBListIterator
{
    size_t count;
    size_t pos;
    IBListItem items[];
};

SOEXPORT IBList *IBList_create(void)
{
    IBList *self = IB_xmalloc(sizeof *self);
    self->capa = INITIALCAPA;
    self->count = 0;
    self->items = IB_xmalloc(self->capa * sizeof *self->items);
    return self;
}

SOEXPORT size_t IBList_size(const IBList *self)
{
    return self->count;
}

SOEXPORT void *IBList_at(const IBList *self, size_t idx)
{
    if (idx >= self->count) return 0;
    return self->items[idx].obj;
}

SOEXPORT void IBList_append(IBList *self, void *obj, void (*deleter)(void *))
{
    if (self->count == self->capa)
    {
	self->capa *= 2;
	self->items = IB_xrealloc(self->items,
		self->capa * sizeof *self->items);
    }
    self->items[self->count].obj = obj;
    self->items[self->count].deleter = deleter;
    ++self->count;
}

static void removeAt(IBList *self, size_t i, int delete)
{
    if (delete && self->items[i].deleter)
    {
	self->items[i].deleter(self->items[i].obj);
    }
    if (i < self->count - 1)
    {
	memmove(self->items+i, self->items+i+1,
		(self->count-i-1) * sizeof *self->items);
    }
    --self->count;
}

SOEXPORT void IBList_remove(IBList *self, void *obj)
{
    for (size_t i = 0; i < self->count; ++i)
    {
	if (self->items[i].obj == obj)
	{
	    removeAt(self, i, 0);
	    break;
	}
    }
}

SOEXPORT void IBList_removeAll(IBList *self,
	int (*matcher)(void *, const void *), const void *arg)
{
    for (size_t i = 0; i < self->count; ++i)
    {
	if (matcher(self->items[i].obj, arg))
	{
	    removeAt(self, i, 1);
	    --i;
	}
    }
}

SOEXPORT IBListIterator *IBList_iterator(const IBList *self)
{
    IBListIterator *iter = IB_xmalloc(sizeof *iter +
	    self->count * sizeof *self->items);
    iter->count = self->count;
    iter->pos = self->count;
    memcpy(iter->items, self->items, self->count * sizeof *self->items);
    return iter;
}

SOEXPORT void IBList_destroy(IBList *self)
{
    if (!self) return;
    for (size_t i = 0; i < self->count; ++i)
    {
	if (self->items[i].deleter) self->items[i].deleter(self->items[i].obj);
    }
    free(self->items);
    free(self);
}

SOEXPORT int IBListIterator_moveNext(IBListIterator *self)
{
    if (self->pos >= self->count) self->pos = 0;
    else ++self->pos;
    return self->pos < self->count;
}

SOEXPORT void *IBListIterator_current(const IBListIterator *self)
{
    if (self->pos >= self->count) return 0;
    return self->items[self->pos].obj;
}

SOEXPORT void IBListIterator_destroy(IBListIterator *self)
{
    free(self);
}

SOEXPORT IBList *IBList_fromString(const char *str, const char *delim)
{
    IBList *list = 0;
    char *buf = IB_copystr(str);
    char *bufp = buf;
    char *word = 0;

    while ((word = strsep(&bufp, delim)))
    {
	if (*word)
	{
	    if (!list) list = IBList_create();
	    IBList_append(list, IB_copystr(word), free);
	}
    }

    free(buf);
    return list;
}
