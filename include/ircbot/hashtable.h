#ifndef IRCBOT_HASHTABLE_H
#define IRCBOT_HASHTABLE_H

#include <ircbot/decl.h>

#include <stddef.h>
#include <stdint.h>

typedef struct HashTable HashTable;
typedef struct HashTableIterator HashTableIterator;

DECLEXPORT HashTable *HashTable_create(uint8_t bits)
    ATTR_RETNONNULL; // bits: [2..8]
DECLEXPORT void HashTable_set(HashTable *self, const char *key,
	void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));
DECLEXPORT void HashTable_delete(HashTable *self, const char *key)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT size_t HashTable_count(const HashTable *self) CMETHOD ATTR_PURE;
DECLEXPORT void *HashTable_get(const HashTable *self, const char *key)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT HashTableIterator *HashTable_iterator(const HashTable *self)
    CMETHOD ATTR_RETNONNULL;
DECLEXPORT void HashTable_destroy(HashTable *self);

DECLEXPORT int HashTableIterator_moveNext(HashTableIterator *self) CMETHOD;
DECLEXPORT const char *HashTableIterator_key(const HashTableIterator *self)
    CMETHOD ATTR_PURE;
DECLEXPORT void *HashTableIterator_current(const HashTableIterator *self)
    CMETHOD ATTR_PURE;
DECLEXPORT void HashTableIterator_destroy(HashTableIterator *self);

#endif
