#include <ircbot/list.h>

#include "util.h"

#include <stdlib.h>
#include <string.h>

#define INITIALCAPA 8

typedef struct ListItem
{
    void *obj;
    void (*deleter)(void *);
} ListItem;

struct List
{
    ListItem *items;
    size_t capa;
    size_t count;
};

struct ListIterator
{
    size_t count;
    size_t pos;
    ListItem items[];
};

SOEXPORT List *List_create(void)
{
    List *self = IB_xmalloc(sizeof *self);
    self->capa = INITIALCAPA;
    self->count = 0;
    self->items = IB_xmalloc(self->capa * sizeof *self->items);
    return self;
}

SOEXPORT size_t List_size(const List *self)
{
    return self->count;
}

SOEXPORT void *List_at(const List *self, size_t idx)
{
    if (idx >= self->count) return 0;
    return self->items[idx].obj;
}

SOEXPORT void List_append(List *self, void *obj, void (*deleter)(void *))
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

static void removeAt(List *self, size_t i, int delete)
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

SOEXPORT void List_remove(List *self, void *obj)
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

SOEXPORT void List_removeAll(List *self,
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

SOEXPORT ListIterator *List_iterator(const List *self)
{
    ListIterator *iter = IB_xmalloc(sizeof *iter +
	    self->count * sizeof *self->items);
    iter->count = self->count;
    iter->pos = self->count;
    memcpy(iter->items, self->items, self->count * sizeof *self->items);
    return iter;
}

SOEXPORT void List_destroy(List *self)
{
    if (!self) return;
    for (size_t i = 0; i < self->count; ++i)
    {
	if (self->items[i].deleter) self->items[i].deleter(self->items[i].obj);
    }
    free(self->items);
    free(self);
}

SOEXPORT int ListIterator_moveNext(ListIterator *self)
{
    if (self->pos >= self->count) self->pos = 0;
    else ++self->pos;
    return self->pos < self->count;
}

SOEXPORT void *ListIterator_current(const ListIterator *self)
{
    if (self->pos >= self->count) return 0;
    return self->items[self->pos].obj;
}

SOEXPORT void ListIterator_destroy(ListIterator *self)
{
    free(self);
}

SOEXPORT List *List_fromString(const char *str, const char *delim)
{
    List *list = 0;
    char *buf = IB_copystr(str);
    char *bufp = buf;
    char *word = 0;

    while ((word = strsep(&bufp, delim)))
    {
	if (*word)
	{
	    if (!list) list = List_create();
	    List_append(list, IB_copystr(word), free);
	}
    }

    free(buf);
    return list;
}
