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
    IrcBotResponse *response = IrcBotEvent_response(event);
    const char *arg = IrcBotEvent_arg(event);
    const char *origin = IrcBotEvent_origin(event);
    const IrcChannel *channel = IrcServer_channel(
	    IrcBotEvent_server(event), origin);

    if (!channel) return;

    if (arg)
    {
	char buf[256];
	List *beerfor = List_fromString(arg, " \t");
	if (beerfor)
	{
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
		IrcBotResponse_addMsg(response, origin, buf, 1);
	    }
	    ListIterator_destroy(i);
	    List_destroy(beerfor);
	}
    }
    else
    {
	IrcBotResponse_addMsg(response, origin,
		beer[rand() % (sizeof beer / sizeof *beer)], 0);
    }
}

static void started(void)
{
    setSyslogLogger("libircbottest", LOG_DAEMON, 0);
    srand(time(0));
}

int main(void)
{
    setSyslogLogger("libircbottest", LOG_DAEMON, 1);

    IrcServer *server = IrcServer_create(IRCNET, SERVER, PORT,
	    NICK, USER, REALNAME);
    IrcServer_join(server, CHANNEL);
    IrcBot_addServer(server);

    IrcBot_addHandler(IBET_JOINED, 0, 0, 0, joined);
    IrcBot_addHandler(IBET_BOTCOMMAND, 0, ORIGIN_CHANNEL, "say", say);
    IrcBot_addHandler(IBET_BOTCOMMAND, 0, ORIGIN_CHANNEL, "bier", bier);

    IrcBot_daemonize(-1, -1, "/tmp/libircbottest.pid", started);

    return IrcBot_run();
}

