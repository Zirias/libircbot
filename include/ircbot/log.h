#ifndef IRCBOT_LOG_H
#define IRCBOT_LOG_H

#include <ircbot/decl.h>

#include <stdio.h>

#define MAXLOGLINE 16384

typedef enum IBLogLevel
{
    L_FATAL,    /**< program execution can't continue */
    L_ERROR,    /**< an error message, can't successfully complete */
    L_WARNING,  /**< a warning message, something seems wrong */
    L_INFO,     /**< an information message */
    L_DEBUG     /**< a debugging message, very verbose */
} IBLogLevel;

typedef void (*IBLogWriter)(IBLogLevel level, const char *message, void *data)
    ATTR_NONNULL((2));

DECLEXPORT void IBLog_setFileLogger(FILE *file) ATTR_NONNULL((1));
DECLEXPORT void IBLog_setSyslogLogger(const char *ident, int facility,
	int withStderr) ATTR_NONNULL((1));
DECLEXPORT void IBLog_setCustomLogger(IBLogWriter writer, void *data)
    ATTR_NONNULL((1));
DECLEXPORT void IBLog_setMaxLogLevel(IBLogLevel level);
DECLEXPORT void IBLog_setSilent(int silent);
DECLEXPORT void IBLog_setAsync(int async);

DECLEXPORT void IBLog_msg(IBLogLevel level, const char *message)
    ATTR_NONNULL((2));
DECLEXPORT void IBLog_fmt(IBLogLevel level, const char *format, ...)
    ATTR_NONNULL((2)) ATTR_FORMAT((printf, 2, 3));

#endif

