#ifndef IRCBOT_STRINGBUILDER_H
#define IRCBOT_STRINGBUILDER_H

#include <ircbot/decl.h>

C_CLASS_DECL(IBStringBuilder);

DECLEXPORT IBStringBuilder *IBStringBuilder_create(void) ATTR_RETNONNULL;
DECLEXPORT void IBStringBuilder_append(IBStringBuilder *self, const char *str)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT const char *IBStringBuilder_str(const IBStringBuilder *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT void IBStringBuilder_destroy(IBStringBuilder *self);

#endif
