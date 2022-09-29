#include <ircbot/config.h>
#include <ircbot/event.h>
#include <ircbot/ircbot.h>
#include <ircbot/ircserver.h>
#include <ircbot/log.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SERVER "irc.libera.chat"
#define PORT 6667
#define NICK "wumsbot"
#define USER "wumsbot"
#define REALNAME "wumsbot"
#define CHANNEL "#bsd-de"

static const char *bier[] = {
    "Prost!",
    "Feierabend?",
    "Beer is the answer, but I can't remember the question ...",
    "gmBh heißt, geh mal Bier holen!",
    "Bierpreisbremse jetzt!"
};

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
    else if (!strncmp(ea->message, "!bier ", 6)
	    || !strcmp(ea->message, "!bier"))
    {
	IrcServer_sendMsg(server, ea->to,
		bier[rand() % (sizeof bier / sizeof *bier)]);
    }
}

int main(void)
{
    setFileLogger(stderr);
    srand(time(0));

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

