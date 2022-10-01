#include "hashtable.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct HashTableEntry HashTableEntry;
struct HashTableEntry
{
    HashTableEntry *next;
    char *key;
    void *obj;
    void (*deleter)(void *);
};

struct HashTable
{
    size_t count;
    uint8_t bits;
    HashTableEntry *bucket[];
};

typedef struct IteratorEntry
{
    const char *key;
    void *obj;
} IteratorEntry;

struct HashTableIterator
{
    size_t count;
    size_t pos;
    IteratorEntry entries[];
};

SOLOCAL HashTable *HashTable_create(uint8_t bits)
{
    size_t htsize = HT_SIZE(bits);
    HashTable *self = xmalloc(sizeof *self + htsize * sizeof *self->bucket);
    memset(self, 0, sizeof *self + htsize * sizeof *self->bucket);
    self->bits = bits;
    return self;
}

SOLOCAL void HashTable_set(HashTable *self, const char *key,
	void *obj, void (*deleter)(void *))
{
    uint8_t h = hashstr(key, self->bits);
    HashTableEntry *parent = 0;
    HashTableEntry *entry = self->bucket[h];
    while (entry)
    {
	if (!strcmp(entry->key, key)) break;
	parent = entry;
	entry = parent->next;
    }
    if (entry)
    {
	if (entry->deleter) entry->deleter(entry->obj);
	entry->obj = obj;
	entry->deleter = deleter;
    }
    else
    {
	entry = xmalloc(sizeof *entry);
	entry->next = 0;
	entry->key = copystr(key);
	entry->obj = obj;
	entry->deleter = deleter;
	if (parent) parent->next = entry;
	else self->bucket[h] = entry;
	++self->count;
    }
}

SOLOCAL void HashTable_delete(HashTable *self, const char *key)
{
    uint8_t h = hashstr(key, self->bits);
    HashTableEntry *parent = 0;
    HashTableEntry *entry = self->bucket[h];
    while (entry)
    {
	if (!strcmp(entry->key, key)) break;
	parent = entry;
	entry = entry->next;
    }
    if (entry)
    {
	if (entry->deleter) entry->deleter(entry->obj);
	if (parent) parent->next = entry->next;
	else self->bucket[h] = entry->next;
	free(entry->key);
	free(entry);
	--self->count;
    }
}

SOLOCAL size_t HashTable_count(const HashTable *self)
{
    return self->count;
}

SOLOCAL void *HashTable_get(const HashTable *self, const char *key)
{
    HashTableEntry *entry = self->bucket[hashstr(key, self->bits)];
    while (entry)
    {
	if (!strcmp(entry->key, key)) return entry->obj;
	entry = entry->next;
    }
    return 0;
}

SOLOCAL HashTableIterator *HashTable_iterator(const HashTable *self)
{
    HashTableIterator *iter = xmalloc(
	    sizeof *iter + self->count * sizeof *iter->entries);
    iter->count = self->count;
    iter->pos = self->count;
    size_t pos = 0;
    for (unsigned h = 0; h < HT_SIZE(self->bits); ++h)
    {
	HashTableEntry *entry = self->bucket[h];
	while (entry)
	{
	    iter->entries[pos].key = entry->key;
	    iter->entries[pos].obj = entry->obj;
	    ++pos;
	    entry = entry->next;
	}
    }
    return iter;
}

SOLOCAL void HashTable_destroy(HashTable *self)
{
    if (!self) return;
    for (unsigned h = 0; h < HT_SIZE(self->bits); ++h)
    {
	HashTableEntry *entry = self->bucket[h];
	HashTableEntry *next;
	while (entry)
	{
	    next = entry->next;
	    if (entry->deleter) entry->deleter(entry->obj);
	    free(entry->key);
	    free(entry);
	    entry = next;
	}
    }
    free(self);
}

SOLOCAL int HashTableIterator_moveNext(HashTableIterator *self)
{
    if (self->pos >= self->count) self->pos = 0;
    else ++self->pos;
    return self->pos < self->count;
}

SOLOCAL const char *HashTableIterator_key(const HashTableIterator *self)
{
    if (self->pos >= self->count) return 0;
    return self->entries[self->pos].key;
}

SOLOCAL void *HashTableIterator_current(const HashTableIterator *self)
{
    if (self->pos >= self->count) return 0;
    return self->entries[self->pos].obj;
}

SOLOCAL void HashTableIterator_destroy(HashTableIterator *self)
{
    free(self);
}

