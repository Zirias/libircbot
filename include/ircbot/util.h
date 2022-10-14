#ifndef IRCBOT_UTIL_H
#define IRCBOT_UTIL_H

#include <ircbot/decl.h>

#include <stddef.h>

/** collection of generic utility functions
 * @file
 */

/** Allocate memory.
 * This replaces malloc(), but always returns successful. On allocation error,
 * a service panic is triggered instead.
 * @param size the size of the memory to allocate
 */
DECLEXPORT void *IB_xmalloc(size_t size)
    ATTR_MALLOC ATTR_RETNONNULL ATTR_ALLOCSZ((1));

/** Reallocate memory.
 * This replaces realloc() but always returns successful. On allocation error,
 * a service panic is triggered instead.
 * @param ptr pointer to previously allocated memory
 * @param size new size for the allocated memory
 */
DECLEXPORT void *IB_xrealloc(void *ptr, size_t size)
    ATTR_RETNONNULL ATTR_ALLOCSZ((2));

/** Copy a string.
 * @param src the string to copy
 * @returns a dyanmically allocated copy of the string
 */
DECLEXPORT char *IB_copystr(const char *src) ATTR_MALLOC;

/** Convert string to lowercase.
 * @param src the original string
 * @returns a dynamically allocated copy with all characters converted to
 *          lowercase
 */
DECLEXPORT char *IB_lowerstr(const char *src) ATTR_MALLOC;

/** Join an array of strings in a single one.
 * @param delim the delimiter to place between individual strings
 * @param strings array of individual strings, terminated by a NULL pointer
 * @returns a newly allocated joined string
 */
DECLEXPORT char *IB_joinstr(const char *delim, char **strings)
    ATTR_MALLOC ATTR_NONNULL((1));

#endif
