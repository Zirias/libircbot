#ifndef IRCBOT_IRCCHANNEL_H
#define IRCBOT_IRCCHANNEL_H

#include <ircbot/decl.h>

/** declarations for the IrcChannel class
 * @file
 */

C_CLASS_DECL(IBHashTable);

/** An IRC channel.
 * @class IrcChannel ircchannel.h <ircbot/ircchannel.h>
 */
C_CLASS_DECL(IrcChannel);

C_CLASS_DECL(IrcServer);

/** The name of the channel.
 * @memberof IrcChannel
 * @param self the IrcChannel
 * @returns the name of the channel
 */
DECLEXPORT const char *IrcChannel_name(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL;

/** The server of the channel.
 * @memberof IrcChannel
 * @param self the IrcChannel
 * @returns the server of the channel
 */
DECLEXPORT const IrcServer *IrcChannel_server(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;

/** Whether the bot is currently on this channel.
 * @memberof IrcChannel
 * @param self the IrcChannel
 * @returns 1 if the channel is currently joined, 0 otherwise
 */
DECLEXPORT int IrcChannel_isJoined(const IrcChannel *self) CMETHOD;

/** A hash table of all the nicks currently on the channel.
 * The nicks are the keys of the hash table, the value is always the name
 * of the channel.
 * @memberof IrcChannel
 * @param self the IrcChannel
 * @returns a hash table of the nicks
 */
DECLEXPORT const IBHashTable *IrcChannel_nicks(const IrcChannel *self)
    CMETHOD ATTR_RETNONNULL;

#endif
