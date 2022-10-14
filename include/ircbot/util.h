#ifndef IRCBOT_UTIL_H
#define IRCBOT_UTIL_H

#include <ircbot/decl.h>

#include <stddef.h>

DECLEXPORT void *IB_xmalloc(size_t size)
    ATTR_MALLOC ATTR_RETNONNULL ATTR_ALLOCSZ((1));
DECLEXPORT void *IB_xrealloc(void *ptr, size_t size)
    ATTR_RETNONNULL ATTR_ALLOCSZ((2));
DECLEXPORT char *IB_copystr(const char *src) ATTR_MALLOC;
DECLEXPORT char *IB_lowerstr(const char *src) ATTR_MALLOC;
DECLEXPORT char *IB_joinstr(const char *delim, char **strings)
    ATTR_MALLOC ATTR_NONNULL((1));

#endif
