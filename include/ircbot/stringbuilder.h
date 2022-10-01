#ifndef IRCBOT_STRINGBUILDER_H
#define IRCBOT_STRINGBUILDER_H

#include <ircbot/decl.h>

C_CLASS_DECL(StringBuilder);

#define StringBuilder_pass(builder, str) do { \
    char *sbp__append = (str); \
    StringBuilder_append((builder), sbp__append); \
    free(sbp__append); } while (0)

DECLEXPORT StringBuilder *StringBuilder_create(void) ATTR_RETNONNULL;
DECLEXPORT void StringBuilder_append(StringBuilder *self, const char *str)
    CMETHOD ATTR_NONNULL((2));
DECLEXPORT const char *StringBuilder_str(const StringBuilder *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;
DECLEXPORT void StringBuilder_destroy(StringBuilder *self);

#endif
