#include <ircbot/hashtable.h>

#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct IBHashTableEntry IBHashTableEntry;
struct IBHashTableEntry
{
    IBHashTableEntry *next;
    char *key;
    void *obj;
    void (*deleter)(void *);
};

struct IBHashTable
{
    size_t count;
    uint8_t bits;
    IBHashTableEntry *bucket[];
};

typedef struct IteratorEntry
{
    const char *key;
    void *obj;
} IteratorEntry;

struct IBHashTableIterator
{
    size_t count;
    size_t pos;
    IteratorEntry entries[];
};

SOEXPORT IBHashTable *IBHashTable_create(uint8_t bits)
{
    size_t htsize = HT_SIZE(bits);
    IBHashTable *self = IB_xmalloc(sizeof *self + htsize * sizeof *self->bucket);
    memset(self, 0, sizeof *self + htsize * sizeof *self->bucket);
    self->bits = bits;
    return self;
}

SOEXPORT void IBHashTable_set(IBHashTable *self, const char *key,
	void *obj, void (*deleter)(void *))
{
    uint8_t h = hashstr(key, self->bits);
    IBHashTableEntry *parent = 0;
    IBHashTableEntry *entry = self->bucket[h];
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
	entry = IB_xmalloc(sizeof *entry);
	entry->next = 0;
	entry->key = IB_copystr(key);
	entry->obj = obj;
	entry->deleter = deleter;
	if (parent) parent->next = entry;
	else self->bucket[h] = entry;
	++self->count;
    }
}

SOEXPORT int IBHashTable_delete(IBHashTable *self, const char *key)
{
    uint8_t h = hashstr(key, self->bits);
    IBHashTableEntry *parent = 0;
    IBHashTableEntry *entry = self->bucket[h];
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
	return 1;
    }
    return 0;
}

SOEXPORT size_t IBHashTable_count(const IBHashTable *self)
{
    return self->count;
}

SOEXPORT void *IBHashTable_get(const IBHashTable *self, const char *key)
{
    IBHashTableEntry *entry = self->bucket[hashstr(key, self->bits)];
    while (entry)
    {
	if (!strcmp(entry->key, key)) return entry->obj;
	entry = entry->next;
    }
    return 0;
}

SOEXPORT IBHashTableIterator *IBHashTable_iterator(const IBHashTable *self)
{
    IBHashTableIterator *iter = IB_xmalloc(
	    sizeof *iter + self->count * sizeof *iter->entries);
    iter->count = self->count;
    iter->pos = self->count;
    size_t pos = 0;
    for (unsigned h = 0; h < HT_SIZE(self->bits); ++h)
    {
	IBHashTableEntry *entry = self->bucket[h];
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

SOEXPORT void IBHashTable_destroy(IBHashTable *self)
{
    if (!self) return;
    for (unsigned h = 0; h < HT_SIZE(self->bits); ++h)
    {
	IBHashTableEntry *entry = self->bucket[h];
	IBHashTableEntry *next;
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

SOEXPORT int IBHashTableIterator_moveNext(IBHashTableIterator *self)
{
    if (self->pos >= self->count) self->pos = 0;
    else ++self->pos;
    return self->pos < self->count;
}

SOEXPORT const char *IBHashTableIterator_key(const IBHashTableIterator *self)
{
    if (self->pos >= self->count) return 0;
    return self->entries[self->pos].key;
}

SOEXPORT void *IBHashTableIterator_current(const IBHashTableIterator *self)
{
    if (self->pos >= self->count) return 0;
    return self->entries[self->pos].obj;
}

SOEXPORT void IBHashTableIterator_destroy(IBHashTableIterator *self)
{
    free(self);
}

