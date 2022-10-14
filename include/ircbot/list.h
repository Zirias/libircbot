#ifndef IRCBOT_LIST_H
#define IRCBOT_LIST_H

#include <ircbot/decl.h>

#include <stddef.h>

C_CLASS_DECL(IBList);
C_CLASS_DECL(IBListIterator);

DECLEXPORT IBList *IBList_create(void) ATTR_RETNONNULL;
DECLEXPORT size_t IBList_size(const IBList *self) CMETHOD ATTR_PURE;
DECLEXPORT void *IBList_at(const IBList *self, size_t idx) CMETHOD ATTR_PURE;
DECLEXPORT void IBList_append(IBList *self, void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT void IBList_remove(IBList *self, void *obj)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT void IBList_removeAll(IBList *self,
	int (*matcher)(void *, const void *), const void *arg)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT IBListIterator *IBList_iterator(const IBList *self) CMETHOD;
DECLEXPORT void IBList_destroy(IBList *self);

DECLEXPORT int IBListIterator_moveNext(IBListIterator *self) CMETHOD;
DECLEXPORT void *IBListIterator_current(const IBListIterator *self)
    CMETHOD ATTR_PURE;
DECLEXPORT void IBListIterator_destroy(IBListIterator *self);

DECLEXPORT IBList *IBList_fromString(const char *str, const char *delim);

#endif
