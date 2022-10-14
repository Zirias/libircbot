#ifndef IRCBOT_QUEUE_H
#define IRCBOT_QUEUE_H

#include <ircbot/decl.h>

/** declarations for the IBQueue class
 * @file
 */

/** A simple queue of objects
 * @class IBQueue queue.h <ircbot/queue.h>
 */
C_CLASS_DECL(IBQueue);

/** IBQueue default constructor.
 * Creates a new IBQueue
 * @memberof IBQueue
 * @returns a newly created IBQueue
 */
DECLEXPORT IBQueue *IBQueue_create(void) ATTR_RETNONNULL;

/** Enqueue an object.
 * @memberof IBQueue
 * @param self the IBQueue
 * @param obj the object to enqueue
 * @param deleter optional function to destroy the object
 */
DECLEXPORT void IBQueue_enqueue(IBQueue *self, void *obj,
	void (*deleter)(void *)) CMETHOD ATTR_NONNULL((2));

/** Dequeue the oldest object.
 * The object will *not* be destroyed, so it can be used by the caller.
 * @memberof IBQueue
 * @param self the IBQueue
 * @returns the dequeued object, or NULL if the IBQueue was empty
 */
DECLEXPORT void *IBQueue_dequeue(IBQueue *self) CMETHOD;

/** IBQueue destructor.
 * All still queued objects that have a deleter attached are also destroyed.
 * @memberof IBQueue
 * @param self the IBQueue
 */
DECLEXPORT void IBQueue_destroy(IBQueue *self);

#endif
