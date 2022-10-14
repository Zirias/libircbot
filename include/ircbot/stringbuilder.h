#ifndef IRCBOT_STRINGBUILDER_H
#define IRCBOT_STRINGBUILDER_H

#include <ircbot/decl.h>

/** declarations for the IBStringBuilder class
 * @file
 */

/** A simple string builder
 * @class IBStringBuilder stringbuilder.h <ircbot/stringbuilder.h>
 */
C_CLASS_DECL(IBStringBuilder);

/** IBStringBuilder default constructor.
 * Creates a new IBStringBuilder
 * @memberof IBStringBuilder
 * @returns a newly created IBStringBuilder
 */
DECLEXPORT IBStringBuilder *IBStringBuilder_create(void) ATTR_RETNONNULL;

/** Append a string to the builder.
 * @memberof IBStringBuilder
 * @param self the IBStringBuilder
 * @param str the string to append
 */
DECLEXPORT void IBStringBuilder_append(IBStringBuilder *self, const char *str)
    CMETHOD ATTR_NONNULL((2));

/** Get the complete string.
 * @memberof IBStringBuilder
 * @param self the IBStringBuilder
 * @returns the complete string
 */
DECLEXPORT const char *IBStringBuilder_str(const IBStringBuilder *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;

/** IBStringBuilder destructor.
 * @memberof IBStringBuilder
 * @param self the IBStringBuilder
 */
DECLEXPORT void IBStringBuilder_destroy(IBStringBuilder *self);

#endif
