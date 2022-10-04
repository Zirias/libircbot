#ifndef IRCBOT_LIST_H
#define IRCBOT_LIST_H

#include <ircbot/decl.h>

#include <stddef.h>

typedef struct List List;
typedef struct ListIterator ListIterator;

DECLEXPORT List *List_create(void) ATTR_RETNONNULL;
DECLEXPORT size_t List_size(const List *self) CMETHOD ATTR_PURE;
DECLEXPORT void *List_at(const List *self, size_t idx) CMETHOD ATTR_PURE;
DECLEXPORT void List_append(List *self, void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT void List_remove(List *self, void *obj)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT void List_removeAll(List *self,
	int (*matcher)(void *, const void *), const void *arg)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT ListIterator *List_iterator(const List *self) CMETHOD;
DECLEXPORT void List_destroy(List *self);

DECLEXPORT int ListIterator_moveNext(ListIterator *self) CMETHOD;
DECLEXPORT void *ListIterator_current(const ListIterator *self)
    CMETHOD ATTR_PURE;
DECLEXPORT void ListIterator_destroy(ListIterator *self);

DECLEXPORT List *List_fromString(const char *str, const char *delim);

#endif
