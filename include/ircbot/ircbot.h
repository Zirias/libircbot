#ifndef IRCBOT_IRCBOT_H
#define IRCBOT_IRCBOT_H

#include <ircbot/decl.h>

/** declarations for the main IrcBot API and related classes
 * @file
 */

/** Static class containing the primary IRC bot API.
 * @class IrcBot ircbot.h <ircbot/ircbot.h>
 */

/** A bot event to be handled.
 * @class IrcBotEvent ircbot.h <ircbot/ircbot.h>
 */
C_CLASS_DECL(IrcBotEvent);

/** A response to a bot event, to be filled by a handler.
 * @class IrcBotResponse ircbot.h <ircbot/ircbot.h>
 */
C_CLASS_DECL(IrcBotResponse);

C_CLASS_DECL(IrcChannel);
C_CLASS_DECL(IrcServer);

/** A configuration object for the integrated thread pool.
 * @class IBThreadOpts ircbot.h <ircbot/ircbot.h>
 */
C_CLASS_DECL(IBThreadOpts);

/** Type of a bot event.
 * @enum IrcBotEventType ircbot.h <ircbot/ircbot.h>
 */
typedef enum IrcBotEventType
{
    IBET_BOTCOMMAND,	/**< A bot command, !-prefixed or sent private */
    IBET_PRIVMSG,	/**< A generic PRIVMSG (channel or private) */
    IBET_CONNECTED,	/**< Connected to IRC server */
    IBET_CHANJOINED,	/**< Channel joined by bot */
    IBET_JOINED,	/**< Channel joined by other user */
    IBET_PARTED		/**< Channel left by other user */
} IrcBotEventType;

/** Handler for a bot event.
 * Will be executed on a worker thread.
 * @param event the event to handle
 */
typedef void (*IrcBotHandler)(IrcBotEvent *event) ATTR_NONNULL((1));

#define ORIGIN_PRIVATE ":" /**< the origin is any private message */
#define ORIGIN_CHANNEL "#" /**< the origin is any channel */

/** Obtain the current thread pool options for configuration.
 * @memberof IrcBot
 * @returns the IBThreadOpts to configure
 */
DECLEXPORT IBThreadOpts *IrcBot_threadOpts(void) ATTR_RETNONNULL;

/** Set fixed number of threads.
 * If this is given, the thread pool always uses this number of threads.
 * Default: 0 (unset).
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the numer of threads
 */
DECLEXPORT void IBThreadOpts_setNThreads(IBThreadOpts *self, int num)
    CMETHOD;

/** Set maximum number of threads.
 * If number of threads are derived from the number of CPU, this will be the
 * upper limit to use.
 * Default: 128.
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the maximum number of threads
 */
DECLEXPORT void IBThreadOpts_setMaxThreads(IBThreadOpts *self, int num)
    CMETHOD;

/** Set number of threads per CPU.
 * Defaults to 2 threads per CPU.
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the number of threads per CPU
 */
DECLEXPORT void IBThreadOpts_setNPerCpu(IBThreadOpts *self, int num)
    CMETHOD;

/** Set default number of threads.
 * This is used if no fixed number is given and the number of CPUs can't be
 * determined.
 * Default: 8.
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the default number of threads
 */
DECLEXPORT void IBThreadOpts_setDefNThreads(IBThreadOpts *self, int num)
    CMETHOD;

/** Set a fixed size for the queue of waiting thread jobs.
 * Default: 0 (unset).
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the queue size
 */
DECLEXPORT void IBThreadOpts_setQueueLen(IBThreadOpts *self, int num)
    CMETHOD;

/** Set the maximum size for the queue of waiting thread jobs.
 * Default: 1024
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the maximum queue size
 */
DECLEXPORT void IBThreadOpts_setMaxQueueLen(IBThreadOpts *self, int num)
    CMETHOD;

/** Set the minimum size for the queue of waiting thread jobs.
 * Default: 64
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the minimum queue size
 */
DECLEXPORT void IBThreadOpts_setMinQueueLen(IBThreadOpts *self, int num)
    CMETHOD;

/** Set the number of queue entries per thread.
 * Default: 2
 * @memberof IBThreadOpts
 * @param self the IBThreadOpts
 * @param num the queue entries per thread
 */
DECLEXPORT void IBThreadOpts_setQLenPerThread(IBThreadOpts *self, int num)
    CMETHOD;

/** Request the bot to deamonize first when running.
 * If a pidfile is given, it is also used to check for an already running
 * instance.
 * @memberof IrcBot
 * @param uid the user id to run as, -1 to not change
 * @param gid the group id to run as, -1 for default or no change
 * @param pidfile full path to the pidfile to use, or NULL for none
 * @param started callback to run when the daemon launched successfully, or NULL
 */
