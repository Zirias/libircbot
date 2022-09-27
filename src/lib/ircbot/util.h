#ifndef IRCBOT_INT_UTIL_H
#define IRCBOT_INT_UTIL_H

#include <ircbot/decl.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define HT_MASK(bits) (((1U<<(bits))-1)&0xffU)
#define HT_SIZE(bits) (HT_MASK(bits)+1)
#define hash(key, bits) hashstr((key), HT_MASK(bits))

#define appendstrlit(str, size, pos, chunksz, strlit) \
    for (size_t i = 0; i < sizeof strlit - 1; ++i) \
    appendchr((str), (size), (pos), (chunksz), strlit[i])

void *xmalloc(size_t size) ATTR_MALLOC ATTR_RETNONNULL ATTR_ALLOCSZ((1));
void *xrealloc(void *ptr, size_t size) ATTR_RETNONNULL ATTR_ALLOCSZ((2));
char *copystr(const char *src) ATTR_MALLOC;
char *lowerstr(const char *src) ATTR_MALLOC;
char *joinstr(const char *delim, char **strings) ATTR_MALLOC ATTR_NONNULL((1));
uint8_t hashstr(const char *key, uint8_t mask) ATTR_NONNULL((1)) ATTR_PURE;
void appendchr(char **str, size_t *size, size_t *pos, size_t chunksz, char c)
    ATTR_NONNULL((1)) ATTR_NONNULL((2)) ATTR_NONNULL((3))
    ATTR_ACCESS((read_write, 1)) ATTR_ACCESS((read_write, 2))
    ATTR_ACCESS((read_write, 3));

#endif
