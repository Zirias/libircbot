#include <ircbot/irccommand.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

SOEXPORT IrcCommand IrcCommand_parse(const char *cmd)
{
    if (isdigit(cmd[0]) && isdigit(cmd[1]) && isdigit(cmd[2]) && !cmd[3])
    {
	IrcCommand self = atoi(cmd);
	if ((self > 99 && self < 200) || !IrcCommand_str(self))
	{
	    self = CMD_UNKNOWN;
	}
	return self;
    }
    switch (cmd[0])
    {
	case 'A':
	    if (!strcmp(cmd, "ADMIN")) return MSG_ADMIN;
	    if (!strcmp(cmd, "AWAY")) return MSG_AWAY;
	    break;
	case 'C':
	    if (!strcmp(cmd, "CONNECT")) return MSG_CONNECT;
	    break;
	case 'D':
	    if (!strcmp(cmd, "DIE")) return MSG_DIE;
	    break;
	case 'E':
	    if (!strcmp(cmd, "ERROR")) return MSG_ERROR;
	    break;
	case 'I':
	    if (!strcmp(cmd, "INFO")) return MSG_INFO;
	    if (!strcmp(cmd, "INVITE")) return MSG_INVITE;
	    if (!strcmp(cmd, "ISON")) return MSG_ISON;
	    break;
	case 'J':
	    if (!strcmp(cmd, "JOIN")) return MSG_JOIN;
	    break;
	case 'K':
	    if (!strcmp(cmd, "KICK")) return MSG_KICK;
	    if (!strcmp(cmd, "KILL")) return MSG_KILL;
	    break;
	case 'L':
	    if (!strcmp(cmd, "LINKS")) return MSG_LINKS;
	    if (!strcmp(cmd, "LIST")) return MSG_LIST;
	    if (!strcmp(cmd, "LUSERS")) return MSG_LUSERS;
	    break;
	case 'M':
	    if (!strcmp(cmd, "MODE")) return MSG_MODE;
	    if (!strcmp(cmd, "MOTD")) return MSG_MOTD;
	    break;
	case 'N':
	    if (!strcmp(cmd, "NAMES")) return MSG_NAMES;
	    if (!strcmp(cmd, "NICK")) return MSG_NICK;
	    if (!strcmp(cmd, "NOTICE")) return MSG_NOTICE;
	    break;
	case 'O':
	    if (!strcmp(cmd, "OPER")) return MSG_OPER;
	    break;
	case 'P':
	    if (!strcmp(cmd, "PASS")) return MSG_PASS;
	    if (!strcmp(cmd, "PART")) return MSG_PART;
	    if (!strcmp(cmd, "PING")) return MSG_PING;
	    if (!strcmp(cmd, "PONG")) return MSG_PONG;
	    if (!strcmp(cmd, "PRIVMSG")) return MSG_PRIVMSG;
	    break;
	case 'Q':
	    if (!strcmp(cmd, "QUIT")) return MSG_QUIT;
	    break;
	case 'R':
	    if (!strcmp(cmd, "REHASH")) return MSG_REHASH;
	    if (!strcmp(cmd, "RESTART")) return MSG_RESTART;
	    break;
	case 'S':
	    if (!strcmp(cmd, "SERVICE")) return MSG_SERVICE;
	    if (!strcmp(cmd, "SERVLIST")) return MSG_SERVLIST;
	    if (!strcmp(cmd, "SQUERY")) return MSG_SQUERY;
	    if (!strcmp(cmd, "SQUIT")) return MSG_SQUIT;
	    if (!strcmp(cmd, "STATS")) return MSG_STATS;
	    if (!strcmp(cmd, "SUMMON")) return MSG_SUMMON;
	    break;
	case 'T':
	    if (!strcmp(cmd, "TIME")) return MSG_TIME;
	    if (!strcmp(cmd, "TOPIC")) return MSG_TOPIC;
	    if (!strcmp(cmd, "TRACE")) return MSG_TRACE;
	    break;
	case 'U':
	    if (!strcmp(cmd, "USER")) return MSG_USER;
	    if (!strcmp(cmd, "USERHOST")) return MSG_USERHOST;
	    if (!strcmp(cmd, "USERS")) return MSG_USERS;
	    break;
	case 'V':
	    if (!strcmp(cmd, "VERSION")) return MSG_VERSION;
	    break;
	case 'W':
	    if (!strcmp(cmd, "WALLOPS")) return MSG_WALLOPS;
	    if (!strcmp(cmd, "WHO")) return MSG_WHO;
	    if (!strcmp(cmd, "WHOIS")) return MSG_WHOIS;
	    if (!strcmp(cmd, "WHOWAS")) return MSG_WHOWAS;
	    break;
	default: ;
    }
    return CMD_UNKNOWN;
}

