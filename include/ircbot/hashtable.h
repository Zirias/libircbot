#ifndef IRCBOT_HASHTABLE_H
#define IRCBOT_HASHTABLE_H

/** declarations for the IBHashTable class
 * @file
 */

#include <ircbot/decl.h>

#include <stddef.h>
#include <stdint.h>

/** A hash table storing any data objects using string keys.
 * @class IBHashTable hashtable.h <ircbot/hashtable.h>
 */
C_CLASS_DECL(IBHashTable);

/** An iterator over the contents of an IBHashTable.
 * @class IBHashTableIterator hashtable.h <ircbot/hashtable.h>
 */
C_CLASS_DECL(IBHashTableIterator);

/** IBHashTable default constructor.
 * Creates a new IBHashTable
 * @memberof IBHashTable
 * @param bits number of bits for the hashes (valid range [2..8])
 * @returns a newly created IBHashTable
 */
DECLEXPORT IBHashTable *IBHashTable_create(uint8_t bits)
    ATTR_RETNONNULL;

/** Set a new object for a key.
 * If there was already an object for the given key, it is replaced. The old
 * object is destroyed if it has a deleter attached.
 * @memberof IBHashTable
 * @param self the IBHashTable
 * @param key the key
 * @param obj the new object
 * @param deleter optional function to destroy the object
 */
DECLEXPORT void IBHashTable_set(IBHashTable *self, const char *key,
	void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));

/** Deletes the object with the specified key.
 * If the object has a deleter attached, it is also destroyed.
 * @memberof IBHashTable
 * @param self the IBHashTable
 * @param key the key
 * @returns 1 if an object was deleted, 0 otherwise
 */
DECLEXPORT int IBHashTable_delete(IBHashTable *self, const char *key)
    CMETHOD ATTR_NONNULL((2));

/** Number of entries.
 * @memberof IBHashTable
 * @param self the IBHashTable
 * @returns the number of entries
 */
DECLEXPORT size_t IBHashTable_count(const IBHashTable *self) CMETHOD ATTR_PURE;

/** Gets an object by key.
 * @memberof IBHashTable
 * @param self the IBHashTable
 * @param key the key
 * @returns the object stored for the give key, or NULL
 */
DECLEXPORT void *IBHashTable_get(const IBHashTable *self, const char *key)
    CMETHOD ATTR_NONNULL((2));

/** Creates an iterator for all entries.
 * The iterator contains a snapshot of all objects currently stored,
 * modifications to the IBHashTable will not be reflected in the iterator.
 * In its initial state, the iterator points to an invalid position.
 * @memberof IBHashTable
 * @param self the IBHashTable
 * @returns an iterator
 */
DECLEXPORT IBHashTableIterator *IBHashTable_iterator(const IBHashTable *self)
    CMETHOD ATTR_RETNONNULL;

/** IBHashTable destructor.
 * All stored objects that have a deleter attached are destroyed as well.
 * @memberof IBHashTable
 * @param self the IBHashTable
 */
DECLEXPORT void IBHashTable_destroy(IBHashTable *self);

/** Move to the next position.
 * If the position was invalid, move to the first position. If the position
 * was pointing to the last entry, move to invalid position.
 * @memberof IBHashTableIterator
 * @param self the IBHashTableIterator
 * @returns 1 if the new position is a valid one, 0 otherwise
 */
DECLEXPORT int IBHashTableIterator_moveNext(IBHashTableIterator *self) CMETHOD;

/** Gets the key at the current position.
 * @memberof IBHashTableIterator
 * @param self the IBHashTableIterator
 * @returns the current key, or NULL for the invalid position
 */
DECLEXPORT const char *IBHashTableIterator_key(const IBHashTableIterator *self)
    CMETHOD ATTR_PURE;

/** Gets the object at the current position.
 * @memberof IBHashTableIterator
 * @param self the IBHashTableIterator
 * @returns the current object, or NULL for the invalid position
 */
DECLEXPORT void *IBHashTableIterator_current(const IBHashTableIterator *self)
    CMETHOD ATTR_PURE;

/** IBHashTableIterator destructor.
 * @memberof IBHashTableIterator
 * @param self the IBHashTableIterator
 */
DECLEXPORT void IBHashTableIterator_destroy(IBHashTableIterator *self);

#endif
