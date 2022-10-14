#ifndef IRCBOT_LOG_H
#define IRCBOT_LOG_H

#include <ircbot/decl.h>

#include <stdio.h>

/** declarations for the IBLog static class
 * @file
 */

/** Static class providing logging functionality with configurable targets.
 * @class IBLog log.h <ircbot/log.h>
 */

#define MAXLOGLINE 16384 /**< maximum length for a log line */

/** Logging level
 * @enum IBLogLevel log.h <ircbot/log.h>
 */
typedef enum IBLogLevel
{
    L_FATAL,    /**< program execution can't continue */
    L_ERROR,    /**< an error message, can't successfully complete */
    L_WARNING,  /**< a warning message, something seems wrong */
    L_INFO,     /**< an information message */
    L_DEBUG     /**< a debugging message, very verbose */
} IBLogLevel;

/** A log writer.
 * This function is responsible to write a processed log message to its
 * ultimate target, like e.g. a logfile.
 * @param level the log level of the message.
 * @param message the message to write
 * @param data optional additional context for the log writer
 */
typedef void (*IBLogWriter)(IBLogLevel level, const char *message, void *data)
    ATTR_NONNULL((2));

/** Set a log writer logging to the specified file handle.
 * @memberof IBLog
 * @param file the file handle to write to
 */
DECLEXPORT void IBLog_setFileLogger(FILE *file) ATTR_NONNULL((1));

/** Set a log writer logging via syslog().
 * @memberof IBLog
 * @param ident identification of the logger
 * @param facility syslog facility to use, see <syslog.h>
 * @param withStderr if non-zero, also log to stderr
 */
DECLEXPORT void IBLog_setSyslogLogger(const char *ident, int facility,
	int withStderr) ATTR_NONNULL((1));

/** Set a custom log writer.
 * @memberof IBLog
 * @param writer the log writer
 * @param data optional context for the writer
 */
DECLEXPORT void IBLog_setCustomLogger(IBLogWriter writer, void *data)
    ATTR_NONNULL((1));

/** Set the maximum level for logging.
 * Default: L_INFO.
 * @memberof IBLog
 * @param level the maximum level
 */
DECLEXPORT void IBLog_setMaxLogLevel(IBLogLevel level);

/** Silence all logging.
 * This is meant to temporarily disable any logging except L_FATAL, for cases
 * when some errors are expected and should be suppressed.
 * @memberof IBLog
 * @param silent 1 to enable silent mode, 0 to disable
 */
DECLEXPORT void IBLog_setSilent(int silent);

/** Enable async logging.
 * If this is set, logging will attempt to offload calling the log writer to
 * the internal thread pool. This is recommended for any log writer that
 * could block. IrcBot_run() will automatically set this if daemonizing was
 * requested.
 * Default: 0
 * @memberof IBLog
 * @param async 1 to enable async logging, 0 to disable
 */
DECLEXPORT void IBLog_setAsync(int async);

/** Log a message.
 * @memberof IBLog
 * @param level the log level
 * @param message the message
 */
DECLEXPORT void IBLog_msg(IBLogLevel level, const char *message)
    ATTR_NONNULL((2));

/** Log a message, using printf-like formatting.
 * @memberof IBLog
 * @param level the log level
 * @param format the format string
 * @param ... the arguments for conversions in the format string
 */
DECLEXPORT void IBLog_fmt(IBLogLevel level, const char *format, ...)
    ATTR_NONNULL((2)) ATTR_FORMAT((printf, 2, 3));

#endif

