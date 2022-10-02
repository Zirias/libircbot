#include <ircbot/config.h>
#include <ircbot/event.h>
#include <ircbot/hashtable.h>
#include <ircbot/ircbot.h>
#include <ircbot/ircchannel.h>
#include <ircbot/ircserver.h>
#include <ircbot/log.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IRCNET "libera"
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
	if (ea->from && !strcmp(ea->message+5, "my name"))
	{
	    IrcServer_sendMsg(server, ea->to, ea->from, 0);
	}
	else
	{
	    IrcServer_sendMsg(server, ea->to, ea->message+5, 0);
	}
    }
    else if (!strcmp(ea->message, "!bier"))
    {
	IrcServer_sendMsg(server, ea->to,
		bier[rand() % (sizeof bier / sizeof *bier)], 0);
    }
    else if (strlen(ea->message) > 6 && !strncmp(ea->message, "!bier ", 6))
    {
	const char *beerfor = ea->message + 6;
	if (!beerfor[strcspn(beerfor, " ")])
	{
	    char buf[256];
	    snprintf(buf, 256, "wird %s mit Bier abfüllen!", beerfor);
	    IrcServer_sendMsg(server, ea->to, buf, 1);
	}
    }
}

static void userJoined(void *receiver, void *sender, void *args)
{
    IrcServer *server = receiver;
    IrcChannel *channel = sender;
    const char *nick = args;

    char buf[256];
    snprintf(buf, 256, "Hallo %s!", nick);
    IrcServer_sendMsg(server, IrcChannel_name(channel), buf, 0);
}

static void chanJoined(void *receiver, void *sender, void *args)
{
    (void)receiver;

    IrcServer *server = sender;
    IrcChannel *channel = args;

    Event_register(IrcChannel_joined(channel), server, userJoined, 0);
}

int main(void)
{
    setFileLogger(stderr);
    srand(time(0));

    Config config;
    memset(&config, 0, sizeof config);
    config.uid = -1;

    IrcServer *server = IrcServer_create(IRCNET, SERVER, PORT,
	    NICK, USER, REALNAME);
    IrcServer_join(server, CHANNEL);
    Event_register(IrcServer_msgReceived(server), 0, msgReceived, 0);
    Event_register(IrcServer_joined(server), 0, chanJoined, 0);

    int rc = IrcBot_run(&config, server);

    IrcServer_destroy(server);
    return rc;
}

