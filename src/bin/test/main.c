#include <ircbot/hashtable.h>
#include <ircbot/ircbot.h>
#include <ircbot/ircchannel.h>
#include <ircbot/ircserver.h>
#include <ircbot/list.h>
#include <ircbot/log.h>

#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#define IRCNET "libera"
#define SERVER "irc.libera.chat"
#define PORT 6667
#define NICK "wumsbot"
#define USER "wumsbot"
#define REALNAME "wumsbot"
#define CHANNEL "#bsd-de"

static const char *beer[] = {
    "Prost!",
    "Feierabend?",
    "Beer is the answer, but I can't remember the question ...",
    "gmBh heißt, geh mal Bier holen!",
    "Bierpreisbremse jetzt!"
};

static void joined(IrcBotEvent *event)
{
    IrcBotResponse *response = IrcBotEvent_response(event);
    char buf[256];
    snprintf(buf, 256, "Hallo %s!", IrcBotEvent_arg(event));
    IrcBotResponse_addMsg(response, IrcBotEvent_origin(event), buf, 0);
}

static void say(IrcBotEvent *event)
{
    IrcBotResponse *response = IrcBotEvent_response(event);
    const char *arg = IrcBotEvent_arg(event);

    if (!strcmp(arg, "my name"))
    {
	IrcBotResponse_addMsg(response, IrcBotEvent_origin(event),
		IrcBotEvent_from(event), 0);
    }
    else
    {
	IrcBotResponse_addMsg(response, IrcBotEvent_origin(event), arg, 0);
    }
}

static void bier(IrcBotEvent *event)
{
    const IrcChannel *channel = IrcBotEvent_channel(event);
    if (!channel) return;

    const char *arg = IrcBotEvent_arg(event);
    List *beerfor;
    IrcBotResponse *response = IrcBotEvent_response(event);
    if (arg && (beerfor = List_fromString(arg, " \t")))
    {
	char buf[256];
	ListIterator *i = List_iterator(beerfor);
	while (ListIterator_moveNext(i))
	{
	    const char *nick = ListIterator_current(i);
	    if (HashTable_get(IrcChannel_nicks(channel), nick))
	    {
		snprintf(buf, 256, "wird %s mit Bier abfüllen!", nick);
	    }
	    else
	    {
		snprintf(buf, 256, "sieht hier keine(n) %s ...", nick);
	    }
	    IrcBotResponse_addMsg(response, IrcBotEvent_origin(event), buf, 1);
	}
	ListIterator_destroy(i);
	List_destroy(beerfor);
    }
    else
    {
	IrcBotResponse_addMsg(response, IrcBotEvent_origin(event),
		beer[rand() % (sizeof beer / sizeof *beer)], 0);
    }
}

static void started(void)
{
    setSyslogLogger("libircbottest", LOG_DAEMON, 0);
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "-f"))
    {
	setFileLogger(stderr);
    }
    else
    {
	setSyslogLogger("libircbottest", LOG_DAEMON, 1);
	IrcBot_daemonize(-1, -1, "/tmp/libircbottest.pid", started);
    }

    IrcServer *server = IrcServer_create(IRCNET, SERVER, PORT,
	    NICK, USER, REALNAME);
    IrcServer_join(server, CHANNEL);
    IrcBot_addServer(server);

    IrcBot_addHandler(IBET_JOINED, 0, 0, 0, joined);
    IrcBot_addHandler(IBET_BOTCOMMAND, 0, ORIGIN_CHANNEL, "say", say);
    IrcBot_addHandler(IBET_BOTCOMMAND, 0, ORIGIN_CHANNEL, "bier", bier);

    srand(time(0));

    return IrcBot_run();
}

