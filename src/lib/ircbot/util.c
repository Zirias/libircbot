#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "service.h"
#include "util.h"

SOLOCAL void *xmalloc(size_t size)
{
    void *m = malloc(size);
    if (!m) Service_panic("memory allocation failed.");
    return m;
}

SOLOCAL void *xrealloc(void *ptr, size_t size)
{
    void *m = realloc(ptr, size);
    if (!m) Service_panic("memory allocation failed.");
    return m;
}

SOLOCAL char *copystr(const char *src)
{
    if (!src) return 0;
    char *copy = xmalloc(strlen(src) + 1);
    strcpy(copy, src);
    return copy;
}

SOLOCAL char *lowerstr(const char *src)
{
    char *lower = copystr(src);
    char *p = lower;
    if (p) while (*p)
    {
	*p = tolower(*p);
	++p;
    }
    return lower;
}

SOLOCAL char *joinstr(const char *delim, char **strings)
{
    int n = 0;
    size_t rlen = 0;
    size_t dlen = strlen(delim);
    char **cur;
    for (cur = strings; *cur; ++cur)
    {
	++n;
	rlen += strlen(*cur);
    }
    if (!n) return 0;
    if (n > 1)
    {
	rlen += (n - 1) * dlen;
    }
    char *joined = xmalloc(rlen + 1);
    strcpy(joined, *strings);
    char *w = joined + strlen(*strings);
    cur = strings+1;
    while (*cur)
    {
	strcpy(w, delim);
	w += dlen;
	strcpy(w, *cur);
	w += strlen(*cur);
	++cur;
    }
    return joined;
}

SOLOCAL uint8_t hashstr(const char *key, uint8_t mask)
{
    size_t h = 5381;
    while (*key)
    {
	h += (h << 5) + ((uint8_t)*key++);
    }
    return h & mask;
}

SOLOCAL void appendchr(char **str, size_t *size, size_t *pos,
	size_t chunksz, char c)
{
    if (*pos >= *size)
    {
	*size += chunksz;
	*str = xrealloc(*str, *size);
    }
    (*str)[(*pos)++] = c;
}

