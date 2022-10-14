#ifndef IRCBOT_HASHTABLE_H
#define IRCBOT_HASHTABLE_H

#include <ircbot/decl.h>

#include <stddef.h>
#include <stdint.h>

C_CLASS_DECL(IBHashTable);
C_CLASS_DECL(IBHashTableIterator);

DECLEXPORT IBHashTable *IBHashTable_create(uint8_t bits)
    ATTR_RETNONNULL; // bits: [2..8]
DECLEXPORT void IBHashTable_set(IBHashTable *self, const char *key,
	void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));
DECLEXPORT int IBHashTable_delete(IBHashTable *self, const char *key)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT size_t IBHashTable_count(const IBHashTable *self) CMETHOD ATTR_PURE;
DECLEXPORT void *IBHashTable_get(const IBHashTable *self, const char *key)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT IBHashTableIterator *IBHashTable_iterator(const IBHashTable *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT void IBHashTable_destroy(IBHashTable *self);

DECLEXPORT int IBHashTableIterator_moveNext(IBHashTableIterator *self) CMETHOD;
DECLEXPORT const char *IBHashTableIterator_key(const IBHashTableIterator *self)
    CMETHOD ATTR_PURE;
DECLEXPORT void *IBHashTableIterator_current(const IBHashTableIterator *self)
    CMETHOD ATTR_PURE;
DECLEXPORT void IBHashTableIterator_destroy(IBHashTableIterator *self);

#endif