SOEXPORT const char *IrcCommand_str(IrcCommand self)
{
    switch (self)
    {
	case RPL_WELCOME: return "001";
	case RPL_YOURHOST: return "002";
	case RPL_CREATED: return "003";
	case RPL_MYINFO: return "004";
	case RPL_BOUNCE: return "005";

	case MSG_PASS: return "PASS";
	case MSG_NICK: return "NICK";
	case MSG_USER: return "USER";
	case MSG_OPER: return "OPER";
	case MSG_MODE: return "MODE";
	case MSG_SERVICE: return "SERVICE";
	case MSG_QUIT: return "QUIT";
	case MSG_SQUIT: return "SQUIT";
	case MSG_JOIN: return "JOIN";
	case MSG_PART: return "PART";
	case MSG_TOPIC: return "TOPIC";
	case MSG_NAMES: return "NAMES";
	case MSG_LIST: return "LIST";
	case MSG_INVITE: return "INVITE";
	case MSG_KICK: return "KICK";
	case MSG_PRIVMSG: return "PRIVMSG";
	case MSG_NOTICE: return "NOTICE";
	case MSG_MOTD: return "MOTD";
	case MSG_LUSERS: return "LUSERS";
	case MSG_VERSION: return "VERSION";
	case MSG_STATS: return "STATS";
	case MSG_LINKS: return "LINKS";
	case MSG_TIME: return "TIME";
	case MSG_CONNECT: return "CONNECT";
	case MSG_TRACE: return "TRACE";
	case MSG_ADMIN: return "ADMIN";
	case MSG_INFO: return "INFO";
	case MSG_SERVLIST: return "SERVLIST";
	case MSG_SQUERY: return "SQUERY";
	case MSG_WHO: return "WHO";
	case MSG_WHOIS: return "WHOIS";
	case MSG_WHOWAS: return "WHOWAS";
	case MSG_KILL: return "KILL";
	case MSG_PING: return "PING";
	case MSG_PONG: return "PONG";
	case MSG_ERROR: return "ERROR";
	case MSG_AWAY: return "AWAY";
	case MSG_REHASH: return "REHASH";
	case MSG_DIE: return "DIE";
	case MSG_RESTART: return "RESTART";
	case MSG_SUMMON: return "SUMMON";
	case MSG_USERS: return "USERS";
	case MSG_WALLOPS: return "WALLOPS";
	case MSG_USERHOST: return "USERHOST";
	case MSG_ISON: return "ISON";

	case RPL_TRACELINK: return "200";
	case RPL_TRACECONNECTING: return "201";
	case RPL_TRACEHANDSHAKE: return "202";
	case RPL_TRACEUNKNOWN: return "203";
	case RPL_TRACEOPERATOR: return "204";
	case RPL_TRACEUSER: return "205";
	case RPL_TRACESERVER: return "206";
	case RPL_TRACESERVICE: return "207";
	case RPL_TRACENEWTYPE: return "208";
	case RPL_TRACECLASS: return "209";
	case RPL_TRACECONNECT: return "210";
	case RPL_STATSLINKINFO: return "211";
	case RPL_STATSCOMMANDS: return "212";
	case RPL_STATSCLINE: return "213";
	case RPL_STATSNLINE: return "214";
	case RPL_STATSILINE: return "215";
	case RPL_STATSKLINE: return "216";
	case RPL_STATSQLINE: return "217";
	case RPL_STATSYLINE: return "218";
	case RPL_ENDOFSTATS: return "219";
	case RPL_UMODEIS: return "221";
	case RPL_SERVICEINFO: return "231";
	case RPL_ENDOFSERVICES: return "232";
	case RPL_SERVICE: return "233";
	case RPL_SERVLIST: return "234";
	case RPL_SERVLISTEND: return "235";
	case RPL_STATSVLINE: return "240";
	case RPL_STATSLLINE: return "241";
	case RPL_STATSUPTIME: return "242";
	case RPL_STATSONLINE: return "243";
	case RPL_STATSHLINE: return "244";
	case RPL_STATSSLINE: return "245";
	case RPL_STATSPING: return "246";
	case RPL_STATSBLINE: return "247";
	case RPL_STATSDLINE: return "250";
	case RPL_LUSERCLIENT: return "251";
	case RPL_LUSEROP: return "252";
	case RPL_LUSERUNKNOWN: return "253";
	case RPL_LUSERCHANNELS: return "254";
	case RPL_LUSERME: return "255";
	case RPL_ADMINME: return "256";
	case RPL_ADMINLOC1: return "257";
	case RPL_ADMINLOC2: return "258";
	case RPL_ADMINEMAIL: return "259";
	case RPL_TRACELOG: return "261";
	case RPL_TRACEEND: return "262";
	case RPL_TRYAGAIN: return "263";
	case RPL_NONE: return "300";
	case RPL_AWAY: return "301";
	case RPL_USERHOST: return "302";
	case RPL_ISON: return "303";
	case RPL_UNAWAY: return "305";
	case RPL_NOWAWAY: return "306";
	case RPL_WHOISUSER: return "311";
	case RPL_WHOISSERVER: return "312";
	case RPL_WHOISOPERATOR: return "313";
	case RPL_WHOWASUSER: return "314";
	case RPL_ENDOFWHO: return "315";
	case RPL_WHOISCHANOP: return "316";
	case RPL_WHOISIDLE: return "317";
	case RPL_ENDOFWHOIS: return "318";
	case RPL_WHOISCHANNELS: return "319";
	case RPL_LISTSTART: return "321";
	case RPL_LIST: return "322";
	case RPL_LISTEND: return "323";
	case RPL_CHANNELMODEIS: return "324";
	case RPL_UNIQOPIS: return "325";
	case RPL_NOTOPIC: return "331";
	case RPL_TOPIC: return "332";
	case RPL_INVITING: return "341";
	case RPL_SUMMONING: return "342";
	case RPL_INVITELIST: return "346";
	case RPL_ENDOFINVITELIST: return "347";
	case RPL_EXCEPTLIST: return "348";
	case RPL_ENDOFEXCEPTLIST: return "349";
	case RPL_VERSION: return "351";
	case RPL_WHOREPLY: return "352";
	case RPL_NAMREPLY: return "353";
	case RPL_KILLDONE: return "361";
	case RPL_CLOSING: return "362";
	case RPL_CLOSEEND: return "363";
	case RPL_LINKS: return "364";
	case RPL_ENDOFLINKS: return "365";
	case RPL_ENDOFNAMES: return "366";
	case RPL_BANLIST: return "367";
	case RPL_ENDOFBANLIST: return "368";
	case RPL_ENDOFWHOWAS: return "369";
	case RPL_INFO: return "371";
	case RPL_MOTD: return "372";
	case RPL_INFOSTART: return "373";
	case RPL_ENDOFINFO: return "374";
	case RPL_MOTDSTART: return "375";
	case RPL_ENDOFMOTD: return "376";
	case RPL_YOUREOPER: return "381";
	case RPL_REHASHING: return "382";
	case RPL_YOURESERVICE: return "383";
	case RPL_MYPORTIS: return "384";
	case RPL_TIME: return "391";
	case RPL_USERSSTART: return "392";
	case RPL_USERS: return "393";
	case RPL_ENDOFUSERS: return "394";
	case RPL_NOUSERS: return "395";

	case ERR_NOSUCHNICK: return "401";
	case ERR_NOSUCHSERVER: return "402";
	case ERR_NOSUCHCHANNEL: return "403";
	case ERR_CANNOTSENDTOCHAN: return "404";
	case ERR_TOOMANYCHANNELS: return "405";
	case ERR_WASNOSUCHNICK: return "406";
	case ERR_TOOMANYTARGETS: return "407";
	case ERR_NOSUCHSERVICE: return "408";
	case ERR_NOORIGIN: return "409";
	case ERR_NORECIPIENT: return "411";
	case ERR_NOTEXTTOSEND: return "412";
	case ERR_NOTOPLEVEL: return "413";
	case ERR_WILDTOPLEVEL: return "414";
	case ERR_BADMASK: return "415";
	case ERR_UNKNOWNCOMMAND: return "421";
	case ERR_NOMOTD: return "422";
	case ERR_NOADMININFO: return "423";
	case ERR_FILEERROR: return "424";
	case ERR_NONICKNAMEGIVEN: return "431";
	case ERR_ERRONEUSNICKNAME: return "432";
	case ERR_NICKNAMEINUSE: return "433";
	case ERR_NICKCOLLISION: return "436";
	case ERR_UNAVAILRESOURCE: return "437";
	case ERR_USERNOTINCHANNEL: return "441";
	case ERR_NOTONCHANNEL: return "442";
	case ERR_USERONCHANNEL: return "443";
	case ERR_NOLOGIN: return "444";
	case ERR_SUMMONDISABLED: return "445";
	case ERR_USERSDISABLED: return "446";
	case ERR_NOTREGISTERED: return "451";
	case ERR_NEEDMOREPARAMS: return "461";
	case ERR_ALREADYREGISTERED: return "462";
	case ERR_NOPERMFORHOST: return "463";
	case ERR_PASSWDMISMATCH: return "464";
	case ERR_YOUREBANNEDCREEP: return "465";
	case ERR_YOUWILLBEBANNED: return "466";
	case ERR_KEYSET: return "467";
	case ERR_CHANNELISFULL: return "471";
	case ERR_UNKNOWNMODE: return "472";
	case ERR_INVITEONLYCHAN: return "473";
	case ERR_BANNEDFROMCHAN: return "474";
	case ERR_BADCHANNELKEY: return "475";
	case ERR_BADCHANMASK: return "476";
	case ERR_NOCHANMODES: return "477";
	case ERR_BANLISTFULL: return "478";
	case ERR_NOPRIVILEGES: return "481";
	case ERR_CHANOPPRIVSNEEDED: return "482";
	case ERR_CANTKILLSERVER: return "483";
	case ERR_RESTRICTED: return "484";
	case ERR_UNIQOPPRIVSNEEDED: return "485";
	case ERR_NOOPERHOST: return "491";
	case ERR_NOSERVICEHOST: return "492";
	case ERR_UMODEUNKNOWNFLAG: return "501";
	case ERR_USERSDONTMATCH: return "502";
	
	default: return 0;
    }
}
