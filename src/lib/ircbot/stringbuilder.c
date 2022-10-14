#include <ircbot/stringbuilder.h>

#include "util.h"

#include <stdlib.h>
#include <string.h>

#define SBCHUNKSZ 512

struct IBStringBuilder
{
    size_t size;
    size_t capa;
    char *str;
};

SOEXPORT IBStringBuilder *IBStringBuilder_create(void)
{
    IBStringBuilder *self = IB_xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    return self;
}

SOEXPORT void IBStringBuilder_append(IBStringBuilder *self, const char *str)
{
    size_t newsz = self->size + strlen(str);
    if (self->capa <= newsz)
    {
	while (self->capa <= newsz) self->capa += SBCHUNKSZ;
	self->str = IB_xrealloc(self->str, self->capa);
    }
    strcpy(self->str + self->size, str);
    self->size = newsz;
}

SOEXPORT const char *IBStringBuilder_str(const IBStringBuilder *self)
{
    return self->str;
}

SOEXPORT void IBStringBuilder_destroy(IBStringBuilder *self)
{
    if (!self) return;
    free(self->str);
    free(self);
}

