#include <ircbot/config.h>
#include <ircbot/event.h>
#include <ircbot/ircbot.h>
#include <ircbot/ircserver.h>
#include <ircbot/log.h>

#include <string.h>

#define SERVER "irc.libera.chat"
#define PORT 6667
#define NICK "wumsbot"
#define USER "wumsbot"
#define REALNAME "wumsbot"
#define CHANNEL "#bsd-de"

static void msgReceived(void *receiver, void *sender, void *args)
{
    (void)receiver;

    IrcServer *server = sender;
    MsgReceivedEventArgs *ea = args;

    if (strcmp(ea->to, CHANNEL)) return;

    if (strlen(ea->message) > 5 && !strncmp(ea->message, "!say ", 5))
    {
	IrcServer_sendMsg(server, ea->to, ea->message+5);
    }
}

int main(void)
{
    setFileLogger(stderr);
    setMaxLogLevel(L_DEBUG);

    Config config;
    memset(&config, 0, sizeof config);
    config.uid = -1;

    IrcServer *server = IrcServer_create(SERVER, PORT, NICK, USER, REALNAME);
    IrcServer_join(server, CHANNEL);
    Event_register(IrcServer_msgReceived(server), 0, msgReceived, 0);

    int rc = IrcBot_run(&config, server);

    IrcServer_destroy(server);
    return rc;
}

