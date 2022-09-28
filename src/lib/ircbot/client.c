#define _DEFAULT_SOURCE

#include <ircbot/log.h>

#include "client.h"
#include "connection.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

SOLOCAL Connection *Connection_createTcpClient(const char *remotehost,
	int port, int numerichosts)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG|AI_NUMERICSERV;
    char portstr[6];
    snprintf(portstr, 6, "%d", port);
    struct addrinfo *res, *res0;
    if (getaddrinfo(remotehost, portstr, &hints, &res0) < 0)
    {
	logmsg(L_ERROR, "client: cannot get address info");
	return 0;
    }
    int fd = -1;
    for (res = res0; res; res = res->ai_next)
    {
	if (res->ai_family != AF_INET && res->ai_family != AF_INET6) continue;
	fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0) continue;
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	errno = 0;
	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0
		&& errno != EINPROGRESS)
	{
	    close(fd);
	    fd = -1;
	}
	else break;
    }
    if (fd < 0)
    {
	freeaddrinfo(res0);
	logfmt(L_ERROR, "client: cannot connect to `%s'", remotehost);
	return 0;
    }
    Connection *conn = Connection_create(fd, CCM_CONNECTING);
    Connection_setRemoteAddr(conn, res->ai_addr, res->ai_addrlen,
	    numerichosts);
    freeaddrinfo(res0);
    return conn;
}