DECLEXPORT void IrcBot_daemonize(long uid, long gid,
	const char *pidfile, void (*started)(void));

/** Set a custom startup function.
 * This function will be run during startup of the bot. It should return
 * EXIT_SUCCESS if everything is fine. If it returns EXIT_FAILURE, startup is
 * aborted and the bot exits.
 * This function is called after the bot successfully started some internal
 * things and, if requested to do so, changed its user id.
 * @memberof IrcBot
 * @param startup the function to call on startup
 */
DECLEXPORT void IrcBot_startup(int (*startup)(void));

/** Set a custom shutdown function.
 * This function will be called during shutdown of the bot. It can for example
 * be used to free resources that were allocated during startup.
 * @memberof IrcBot
 * @param shutdown the function to call on shutdown
 */
DECLEXPORT void IrcBot_shutdown(void (*shutdown)(void));

/** Register a handler for a bot event.
 * The handler registered will be executed on a worker thread when a matching
 * bot event occurs. It should examine the IrcBotEvent it gets passed and then
 * configure the IrcBotResponse that can be obtained from that event.
 * @memberof IrcBot
 * @param eventType the type of the event
 * @param serverId the id of the IrcServer, or NULL for any server
 * @param origin the channel name or the nick of the bot, or ORIGIN_CHANNEL
 *               for any channel, or ORIGIN_PRIVATE for any message received
 *               privately, or NULL for any message
 * @param filter an additional filter depending on the eventType, e.g. the
 *               command for IBET_BOTCOMMAND, or NULL for any
 * @param handler the handler to execute for the event
 */
DECLEXPORT void IrcBot_addHandler(IrcBotEventType eventType,
	const char *serverId, const char *origin, const char *filter,
	IrcBotHandler handler);

/** Add an IRC server to be managed by the bot.
 * Any server added will be automatically connected to. The bot will also
 * destroy the server on shutdown.
 * @memberof IrcBot
 * @param server the server
 */
DECLEXPORT void IrcBot_addServer(IrcServer *server) ATTR_NONNULL((1));

/** Run the IRC bot.
 * This should typically be the last function to call in your main(). It will
 * launch a thread pool, a service loop and run the bot as previously
 * configured, e.g. daemonize if requested and connect to all servers that
 * were added.
 * @memberof IrcBot
 * @returns the exit code
 */
DECLEXPORT int IrcBot_run(void);

/** The type of the event.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns the type of the event
 */
DECLEXPORT IrcBotEventType IrcBotEvent_type(const IrcBotEvent *self) CMETHOD;

/** The server the event came from.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns the server
 */
DECLEXPORT const IrcServer *IrcBotEvent_server(const IrcBotEvent *self)
    CMETHOD;

/** The channel the event occured on.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns the channel, of NULL if there isn't one
 */
DECLEXPORT const IrcChannel *IrcBotEvent_channel(const IrcBotEvent *self)
    CMETHOD;

/** The origin of the event.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns A channel name, or the nick of the bot for a private message,
 *          or NULL if not applicable
 */
DECLEXPORT const char *IrcBotEvent_origin(const IrcBotEvent *self) CMETHOD;

/** The bot command.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns the bot command, or NULL if this is not a command event
 */
DECLEXPORT const char *IrcBotEvent_command(const IrcBotEvent *self) CMETHOD;

/** The sender of the message.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns the nick who sent the originating message, of NULL if none
 */
DECLEXPORT const char *IrcBotEvent_from(const IrcBotEvent *self) CMETHOD;

/** Additional arguments of the event.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns a string with all additional arguments, or NULL if there are none
 */
DECLEXPORT const char *IrcBotEvent_arg(const IrcBotEvent *self) CMETHOD;

/** Obtain a response object to configure.
 * @memberof IrcBotEvent
 * @param self the IrcBotEvent
 * @returns an object to hold any responses a handler wants to add
 */
DECLEXPORT IrcBotResponse *IrcBotEvent_response(IrcBotEvent *self)
    CMETHOD ATTR_RETNONNULL;

/** Add a message to a bot response.
 * @memberof IrcBotResponse
 * @param self the IrcBotResponse
 * @param to channel or nick to send the message to
 * @param msg the message to send
 * @param action 0 for a normal message, 1 for an ACTION (like /me command)
 */
DECLEXPORT void IrcBotResponse_addMsg(IrcBotResponse *self,
	const char *to, const char *msg, int action)
    CMETHOD ATTR_NONNULL((2)) ATTR_NONNULL((3));

#endif
