#include "list.h"
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

SOLOCAL List *List_create(void)
{
    List *self = xmalloc(sizeof *self);
    self->capa = INITIALCAPA;
    self->count = 0;
    self->items = xmalloc(self->capa * sizeof *self->items);
    return self;
}

SOLOCAL size_t List_size(const List *self)
{
    return self->count;
}

SOLOCAL void List_append(List *self, void *obj, void (*deleter)(void *))
{
    if (self->count == self->capa)
    {
	self->capa *= 2;
	self->items = xrealloc(self->items, self->capa * sizeof *self->items);
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

SOLOCAL void List_remove(List *self, void *obj)
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

SOLOCAL void List_removeAll(List *self,
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

SOLOCAL ListIterator *List_iterator(const List *self)
{
    ListIterator *iter = xmalloc(sizeof *iter +
	    self->count * sizeof *self->items);
    iter->count = self->count;
    iter->pos = self->count;
    memcpy(iter->items, self->items, self->count * sizeof *self->items);
    return iter;
}

SOLOCAL void List_destroy(List *self)
{
    if (!self) return;
    for (size_t i = 0; i < self->count; ++i)
    {
	if (self->items[i].deleter) self->items[i].deleter(self->items[i].obj);
    }
    free(self->items);
    free(self);
}

SOLOCAL int ListIterator_moveNext(ListIterator *self)
{
    if (self->pos >= self->count) self->pos = 0;
    else ++self->pos;
    return self->pos < self->count;
}

SOLOCAL void *ListIterator_current(const ListIterator *self)
{
    if (self->pos >= self->count) return 0;
    return self->items[self->pos].obj;
}

SOLOCAL void ListIterator_destroy(ListIterator *self)
{
    free(self);
}

