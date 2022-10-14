#ifndef IRCBOT_IRCMESSAGE_H
#define IRCBOT_IRCMESSAGE_H

#include <ircbot/decl.h>
#include <ircbot/irccommand.h>

/** declarations for the IrcMessage class
 * @file
 */

/** An IRC protocol message
 * @class IrcMessage ircmessage.h <ircbot/ircmessage.h>
 */
C_CLASS_DECL(IrcMessage);

C_CLASS_DECL(IBList);

/** The prefix of the message.
 * @memberof IrcMessage
 * @param self the IrcMessage
 * @returns the prefix, or NULL if there is none
 */
DECLEXPORT const char *IrcMessage_prefix(const IrcMessage *self)
    CMETHOD ATTR_PURE;

/** The command of the message.
 * @memberof IrcMessage
 * @param self the IrcMessage
 * @returns the command of the message
 */
DECLEXPORT IrcCommand IrcMessage_command(const IrcMessage *self)
    CMETHOD ATTR_PURE;

/** The unparsed command of the message.
 * @memberof IrcMessage
 * @param self the IrcMessage
 * @returns the original string received for the command
 */
DECLEXPORT const char *IrcMessage_rawCmd(const IrcMessage *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;

/** The parameters of the message.
 * @memberof IrcMessage
 * @param self the IrcMessage
 * @returns the list of parameters
 */
DECLEXPORT const IBList *IrcMessage_params(const IrcMessage *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;

/** The unparsed parameters of the message.
 * @memberof IrcMessage
 * @param self the IrcMessage
 * @returns the original string received for the parameters
 */
DECLEXPORT const char *IrcMessage_rawParams(const IrcMessage *self)
    CMETHOD ATTR_RETNONNULL ATTR_PURE;

#endif
