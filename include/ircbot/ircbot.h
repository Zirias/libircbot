#ifndef IRCBOT_IRCBOT_H
#define IRCBOT_IRCBOT_H

#include <ircbot/decl.h>

C_CLASS_DECL(Config);
C_CLASS_DECL(IrcServer);

DECLEXPORT int IrcBot_run(Config *config, IrcServer *server);

#endif
