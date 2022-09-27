#ifndef IRCBOT_INT_LOG_H
#define IRCBOT_INT_LOG_H

#include <ircbot/decl.h>

#include <stdio.h>

#define MAXLOGLINE 16384

typedef enum LogLevel
{
    L_FATAL,    /**< program execution can't continue */
    L_ERROR,    /**< an error message, can't successfully complete */
    L_WARNING,  /**< a warning message, something seems wrong */
    L_INFO,     /**< an information message */
    L_DEBUG     /**< a debugging message, very verbose */
} LogLevel;

typedef void (*logwriter)(LogLevel level, const char *message, void *data)
    ATTR_NONNULL((2));

void setFileLogger(FILE *file) ATTR_NONNULL((1));
void setCustomLogger(logwriter writer, void *data) ATTR_NONNULL((1));
void setMaxLogLevel(LogLevel level);

void logmsg(LogLevel level, const char *message) ATTR_NONNULL((2));
void logfmt(LogLevel level, const char *format, ...)
    ATTR_NONNULL((2)) ATTR_FORMAT((printf, 2, 3));
void logsetsilent(int silent);
void logsetasync(int async);

#endif

