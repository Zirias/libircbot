#ifndef IRCBOT_IRCSERVER_H
#define IRCBOT_IRCSERVER_H

#include <ircbot/decl.h>

/** declarations for the IrcServer class
 * @file
 */

C_CLASS_DECL(IBHashTable);
C_CLASS_DECL(IrcChannel);

/** An IRC server.
 * @class IrcServer ircserver.h <ircbot/ircserver.h>
 */
C_CLASS_DECL(IrcServer);

/** Create a new IRC server to connect to.
 * @memberof IrcServer
 * @param id an identifier for the server, e.g. the name of the IRC network
 * @param remotehost host name or IP address to connect to
 * @param port port number to connect to (typically 6667)
 * @param nick nickname to use on that server
 * @param user username to use, NULL to derive from local account
 * @param realname real name to use, NULL to derive from local account
 */
DECLEXPORT IrcServer *IrcServer_create(const char *id,
	const char *remotehost, int port,
	const char *nick, const char *user, const char *realname)
    ATTR_RETNONNULL ATTR_NONNULL((1))
    ATTR_NONNULL((2)) ATTR_NONNULL((4));

/** The identifier of the server.
 * @memberof IrcServer
 * @param self the IrcServer
 * @returns the identifier
 */
DECLEXPORT const char *IrcServer_id(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;

/** The name of the server.
 * This is either the name advertised by the server itself, or as long as this
 * isn't known yet, the remote host name given in IrcServer_create().
 * @memberof IrcServer
 * @param self the IrcServer
 * @returns the name of the server
 */
DECLEXPORT const char *IrcServer_name(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;

/** The nick currently used on the server.
 * @memberof IrcServer
 * @param self the IrcServer
 * @returns the current own nick
 */
DECLEXPORT const char *IrcServer_nick(const IrcServer *self)
    CMETHOD ATTR_RETNONNULL;

/** A hash table of all currently requested or joined channels.
 * The keys are the channel names, the objects are IrcChannel objects.
 * @memberof IrcServer
 * @param self the IrcServer
 * @returns a hash table of channels
 */
DECLEXPORT const IBHashTable *IrcServer_channels(const IrcServer *self)
    CMETHOD;

/** Request to join a channel.
 * Do NOT use this function from bot event handlers!
 * @memberof IrcServer
 * @param self the IrcServer
 * @param channel the name of the channel to join
 */
DECLEXPORT void IrcServer_join(IrcServer *self, const char *channel)
    CMETHOD ATTR_NONNULL((2));

/** Request to leave a channel.
 * Do NOT use this function from bot event handlers!
 * @memberof IrcServer
 * @param self the IrcServer
 * @param channel the name of the channel to leave
 */
DECLEXPORT void IrcServer_part(IrcServer *self, const char *channel)
    CMETHOD ATTR_NONNULL((2));

/** IrcServer destructor.
 * @memberof IrcServer
 * @param self the IrcServer
 */
DECLEXPORT void IrcServer_destroy(IrcServer *self);

#endif
