#ifndef IRCBOT_LIST_H
#define IRCBOT_LIST_H

#include <ircbot/decl.h>

#include <stddef.h>

/** declarations for the IBList class
 * @file
 */

/** A list of objects.
 * @class IBList list.h <ircbot/list.h>
 */
C_CLASS_DECL(IBList);

/** An iterator over the contents of an IBList.
 * @class IBListIterator list.h <ircbot/list.h>
 */
C_CLASS_DECL(IBListIterator);

/** IBList default constructor.
 * Creates a new IBList
 * @memberof IBList
 * @returns a newly created IBList
 */
DECLEXPORT IBList *IBList_create(void) ATTR_RETNONNULL;

/** Number of entries.
 * @memberof IBList
 * @param self the IBList
 * @returns the number of entries
 */
DECLEXPORT size_t IBList_size(const IBList *self) CMETHOD ATTR_PURE;

/** Gets an object by position.
 * @memberof IBList
 * @param self the IBList
 * @param idx position of the object
 * @returns the object stored at that position, or NULL
 */
DECLEXPORT void *IBList_at(const IBList *self, size_t idx) CMETHOD ATTR_PURE;

/** Append an object to the list.
 * @memberof IBList
 * @param self the IBList
 * @param obj the new object
 * @param deleter optional function to destroy the object
 */
DECLEXPORT void IBList_append(IBList *self, void *obj, void (*deleter)(void *))
    CMETHOD ATTR_NONNULL((2));

/** Remove a given object from the list.
 * The object will *not* be automatically destroyed.
 * @memberof IBList
 * @param self the IBList
 * @param obj the object to remove
 */
DECLEXPORT void IBList_remove(IBList *self, void *obj)
    CMETHOD ATTR_NONNULL((2));

/** Remove matching objects from the list.
 * Objects that are removed will be destroyed if they have a deleter attached.
 * @memberof IBList
 * @param self the IBList
 * @param matcher function to compare each object to some specified value, must
 *                return 1 to have that object removed, 0 otherwise
 * @param arg some value for the matcher function to compare the objects
 *            against.
 */
DECLEXPORT void IBList_removeAll(IBList *self,
	int (*matcher)(void *, const void *), const void *arg)
    CMETHOD ATTR_NONNULL((2));

/** Creates an iterator for all entries.
 * The iterator contains a snapshot of all objects currently stored,
 * modifications to the IBList will not be reflected in the iterator. In its
 * initial state, the iterator points to an invalid position.
 * @memberof IBList
 * @param self the IBList
 * @returns an iterator
 */
DECLEXPORT IBListIterator *IBList_iterator(const IBList *self) CMETHOD;

/** IBList destructor.
 * All stored objects that have a deleter attached are destroyed as well.
 * @memberof IBList
 * @param self the IBList
 */
DECLEXPORT void IBList_destroy(IBList *self);

/** Move to the next position.
 * If the position was invalid, move to the first position. If the position was
 * pointing to the last entry, move to invalid position.
 * @memberof IBListIterator
 * @param self the IBListIterator
 * @returns 1 if the new position is a valid one, 0 otherwise
 */
DECLEXPORT int IBListIterator_moveNext(IBListIterator *self) CMETHOD;

/** Gets the object at the current position.
 * @memberof IBListIterator
 * @param self the IBListIterator
 * @returns the current object, or NULL for the invalid position
 */
DECLEXPORT void *IBListIterator_current(const IBListIterator *self)
    CMETHOD ATTR_PURE;

/** IBListIterator destructor.
 * @memberof IBListIterator
 * @param self the IBListIterator
 */
DECLEXPORT void IBListIterator_destroy(IBListIterator *self);

/** Create a List of strings by splitting a given string.
 * The string is split at any of the characters given in delim. Empty fields
 * are ignored.
 * @memberof IBList
 * @param str the string to split
 * @param delim characters that are considered delimiting fields
 * @returns a list of strings, or NULL if no non-empty fields were found
 */
DECLEXPORT IBList *IBList_fromString(const char *str, const char *delim);

#endif
